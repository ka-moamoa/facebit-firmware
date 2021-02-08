#ifndef BCG_H_
#define BCG_H_

#include "mbed.h"
#include "LSM6DSLSensor.h"
#include "BusControl.h"
#include <vector>
#include "fft.hpp"

class BCG
{
public:
    BCG(SPI *spi, PinName int1_pin, PinName cs);
    ~BCG();

    void collect_data(std::chrono::seconds num_seconds);
    void collect_data(uint16_t num_samples);
    uint8_t calc_hr();

private:
    BusControl *_bus_control;
    SPI *_spi;
    DigitalIn _g_drdy;
    PinName _cs;
    LowPowerTimer _sample_timer;
    FFT _fft;

    vector<float> _g_x;
    vector<float> _g_y;
    vector<float> _g_z;

    std::chrono::seconds MIN_SAMPLE_DURATION = 5s; //seconds
    uint16_t MIN_SAMPLES = 5 * 52;
    float G_FREQUENCY = 52.0;
    float G_FULL_SCALE = 124.0;

    void _step_1_filter(vector<float> &to_filter);
    void _l2norm(vector<float> &x, vector<float> &y, vector<float> &z, vector<float> &l2norm);
    void _step_2_filter(vector<float> &to_filter);
};

#endif //BCG_H_