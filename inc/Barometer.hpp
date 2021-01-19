#ifndef BAROMETER_H_
#define BAROMETER_H_

#include "LPS22HBSensor.h"
#include "SWOLogger.h"
#include "BusControl.h"

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

    void get_pressure_buffer(float *pressure_data, uint8_t num_elements);
    void get_temperature_buffer(float *temp_data, uint8_t num_elements);

    bool get_high_pressure_event_flag() { return _high_pressure_event_flag; };
private:
    bool _initialized = false;
    bool _bar_data_ready = false;
    bool _unread_pressure_data = false;
    bool _unread_temperature_data = false;
    LPS22HB_Data_st _lps22hbData[FIFO_LENGTH] = {{0}, {0}};
    bool _high_pressure_event_flag = false;

    BusControl *_bus_control;
    LPS22HBSensor _barometer;
    InterruptIn _int_pin;


    void bar_data_ready();
    bool read_buffered_data();
};

#endif //BAROMETER_H_