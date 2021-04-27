#include "RespiratoryRate.hpp"
#include <numeric>
#include "Utilites.h"

// #define RESP_RATE_LOGGING

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

float RespiratoryRate::respiratory_rate(const uint8_t num_seconds, RespSource_t source)
{
	if (source == BAROMETER)
	{
		// turn on SPI bus
		_bus_control->spi_power(true);

		// give time for the chip to turn on
		ThisThread::sleep_for(10ms);

		if (!_barometer.initialize() || !_barometer.set_fifo_full_interrupt(true) || !_barometer.set_frequency(FREQUENCY))
		{
			_logger->log(TRACE_WARNING, "%s", "barometer failed to initialize");
			return ERROR;
		}
	}
	else if (source == THERMOMETER)
	{
		// turn on I2C bus
		_bus_control->i2c_power(true);

		// give time for the chip to turn on
		ThisThread::sleep_for(10ms);

		_temp.initialize();
		_temp.setFrequency(FREQUENCY); // hz
	}

    /**
     * @brief Init 2nd order bandpass (1/15-0.5 Hz) Butterworth filter
     * 
     * Documentation can be found in BiQuad.h. BiQuads were 
     * generated with filter-designer.py assuming 10 Hz sampling frequency.
     */

    BiQuadChain bpf;
	BiQuad bq1( 0.0154759, 0.03095179, 0.0154759, 1., -1.64951809, 0.7183872 );
	BiQuad bq2( 1., -2., 1., 1., -1.94510068, 0.9471958 );

	bpf.add( &bq1 ).add( &bq2 );

    // start timer
    LowPowerTimer timer;
    timer.start();

    // initialize variables
    double last_sample = -1.0;
    uint16_t sample_index = 0;
    vector<uint16_t> zc_indices;
	bool zc_initialized;
	bool initialized = false;

	#ifdef RESP_RATE_LOGGING
	{
		_logger->log(TRACE_INFO, "ts, %s, %s, d_zc, a_zc", source == THERMOMETER ? "raw_temp" : "raw_pressure", source == THERMOMETER ? "filtered_temp" : "filtered_humidity");
	}
	#endif // RESP_RATE_LOGGING

    while (timer.read() <= num_seconds)
    {
		uint16_t buffer_size = 0;

        if (source == BAROMETER)
		{
			_barometer.update();
	        buffer_size = _barometer.get_pressure_buffer_size();
		}
		else if (source == THERMOMETER)
		{
			_temp.update();
			buffer_size = _temp.getBufferSize();
		}

        if (buffer_size >= (FREQUENCY * BUFFER))
        {
			uint16_t* samples = NULL;
			if (source == BAROMETER)
			{
				samples = _barometer.get_pressure_array();
				_barometer.clear_buffers();
			}
			else if (source == THERMOMETER)
			{
            	samples = _temp.getBuffer();
				_temp.clearBuffer();
			}
            
            for (int i = 0; i < buffer_size; i++)
            {
				double sample = 0;
				if (source == BAROMETER)
				{
					// sample = samples[i];
					sample = _barometer.convert_to_hpa(samples[i]);
				}
				else if (source == THERMOMETER)
				{
					sample = (float)samples[i] / 100.0;
				}

				if (!initialized)
				{
					for (int i = 0; i < FREQUENCY * 200; i++)
					{
						bpf.step(sample); // prime filter with initial value
					}

					initialized = true;
				}

                // pass sample through bandpass filter
                double filtered_sample = bpf.step(sample);

                // look for zero-crosses
				bool d_zc = false;
				bool a_zc = false;
                if (last_sample > 0 && filtered_sample < 0)
                {
					if (zc_initialized)
					{
                    	zc_indices.push_back(sample_index);
						_logger->log(TRACE_DEBUG, "breath detected");
					}
					else
					{
						zc_initialized = true;
					}
					d_zc = true;
                }
				else if (last_sample < 0 && filtered_sample > 0)
				{
					a_zc = true;
				}

				#ifdef RESP_RATE_LOGGING
				{
					_logger->log(TRACE_INFO, "%f, %f, %f, %i, %i", timer.read(), sample, filtered_sample, d_zc, a_zc);
					wait_us(750);
				}
				#endif // RESP_RATE_LOGGING

				last_sample = filtered_sample;
                sample_index++;
            }

			_barometer.clear_buffers();
        }

		ThisThread::sleep_for(10ms);
    }

	if (source == BAROMETER)
	{
		// turn off SPI bus
		_bus_control->spi_power(false);
	}
	else if (source == THERMOMETER)
	{
		// turn off I2C bus
		_temp.stop();
		// _bus_control->i2c_power(false); // turning off the bus actually results in _higher_ current consumption than leaving it on
	}

	// now calculate resp rate from the zero-crosses we've detected
	vector<double> zc_ts(zc_indices.begin(), zc_indices.end()); // copy indices to new vector

	Utilities::multiply(zc_ts, 1.0 / (float)FREQUENCY); // convert indices to timestamps

	std::adjacent_difference(zc_ts.begin(), zc_ts.end(), zc_ts.begin());
	zc_ts.erase(zc_ts.begin());

	Utilities::reciprocal(zc_ts); // get element-wise frequency
	Utilities::multiply(zc_ts, 60.0); // get element-wise respiratory rate

	float std_dev = Utilities::std_dev(zc_ts); // get standard deviation

	for (int i = 0; i < zc_ts.size(); i++)
	{
		_logger->log(TRACE_TRACE, "rr[%i] = %0.1f", zc_ts[i]);
		if (zc_ts[i] < 4 || zc_ts[i] > 30) // these resp rates are out-of bounds for our filtering (and physiologically unlikely)
		{
			_logger->log(TRACE_DEBUG, "Deleting resp rate element %i: %0.1f breaths/min", zc_ts[i]);
			zc_ts.erase(zc_ts.begin() + i); 
		}
	}

	double resp_rate = 0;
	if (zc_ts.size() < (4 * num_seconds / 60))
	{
		resp_rate = -1;
		_logger->log(TRACE_WARNING, "Not enough zero-crosses to detect resp rate: %i crosses", zc_ts.size());
	}
	else
	{
		resp_rate = Utilities::mean(zc_ts);
		if (resp_rate < 4 || resp_rate > 30) // filter not designed to detect RR greater than this.
		{
			resp_rate = -1; 
		}
		_logger->log(TRACE_INFO, "Respiration rate = %0.1f, std dev = %0.1f", resp_rate, std_dev);
	}

	return resp_rate;
}
