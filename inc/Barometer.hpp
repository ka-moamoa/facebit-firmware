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

    void set_max_buffer_size(uint16_t size);
    uint16_t get_max_buffer_size() { return _max_buffer_size; };
    bool get_buffer_full() { return _pressure_buffer.size() >= _max_buffer_size; };

    uint16_t get_pressure_buffer_size() { return _pressure_buffer.size(); };
    uint16_t get_temp_buffer_size() { return _temperature_buffer.size(); };
    uint16_t* get_pressure_array() { return _pressure_buffer.data(); };
    uint16_t* get_temperature_array() { return _temperature_buffer.data(); };
    void clear_buffers() { _temperature_buffer.clear(); _pressure_buffer.clear(); };
private:
    bool _initialized = false;
    bool _bar_data_ready = false;
    LPS22HB_Data_st _lps22hbData[FIFO_LENGTH] = {{0}, {0}};
    std::vector<uint16_t> _pressure_buffer;
    std::vector<uint16_t> _temperature_buffer;
    bool _high_pressure_event_flag = false;
    uint16_t _max_buffer_size = 100; // by default

    BusControl *_bus_control;
    LPS22HBSensor _barometer;
    InterruptIn _int_pin;

    void bar_data_ready();
    bool read_buffered_data();
};

#endif //BAROMETER_H_