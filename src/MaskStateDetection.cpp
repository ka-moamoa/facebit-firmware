#include "MaskStateDetection.hpp"
#include "BusControl.h"

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
    _logger->log(TRACE_INFO, "%s", "CHECKING MASK ON");

    BusControl* _bus_control = BusControl::get_instance();
    _bus_control->spi_power(true);

    LowPowerTimer timer;
    
    ThisThread::sleep_for(10ms);

    if (!_barometer->initialize() || !_barometer->set_fifo_full_interrupt(true))
    {
        _logger->log(TRACE_WARNING, "%s", "barometer failed to initialize");
        return ERROR;
    }

    _barometer->set_max_buffer_size(int(DETECTION_WINDOW * _barometer->BAROMETER_FREQUENCY));

    _bus_control->set_power_lock(BusControl::BAROMETER, true);

    while(timer.read() < DETECTION_WINDOW + 1) // timeout so we don't get stuck
    {
        _bus_control->spi_power(true);
        ThisThread::sleep_for(10ms);

        _barometer->update();

        _bus_control->spi_power(false);
        
        if(_barometer->get_buffer_full())
        {
            uint16_t* pressure_buffer = _barometer->get_pressure_array();

            uint16_t min = 65535;
            uint16_t max = 0;
            for (int i = 0; i < _barometer->get_pressure_buffer_size(); i++)
            {
                uint16_t val = pressure_buffer[i];

                if (val < min) min = val;
                else if (val > max) max = val;
            }
            
            uint16_t diff = abs(max - min);
            _logger->log(TRACE_TRACE, "max/min diff = %u", diff);

            if (diff > ON_THRESHOLD) return ON;
            else return OFF;
        }
        
        ThisThread::sleep_for(200ms);
    }

    return ERROR; // shouldn't end up here, unless barometer stops responding
}
