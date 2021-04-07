#include "BCG.h"
#include "Logger.h"
#include "Utilites.h"
#include <numeric>

#define BCG_LOGGING

using namespace std::chrono;

BCG::BCG(SPI *spi, PinName int1_pin, PinName cs) : 
_g_drdy(int1_pin)
{
    _bus_control = BusControl::get_instance();
    _spi = spi;
    _cs = cs;
    _logger = Logger::get_instance();
}

BCG::~BCG()
{
}

BCG::HR_t BCG::get_buffer_element()
{
    HR_t tmp = _HR.front();
    _HR.erase(_HR.begin());
    return tmp;
}

bool BCG::bcg(const seconds num_seconds)
{
    // turn on SPI bus
    _bus_control->spi_power(true);

    /**
     * @brief Init 4th order bandpass (10-13 Hz) Butterworth filters
     * 
     * We need 3 of them (one per axis) since we're doing this real-time.
     * 
     * Documentation can be found in BiQuad.h. BiQuads were 
     * generated with MATLAB assuming 104 Hz sampling frequency.
     */

    
    
    
    
    BiQuadChain bcg_isolation_x;
    BiQuad bqx1( 6.76087639e-04,  1.35217528e-03,  6.76087639e-04,  1.00000000e+00, -2.16739514e-01,  7.10632547e-01);
    BiQuad bqx2( 1.00000000e+00,  2.00000000e+00,  1.00000000e+00,  1.00000000e+00, -4.54393846e-01,  7.17539837e-01);
    BiQuad bqx3( 1.00000000e+00, -2.00000000e+00,  1.00000000e+00,  1.00000000e+00, -5.35180090e-02,  8.69556033e-01);
    BiQuad bqx4( 1.00000000e+00, -2.00000000e+00,  1.00000000e+00,  1.00000000e+00, -6.64197076e-01,  8.77428070e-01);

    BiQuadChain bcg_isolation_y;
    BiQuad bqy1( 6.76087639e-04,  1.35217528e-03,  6.76087639e-04,  1.00000000e+00, -2.16739514e-01,  7.10632547e-01);
    BiQuad bqy2( 1.00000000e+00,  2.00000000e+00,  1.00000000e+00,  1.00000000e+00, -4.54393846e-01,  7.17539837e-01);
    BiQuad bqy3( 1.00000000e+00, -2.00000000e+00,  1.00000000e+00,  1.00000000e+00, -5.35180090e-02,  8.69556033e-01);
    BiQuad bqy4( 1.00000000e+00, -2.00000000e+00,  1.00000000e+00,  1.00000000e+00, -6.64197076e-01,  8.77428070e-01);

    BiQuadChain bcg_isolation_z;
    BiQuad bqz1( 6.76087639e-04,  1.35217528e-03,  6.76087639e-04,  1.00000000e+00, -2.16739514e-01,  7.10632547e-01);
    BiQuad bqz2( 1.00000000e+00,  2.00000000e+00,  1.00000000e+00,  1.00000000e+00, -4.54393846e-01,  7.17539837e-01);
    BiQuad bqz3( 1.00000000e+00, -2.00000000e+00,  1.00000000e+00,  1.00000000e+00, -5.35180090e-02,  8.69556033e-01);
    BiQuad bqz4( 1.00000000e+00, -2.00000000e+00,  1.00000000e+00,  1.00000000e+00, -6.64197076e-01,  8.77428070e-01);

    bcg_isolation_x.add( &bqx1 ).add( &bqx2 ).add( &bqx3 ).add( &bqx4 );
    bcg_isolation_y.add( &bqy1 ).add( &bqy2 ).add( &bqy3 ).add( &bqy4 );
    bcg_isolation_z.add( &bqz1 ).add( &bqz2 ).add( &bqz3 ).add( &bqz4 );

    // Init 2nd order bandpass (0.75-2.5 Hz) Butterworth filter
    BiQuadChain hr_isolation;
    BiQuad bq5( 0.00952329,  0.01904657,  0.00952329,  1., -1.74516121,  0.8078649 );
    BiQuad bq6( 1.,          -2.,         1.,          1., -1.91061565,  0.92055723 );

    hr_isolation.add( &bq5 ).add ( &bq6 );

    // Give time for the chip to turn on
    ThisThread::sleep_for(10ms);

    // Set up gyroscope
    LSM6DSLSensor imu(_spi, _cs);
    imu.init(NULL);
    imu.set_g_odr(G_FREQUENCY);
    imu.set_g_fs(G_FULL_SCALE);
    imu.enable_g();
    imu.enable_int1_drdy_g();

    // let gyroscope warm up
    ThisThread::sleep_for(500ms);

    // init some tracker variables
    float last_bcg_val = -1.0;
    uint32_t acquired_samples = 0;
    vector<double> last_crosses;
    bool new_hr_reading = false;

    LowPowerTimer zc_timer;
    zc_timer.start();

    _logger->log(TRACE_INFO, "%s", "SENSING HR");

    #ifdef BCG_LOGGING
    {
        _logger->log(TRACE_WARNING, "ts, a_x, a_y, a_z, g_x, g_y, g_z, x_filt, y_filt, z_filt, l2norm, bcg");
    }
    #endif // BCG_LOGGING

    // TODO add timeout to IMU

    // acquire and process samples until num_seconds has elapsed
    while(zc_timer.elapsed_time() <= duration_cast<microseconds>(num_seconds))
    {
        if (_g_drdy.read())        
        {
            // get samples
            float gyr[3] = {0};
            imu.get_g_axes_f(gyr);
            double x = gyr[0];
            double y = gyr[1];
            double z = gyr[2];

            float acc[3] = {0};
            imu.get_x_axes_f(acc);

            acquired_samples++;

            // put each axis through bcg features isolation filter
            double xfilt = bcg_isolation_x.step(x);
            double yfilt = bcg_isolation_y.step(y);
            double zfilt = bcg_isolation_z.step(z);
            
            // l2norm the signal
            double mag = _l2norm(xfilt, yfilt, zfilt);

            // send l2norm through hr isolation filter
            float next_bcg_val = hr_isolation.step(mag);

            #ifdef BCG_LOGGING
            {
                _logger->log(TRACE_WARNING, "%i, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f", zc_timer.read_ms(), acc[0], acc[1], acc[2], x, y, z, xfilt, yfilt, zfilt, mag, next_bcg_val);
            }
            #endif // BCG_LOGGING

            // look for a descending zero-cross
            if (last_bcg_val > 0 && next_bcg_val <= 0)
            {
                double zc_ts = (double)zc_timer.read();
                last_crosses.push_back(zc_ts);

                if (last_crosses.size() >= NUM_EVENTS)
                {
                    /**
                     * Now see if the last NUM_EVENTS crosses warrant a heart rate calculation,
                     * by checking the standard deviation of the instantaneous heart rates 
                     * derived from them. If the standard deviation falls below our STD_DEV_THRESHOLD,
                     * use them to calculate a heart rate and save to the vector.
                     */
                    vector<double> crosses_copy = last_crosses;

                    std::adjacent_difference(crosses_copy.begin(), crosses_copy.end(), crosses_copy.begin());
                    crosses_copy.erase(crosses_copy.begin());

                    Utilities::reciprocal(crosses_copy); // get element-wise frequency
                    Utilities::multiply(crosses_copy, 60.0); // get element-wise heart rate
                    float std_dev = Utilities::std_dev(crosses_copy); // calculate standard deviation across the heart rates
                    // printf("%0.3f", std_dev);

                    if (std_dev < STD_DEV_THRESHOLD) // we have some stable readings! calculate heart rate
                    {
                        float rate_raw = Utilities::mean(crosses_copy);
                        uint8_t rate = (uint8_t)Utilities::round(rate_raw);

                        // bounds checking
                        if (rate >= MIN_HR && rate <= MAX_HR)
                        {
                            new_hr_reading = true;

                            HR_t new_hr;
                            new_hr.rate = rate;
                            new_hr.timestamp = time(NULL);

                            // _logger->log(TRACE_INFO, "New HR reading --> rate: %0.1f, time: %lli", rate_raw, new_hr.timestamp);

                            while (_HR.size() >= HR_BUFFER_SIZE)
                            {
                                _HR.erase(_HR.begin());
                            }

                            _HR.push_back(new_hr);
                        }
                    }

                    // now remove first element from last_crosses vector to keep it at NUM_EVENTS length
                    while(last_crosses.size() >= NUM_EVENTS)
                    {
                        last_crosses.erase(last_crosses.begin());
                    }
                }
            }

            last_bcg_val = next_bcg_val;
        }
        
        ThisThread::sleep_for(1ms);
    }

    _bus_control->spi_power(false);
    zc_timer.stop(); 

    return new_hr_reading;
}

double BCG::_l2norm(double x, double y, double z)
{
    double result = sqrt( (x * x) + (y * y) + (z * z) );
    return result;
}


/*DINOSAURS
   *                               *     _
        /\     *            ___.       /  `)
    *  //\\    /\          ///\\      / /
      ///\\\  //\\/\      ////\\\    / /     /\
     ////\\\\///\\/\\.-~~-.///\\\\  / /     //\\
    /////\\\\///\\/         `\\\\\\/ /     ///\\
   //////\\\\// /            `\\\\/ /     ////\\
  ///////\\\\\//               `~` /\    /////\\
 ////////\\\\\/      ,_____,   ,-~ \\\__//////\\\
 ////////\\\\/  /~|  |/////|  |\\\\\\\\@//jro/\\
 //<           / /|__|/////|__|///////~|~/////\\
 ~~~     ~~   ` ~   ..   ~  ~    .     ~` `   '.
 ~ _  -  -~.    .'   .`  ~ .,    '.    ~~ .  '*/

// uint8_t BCG::calc_hr()
// {
//     if (_g_x.size() < MIN_SAMPLES)
//     {
//         _logger->log(TRACE_WARNING, "fewer samples than recommended (%u of %u)", _g_x.size(), MIN_SAMPLES);
//     }

//     // printf("pre-filter:\r\n");
//     // for (int i = 0; i < _g_x.size(); i++)
//     // {
//     //     printf("%f, %f, %f\r\n", _g_x.at(i), _g_y.at(i), _g_z.at(i));
//     // }

//     _step_1_filter(_g_x);
//     _step_1_filter(_g_y);
//     _step_1_filter(_g_z);

//     // printf("post-filter:\r\n");
//     // for (int i = 0; i < _g_x.size(); i++)
//     // {
//     //     printf("%f, %f, %f\r\n", _g_x.at(i), _g_y.at(i), _g_z.at(i));
//     // }

//     vector<float> l2norm;
//     _l2norm(_g_x, _g_y, _g_z, l2norm);

//     // for (int i = 0; i < l2norm.size(); i++)
//     // {
//     //     printf("%f\r\n", l2norm.at(i));
//     // }

//     _step_2_filter(l2norm);
//     _g_x.clear();
//     _g_y.clear();
//     _g_z.clear();

//     // for (int i = 0; i < l2norm.size(); i++)
//     // {
//     //     printf("%f\r\n", l2norm.at(i));
//     // }

//     /** throw out the first 128 data points.
//      * these are attenuated due to the FIR
//      * filters.
//      */

//     l2norm.erase(l2norm.begin(), l2norm.begin()+128);
//     l2norm.resize(MIN_SAMPLES, 0); // pad with zeros to interpolate
//     // printf("%u", l2norm.size());

//     vector<complex<double>> fft;
//     fft.resize(l2norm.size());
//     _fft.fft(l2norm.data(), fft.data(), (uint16_t)l2norm.size());

//     float freq_resolution = 52.0 / (float)l2norm.size();
//     float max_val = 0.0;
//     float max_val_frequency = 0.0;
//     for (int i = 0; i < fft.size() / 2; i++)
//     {
//         float val = std::abs(fft.at(i));
//         if (val > max_val) 
//         {
//             max_val = val;
//             max_val_frequency = i * freq_resolution;
//         }
//     }

//     float HR = max_val_frequency * 60.0;
//     _logger->log(TRACE_INFO, "HR = %0.1f", HR);

//     return (uint8_t)Utilities::round(HR);
// }



// void BCG::_l2norm(vector<float> &x, vector<float> &y, vector<float> &z, vector<float> &l2norm)
// {
//     while(!x.empty())
//     {
//         float result = sqrt(x.front() * x.front() + y.front() * y.front() + z.front() * z.front());

//         // printf("result = %f, sizes: x = %u, y = %u, z = %u\r\n", result, x.size(), y.size(), z.size());

//         x.erase(x.begin());
//         y.erase(y.begin());
//         z.erase(z.begin());

//         l2norm.push_back(result);
//     }
// }

            // /**
            //  * if it's the first sample, feed in data to prime the filter
            //  * and avoid an obvious step response. Step response
            //  * calculated by MATLAB.
            //  * 
            //  * We don't do anything with the values returned
            //  * from the function.
            //  */
            // if (acquired_samples == 1) 
            // {
            //     for (int i = 0; i < G_FREQUENCY * BCG_STEP_DUR; i++)
            //     {
            //         bcg_isolation_x.step(x);
            //         bcg_isolation_y.step(y);
            //         bcg_isolation_z.step(z);
            //     }
            // }

                        // /**
            //  * Similar to the bcg_isolation filter, we want
            //  * to prime the hr_isolation filter with this first
            //  * value for about the length of its step response.
            //  */
            // if (acquired_samples == 1) 
            // {
            //     for (int i = 0; i < G_FREQUENCY * HR_STEP_DUR; i++)
            //     {
            //         hr_isolation.step(mag);
            //     }
            // }


                            // if (num_zc >= INSTANT_AVERAGE)
                // {
                //     float result = 0.0;
                //     for (int i = num_zc; i > num_zc - INSTANT_AVERAGE; i--)
                //     {
                //          result += (1.0 / (descending_zc_timestamps.at(i) - descending_zc_timestamps.at(i-1))) * 60.0 * 1.0 / INSTANT_AVERAGE;
                //     }
                //     _logger->log(TRACE_INFO, "HR = %f", result);
                // }


    // // now that we've collected and processed the samples, let's see how clean the signal looks
    // if (num_zc < 3)
    // {
    //     _logger->log(TRACE_WARNING, "Not enough zero-crosses to detect HR! Detected only %u.", num_zc);
    //     return -1;
    // }
    // if (collection_time < 3)
    // {
    //     _logger->log(TRACE_WARNING, "Collection time not long enough to calculate HR! Only %0.2f s", collection_time);
    // }
    // // others checks here as we come up with them...

    // // calculate inter-beat intervals -> instananeous HRs -> average them
    // float HR_sum = 0;
    // for (int i = 1; i < descending_zc_timestamps.size(); i++) // start at index 2
    // {
    //     float interval = descending_zc_timestamps.at(i) - descending_zc_timestamps.at(i-1);
    //     float instant_HR = 1.0 / interval * 60.0;
    //     HR_sum += instant_HR;
    // }

    // float HR = HR_sum / descending_zc_timestamps.size();
    // _logger->log(TRACE_INFO, "HR = %0.1f", HR);

    //     float sum = 0;
    // uint8_t size = _HR.size();
    // for (int i = 0; i < size; i++)
    // {
    //     sum += _HR.front();
    //     _HR.erase(_HR.begin());
    // }

    // float mean = sum / (float)size;  