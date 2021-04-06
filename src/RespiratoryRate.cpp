#include "RespiratoryRate.hpp"
#include <numeric>
#include "Utilites.h"

#define RESP_RATE_LOGGING

using namespace std::chrono;

RespiratoryRate::RespiratoryRate(Si7051 &temp,  Barometer &barometer) : 
_temp(temp),
_barometer(barometer)
{
    _bus_control = BusControl::get_instance();
    _logger = Logger::get_instance();
}

RespiratoryRate::~RespiratoryRate()
{
}

RespiratoryRate::RR_t RespiratoryRate::get_buffer_element()
{
    RR_t tmp = respiratory_rate_buffer.at(0);
    respiratory_rate_buffer.erase(respiratory_rate_buffer.begin());
    return tmp;
}

float RespiratoryRate::respiratory_rate(const seconds num_seconds)
{
    // turn on SPI bus
    _bus_control->spi_power(true);

    /**
     * @brief Init 4th order bandpass (1/15-0.5 Hz) Butterworth filter
     * 
     * Documentation can be found in BiQuad.h. BiQuads were 
     * generated with MATLAB assuming 25 Hz sampling frequency.
     */

    BiQuadChain pressure_bpf;
	BiQuad bq1( 5.68788e-02, 0.00000e+00, -5.68788e-02, -1.88565e+00, 8.86242e-01 );	
	pressure_bpf.add( &bq1 );


	BiQuadChain temperature_bpf;
	BiQuad bq2( 1.99140e-01, 0.00000e+00, -1.99140e-01, -1.59970e+00, 6.01721e-01 );	
	temperature_bpf.add( &bq2 );

    // give time for the chip to turn on
    ThisThread::sleep_for(10ms);

    if (!_barometer.initialize() || !_barometer.set_fifo_full_interrupt(true))
    {
        _logger->log(TRACE_WARNING, "%s", "barometer failed to initialize");
        return ERROR;
    }

    // start timer
    LowPowerTimer timer;
    timer.start();

    // initialize variables
    double last_pressure_val = -1.0;
    uint16_t sample_index = 0;
    vector<uint16_t> zc_indices;
	float min_val = 0, max_val = 0;
	bool initialized = false;

	#ifdef RESP_RATE_LOGGING
	{
		_logger->log(TRACE_INFO, "ts, pressure, filtered_pressure, d_zc, a_zc, raw_temp, filtered_temp");
	}
	#endif // RESP_RATE_LOGGING

    while (timer.elapsed_time() <= duration_cast<microseconds>(num_seconds))
    {
        _barometer.update();

        uint16_t buffer_size = _barometer.get_pressure_buffer_size();
        if (buffer_size >= 32)
        {
            uint16_t* pressure_samples = _barometer.get_pressure_array();
			uint16_t* temperature_samples = _barometer.get_temperature_array();
            
            for (int i = 0; i < buffer_size; i++)
            {
                double pressure_sample = _barometer.convert_to_hpa(pressure_samples[i]);
				double temperature_sample = (float)temperature_samples[i] / 10.0;

				if (!initialized)
				{
					for (int i = 0; i < _barometer.BAROMETER_FREQUENCY * 200; i++)
					{
						pressure_bpf.step(pressure_sample); // prime filter with initial value
						temperature_bpf.step(temperature_sample);
					}

					initialized = true;
				}

                // pass sample through bandpass filter
                double filtered_pressure_sample = pressure_bpf.step(pressure_sample);
				double filtered_temp_sample = temperature_bpf.step(temperature_sample);

                // look for zero-crosses
				bool d_zc = false;
				bool a_zc = false;
                if (last_pressure_val > 0 && filtered_pressure_sample < 0)
                {
                    zc_indices.push_back(sample_index);
					max_val = 0;
					min_val = 0;

					d_zc = true;
                }
				else if (last_pressure_val < 0 && filtered_pressure_sample > 0)
				{
					a_zc = true;
				}

				#ifdef RESP_RATE_LOGGING
				{
					_logger->log(TRACE_INFO, "%f, %f, %f, %i, %i, %f, %f", timer.read(), pressure_sample, filtered_pressure_sample, d_zc, a_zc, temperature_sample, filtered_temp_sample);
					wait_us(750);
				}
				#endif // RESP_RATE_LOGGING

				last_pressure_val = filtered_pressure_sample;
                sample_index++;
            }

			_barometer.clear_buffers();
        }

		ThisThread::sleep_for(10ms);
    }

	// now calculate resp rate from the zero-crosses we've detected
	vector<double> zc_ts(zc_indices.begin(), zc_indices.end()); // copy indices to new vector

	Utilities::multiply(zc_ts, 1.0 / _barometer.BAROMETER_FREQUENCY); // convert indices to timestamps

	std::adjacent_difference(zc_ts.begin(), zc_ts.end(), zc_ts.begin());

	Utilities::reciprocal(zc_ts); // get element-wise frequency
	Utilities::multiply(zc_ts, 60.0); // get element_wise respiratory rate

	double resp_rate = Utilities::mean(zc_ts);

	return resp_rate;
}
