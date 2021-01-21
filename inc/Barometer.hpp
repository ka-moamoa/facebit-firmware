#ifndef BAROMETER_H_
#define BAROMETER_H_

#include "LPS22HBSensor.h"
#include "SWOLogger.h"
#include "BusControl.h"
#include <queue>

class Barometer
{
public:
    Barometer(SPI *spi, PinName cs_pin, PinName int_pin);
    ~Barometer();

    bool initialize();
    bool update();

    bool set_fifo_full_interrupt(bool enable);
    bool enable_pressure_threshold(bool enable, bool high_pressure, bool low_pressure);
    bool set_pressure_threshold(int16_t hPa);

    bool get_high_pressure_event_flag() { return _high_pressure_event_flag; };

    uint32_t get_pressure_buffer_element();
    uint32_t get_temp_buffer_element();
    uint16_t get_pressure_buffer_size() { return _pressure_buffer.size(); };
    uint16_t get_temp_buffer_size() { return _temperature_buffer.size(); };
private:
    bool _initialized = false;
    bool _bar_data_ready = false;
    LPS22HB_Data_st _lps22hbData[FIFO_LENGTH] = {{0}, {0}};
    std::queue<uint32_t> _pressure_buffer;
    std::queue<uint32_t> _temperature_buffer;
    bool _high_pressure_event_flag = false;

    BusControl *_bus_control;
    LPS22HBSensor _barometer;
    InterruptIn _int_pin;

    void bar_data_ready();
    bool read_buffered_data();
};

#endif //BAROMETER_H_