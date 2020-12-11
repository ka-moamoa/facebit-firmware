#ifndef BUSCONTROL_H_
#define BUSCONTROL_H_

#include "mbed.h"
#include "Mutex.h"

class BusControl
{
public:
    enum SPIDevices 
    {
        FRAM,
        MAGNETOMETER,
        BAROMETER,
        IMU
    };

    BusControl(BusControl &other) = delete;
    void operator=(const BusControl &) = delete;

    static BusControl* get_instance();

    void spi_power(bool power);
    void i2c_power(bool power);
private:
    BusControl(); //Singleton
    ~BusControl();

    // SPI device power
    DigitalOut _fram_vcc;
    DigitalOut _mag_vcc;
    DigitalOut _bar_vcc;
    DigitalOut _imu_vcc;

    // SPI device CS lines
    DigitalOut _fram_cs;
    DigitalOut _mag_cs;
    DigitalOut _bar_cs;
    DigitalOut _imu_cs; // must be high drive

    // I2C device power
    DigitalOut _temp_vcc; // must be high drive
    DigitalOut _voc_vcc;

    // I2C pullup
    DigitalOut _i2c_pu; // must be high drive

    static BusControl* _instance;
    static Mutex _mutex;
};




#endif // BUSCONTROL_H_