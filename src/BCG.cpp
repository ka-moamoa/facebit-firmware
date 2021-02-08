#include "BCG.h"
#include "SWOLogger.h"
#include "BCGIsolationFilter.h"
#include "HRFilter.h"
#include "math.h"
#include <complex.h>

using namespace std::chrono;

BCG::BCG(SPI *spi, PinName int1_pin, PinName cs) : 
_g_drdy(int1_pin),
_fft()
{
    _bus_control = BusControl::get_instance();
    _spi = spi;
    _cs = cs;
}

BCG::~BCG()
{
}

void BCG::collect_data(seconds num_seconds)
{
    if (num_seconds < MIN_SAMPLE_DURATION)
    {
        LOG_WARNING("bcg interval less than suggested interval (%lli s)", MIN_SAMPLE_DURATION);
    }

    _bus_control->init();
    _bus_control->spi_power(true);

    _spi->frequency(8000000);

    ThisThread::sleep_for(10ms);

    LSM6DSLSensor imu(_spi, _cs);

    imu.init(NULL);
    imu.set_g_odr(G_FREQUENCY);
    imu.set_g_fs(G_FULL_SCALE);
    imu.enable_g();
    imu.enable_int1_drdy_g();

    ThisThread::sleep_for(500ms); // let gyroscope warm up

    _sample_timer.start();

    while(_sample_timer.elapsed_time() <= duration_cast<microseconds>(num_seconds))
    {
        if (_g_drdy.read())        
        {
            float gyr[3] = {0};
            imu.get_g_axes_f(gyr);
            _g_x.push_back(gyr[0]);
            _g_y.push_back(gyr[1]);
            _g_z.push_back(gyr[2]);           
        }
        ThisThread::sleep_for(5ms);
    }
    // LOG_DEBUG("%s", "BCG data collection complete");
}

void BCG::collect_data(uint16_t num_samples)
{
    _bus_control->init();
    _bus_control->spi_power(true);

    _spi->frequency(8000000);

    ThisThread::sleep_for(10ms);

    LSM6DSLSensor imu(_spi, _cs);

    imu.init(NULL);
    imu.set_g_odr(G_FREQUENCY);
    imu.set_g_fs(G_FULL_SCALE);
    imu.enable_g();
    imu.enable_int1_drdy_g();

    ThisThread::sleep_for(500ms); // let gyroscope warm up

    while(_g_x.size() < num_samples)
    {
        if (_g_drdy.read())        
        {
            float gyr[3] = {0};
            imu.get_g_axes_f(gyr);
            _g_x.push_back(gyr[0]);
            _g_y.push_back(gyr[1]);
            _g_z.push_back(gyr[2]);           
        }
        ThisThread::sleep_for(5ms);
    }

    _bus_control->spi_power(false);
}

uint8_t BCG::calc_hr()
{
    if (_g_x.size() < MIN_SAMPLES)
    {
        LOG_WARNING("fewer samples than recommended (%u of %u)", _g_x.size(), MIN_SAMPLES);
    }

    // printf("pre-filter:\r\n");
    // for (int i = 0; i < _g_x.size(); i++)
    // {
    //     printf("%f, %f, %f\r\n", _g_x.at(i), _g_y.at(i), _g_z.at(i));
    // }

    _step_1_filter(_g_x);
    _step_1_filter(_g_y);
    _step_1_filter(_g_z);

    // printf("post-filter:\r\n");
    // for (int i = 0; i < _g_x.size(); i++)
    // {
    //     printf("%f, %f, %f\r\n", _g_x.at(i), _g_y.at(i), _g_z.at(i));
    // }

    vector<float> l2norm;
    _l2norm(_g_x, _g_y, _g_z, l2norm);

    // for (int i = 0; i < l2norm.size(); i++)
    // {
    //     printf("%f\r\n", l2norm.at(i));
    // }

    _step_2_filter(l2norm);

    // for (int i = 0; i < l2norm.size(); i++)
    // {
    //     printf("%f\r\n", l2norm.at(i));
    // }

    /** throw out the first 128 data points.
     * these are attenuated due to the FIR
     * filters.
     */

    l2norm.erase(l2norm.begin(), l2norm.begin()+128);

    printf("%u", l2norm.size());

    vector<complex<double>> fft;
    fft.resize(l2norm.size());
    _fft.fft(l2norm.data(), fft.data(), (uint16_t)l2norm.size());

    for (int i = 0; i < fft.size() / 2; i++)
    {
        printf("%f, %f\r\n", fft.at(i), 1.0/52.0 * fft.size() * (i+1));
    }

    return 1;
}

void BCG::_step_1_filter(vector<float> &to_filter)
{
    BCGIsolationFilter filt;
    BCGIsolationFilter_init(&filt);

    for (unsigned int i = 0; i < to_filter.size(); i++)
    {
        BCGIsolationFilter_put(&filt, to_filter.at(i));
        to_filter.at(i) = BCGIsolationFilter_get(&filt);
    }
}

void BCG::_step_2_filter(vector<float> &to_filter)
{
    HRFilter filt;
    HRFilter_init(&filt);

    for (unsigned int i = 0; i < to_filter.size(); i++)
    {
        HRFilter_put(&filt, to_filter.at(i));
        to_filter.at(i) = HRFilter_get(&filt);
    }
}

void BCG::_l2norm(vector<float> &x, vector<float> &y, vector<float> &z, vector<float> &l2norm)
{
    while(!x.empty())
    {
        float result = sqrt(x.front() * x.front() + y.front() * y.front() + z.front() * z.front());

        // printf("result = %f, sizes: x = %u, y = %u, z = %u\r\n", result, x.size(), y.size(), z.size());

        x.erase(x.begin());
        y.erase(y.begin());
        z.erase(z.begin());

        l2norm.push_back(result);
    }
}