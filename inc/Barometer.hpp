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
    void get_pressure_buffer(float *pressure_data, uint8_t num_elements);
    void get_temperature_buffer(float *temp_data, uint8_t num_elements);
private:
    bool _initialized = false;
    bool _bar_data_ready = false;
    bool _unread_pressure_data = false;
    bool _unread_temperature_data = false;
    LPS22HB_Data_st _lps22hbData[FIFO_LENGTH] = {{0}, {0}};
    float _pressure_buffer[FIFO_LENGTH] = {{0}};

    BusControl *_bus_control;
    LPS22HBSensor _barometer;
    InterruptIn _int_pin;

    uint8_t last_fifo_status;

    void bar_data_ready();
    bool read_buffered_data();
    bool read_buffered_pressure();
};

#endif //BAROMETER_H_