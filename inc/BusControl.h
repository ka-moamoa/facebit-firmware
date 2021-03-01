#ifndef BUSCONTROL_H_
#define BUSCONTROL_H_

#include "mbed.h"
#include "Mutex.h"
#include "Logger.h"

class BusControl
{
public:
    enum SPIDevices 
    {
        FRAM,
        MAGNETOMETER,
        BAROMETER,
        IMU,
        SPI_LAST
    };

    BusControl(BusControl &other) = delete;
    void operator=(const BusControl &) = delete;

    static BusControl* get_instance();

    void init(void);

    void set_power_lock(SPIDevices device, bool lock);

    void spi_power(bool power);
    void i2c_power(bool power);
    
    void blink_led();

    bool get_spi_power();
    bool get_i2c_power();
private:
    BusControl(); //Singleton
    ~BusControl();

    Logger* _logger;

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

    DigitalOut _led; // must be high drive

    // Initialized flag
    bool _initialized = false;

    struct power_lock_t
    {
        bool fram;
        bool magnetometer;
        bool barometer;
        bool imu;
    };

    power_lock_t power_lock;

    static BusControl* _instance;
    static Mutex _mutex;

    const float LED_ENERGY = 0.001;

    bool _spi_power = false;
    bool _i2c_power = false;
};




#endif // BUSCONTROL_H_