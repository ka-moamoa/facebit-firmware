#include "MaskStateDetection.hpp"
#include "BusControl.h"
#include "../iir-filter-kit/BiQuad.h"

MaskStateDetection::MaskStateDetection(Barometer* barometer)
{
    _logger = Logger::get_instance();
    _barometer = barometer;
}

MaskStateDetection::~MaskStateDetection()
{
}

MaskStateDetection::MASK_STATE_t MaskStateDetection::is_on()
{
    MASK_STATE_t mask_state = OFF;

    _logger->log(TRACE_INFO, "%s", "CHECKING MASK ON");

    BusControl* _bus_control = BusControl::get_instance();
    _bus_control->spi_power(true);

    LowPowerTimer timer;
    
    ThisThread::sleep_for(10ms);

    /**
     * @brief Init 2nd order bandpass (1/15-1 Hz) Butterworth filter
     * 
     * Documentation can be found in BiQuad.h. BiQuads were 
     * generated with filter-designer.py assuming 10 Hz sampling frequency.
     */

    BiQuadChain bpf;
	BiQuad bq1( 0.06004382,  0.12008764,  0.06004382,  1.,         -1.21246615,  0.46367415);
 	BiQuad bq2( 1.,         -2.,          1.,          1.,         -1.94162756,  0.94354483);

	bpf.add( &bq1 ).add( &bq2 );

    if (!_barometer->initialize() || !_barometer->set_fifo_full_interrupt(true) || !_barometer->set_frequency(SAMPLING_FREQUENCY))
    {
        _logger->log(TRACE_WARNING, "%s", "barometer failed to initialize");
        return ERROR;
    }

    _barometer->set_max_buffer_size(int(DETECTION_WINDOW * _barometer->get_frequency()));

    timer.start();
    while(timer.read() < DETECTION_WINDOW + 5) // timeout so we don't get stuck
    {
        uint32_t sleep_time = std::min((int)(32.0 / (float)_barometer->get_frequency() * 1000.0 + 10.0), (int)((DETECTION_WINDOW * 1000.0) - timer.read_ms() + 10));
        _logger->log(TRACE_DEBUG, "sleep time = %lu", sleep_time);
        ThisThread::sleep_for(sleep_time);

        _barometer->update(true);

        if(_barometer->get_buffer_full())
        {
            uint16_t* pressure_buffer = _barometer->get_pressure_array();

            for (int i = 0; i < SAMPLING_FREQUENCY * 200; i++)
            {
                bpf.step(pressure_buffer[1]); // prime filter with initial value
            }

            float peak_max = 0;
            float trough_min = 4294967295;
            uint32_t last_zc = 0;
            float last_filtered_pressure = -1;
            for (int i = 1; i < _barometer->get_pressure_buffer_size(); i++)
            {
                float filtered_pressure = bpf.step(pressure_buffer[i]);

                if (filtered_pressure < trough_min) trough_min = filtered_pressure;
                else if (filtered_pressure > peak_max) peak_max = filtered_pressure;

                if (filtered_pressure < 0 && last_filtered_pressure > 0)
                {
                    if (peak_max > ON_THRESHOLD && trough_min < (-1 * ON_THRESHOLD) && last_zc != 0 && i - last_zc > SAMPLING_FREQUENCY * 2)
                    {
                        _logger->log(TRACE_DEBUG, "breath detected! mask on. peak max = %0.2f, peak min = %0.2f\n", peak_max, trough_min);
                        mask_state = ON;
                    }
                    else
                    {
                        _logger->log(TRACE_DEBUG, "breath NOT detected! mask on. peak max = %0.2f, peak min = %0.2f\n", peak_max, trough_min);
                    }
                    last_zc = i;
                }
                last_filtered_pressure = filtered_pressure;
            }

            _barometer->clear_buffers();

            _bus_control->spi_power(false);

            return mask_state;
        }
    }

    return ERROR; // shouldn't end up here, unless barometer stops responding
}
