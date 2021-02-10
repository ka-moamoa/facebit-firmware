#include "BCG.h"
#include "SWOLogger.h"

BCG::BCG(SPI *spi, PinName int1_pin, PinName cs) : 
_g_drdy(int1_pin)
{
    _bus_control = BusControl::get_instance();
    _spi = spi;
    _cs = cs;
}

BCG::~BCG()
{
}

float BCG::bcg(const uint16_t num_samples)
{
    // turn on SPI bus
    _bus_control->init();
    _bus_control->spi_power(true);

    // speed up bus to reduce transaction time
    _spi->frequency(8000000);

    /**
     * @brief Init 4th order bandpass (10-13 Hz) Butterworth filters
     * 
     * We need 3 of them since we're doing this real-time.
     * 
     * Documentation can be found in BiQuad.h. BiQuads were 
     * generated with MATLAB assuming 104 Hz sampling frequency.
     */
    BiQuadChain bcg_isolation_x;
    BiQuadChain bcg_isolation_y;
    BiQuadChain bcg_isolation_z;
    BiQuad bq1( 3.48624e-05, -6.97248e-05, 3.48624e-05, -1.42500e+00, 8.55705e-01 );
    BiQuad bq2( 1.00000e+00, 2.00013e+00, 1.00013e+00, -1.50474e+00, 8.66039e-01 );
    BiQuad bq3( 1.00000e+00, 1.99987e+00, 9.99873e-01, -1.42640e+00, 9.34678e-01 );
    BiQuad bq4( 1.00000e+00, -2.00000e+00, 1.00000e+00, -1.61444e+00, 9.45725e-01 );

    bcg_isolation_x = bq1 * bq2 * bq3 * bq4;
    bcg_isolation_y = bq1 * bq2 * bq3 * bq4;
    bcg_isolation_z = bq1 * bq2 * bq3 * bq4;

    // Init 2nd order bandpass (0.75-2.5 Hz) Butterworth filter
    BiQuadChain hr_isolation;
    BiQuad bq5( 2.58488e-03, -5.16976e-03, 2.58488e-03, -1.88131e+00, 8.98067e-01 );
    BiQuad bq6( 1.00000e+00, 2.00000e+00, 1.00000e+00, -1.95665e+00, 9.59238e-01 );

    hr_isolation = bq5 * bq6;

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
    uint8_t num_zc = 0;
    vector<float> descending_zc_timestamps;
    LowPowerTimer zc_timer;
    zc_timer.start();

    // acquire and process samples until we have as many as asked for
    while(acquired_samples < num_samples)
    {
        if (_g_drdy.read())        
        {
            // get samples
            float gyr[3] = {0};
            imu.get_g_axes_f(gyr);
            double x = gyr[0];
            double y = gyr[1];
            double z = gyr[2];

            acquired_samples++;

            /**
             * if it's the first sample, feed in about
             * 2 seconds worth of data to prime the filter
             * and avoid an obvious step response. Two
             * seconds was chosen based on the step response
             * calculated by MATLAB.
             * 
             * We don't do anything with the values returned
             * from the function.
             */
            if (acquired_samples == 1) 
            {
                for (int i = 0; i < G_FREQUENCY * BCG_STEP_DUR; i++)
                {
                    bcg_isolation_x.step(x);
                    bcg_isolation_y.step(y);
                    bcg_isolation_z.step(z);
                }
            }

            // put each axis through bcg features isolation filter
            x = bcg_isolation_x.step(x);
            y = bcg_isolation_y.step(y);
            z = bcg_isolation_z.step(z);

            // l2norm the signal
            double mag = _l2norm(x, y, z);

            /**
             * Similar to the bcg_isolation filter, we want
             * to prime the hr_isolation filter with this first
             * value for about the length of its step response (~2.5s).
             */
            if (acquired_samples == 1) 
            {
                for (int i = 0; i < G_FREQUENCY * HR_STEP_DUR; i++)
                {
                    hr_isolation.step(mag);
                }
            }

            // send l2norm through hr isolation filter
            float next_bcg_val = hr_isolation.step(mag);
            printf("%f\r\n", next_bcg_val);
            
            // look for a descending zero-cross
            if (last_bcg_val > 0 && next_bcg_val <= 0)
            {
                float zc_ts = zc_timer.read();
                descending_zc_timestamps.push_back(zc_ts);
                
                // LOG_INFO("%s", "BEAT!");
    
                num_zc++;
            }

            last_bcg_val = next_bcg_val;
        }
        ThisThread::sleep_for(1ms);
    }

    _bus_control->spi_power(false);

    float collection_time = zc_timer.read();
    zc_timer.stop();

    // now that we've collected and processed the samples, let's see how clean the signal looks
    if (num_zc < 3)
    {
        LOG_WARNING("Not enough zero-crosses to detect HR! Detected only %u.", num_zc);
        return -1;
    }
    if (collection_time < 3)
    {
        LOG_WARNING("Collection time not long enough to calculate HR! Only %0.2f s", collection_time);
    }
    // others checks here as we come up with them...

    // calculate inter-beat intervals -> instananeous HRs -> average them
    float HR_sum;
    for (int i = 1; i < descending_zc_timestamps.size(); i++) // start at index 2
    {
        float interval = descending_zc_timestamps.at(i) - descending_zc_timestamps.at(i-1);
        float instant_HR = 1.0 / interval * 60.0;
        HR_sum += instant_HR;
    }

    float HR = HR_sum / descending_zc_timestamps.size();
    // LOG_INFO("HR = %0.1f", HR);

    return HR;
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
//         LOG_WARNING("fewer samples than recommended (%u of %u)", _g_x.size(), MIN_SAMPLES);
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
//     LOG_INFO("HR = %0.1f", HR);

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