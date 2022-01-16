/**
 * @file BusControl.h
 * @author Alexander Curtiss apcurtiss@gmail.com
 * @brief 
 * @version 0.1
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022 Ka Moamoa
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef BUSCONTROL_H_
#define BUSCONTROL_H_

#include "mbed.h"
#include "Mutex.h"
#include "Logger.h"

using namespace std::chrono;

class BusControl
{
public:
    enum Devices 
    {
        FRAM,
        MAGNETOMETER,
        BAROMETER,
        IMU,
        THERMOMETER,
        VOC,
        DEVICES_LAST
    };

    BusControl(BusControl &other) = delete;
    void operator=(const BusControl &) = delete;

    static BusControl* get_instance();

    void init(void);

    void set_power_lock(Devices device, bool lock);

    void spi_power(bool power);
    void i2c_power(bool power);
    
    void blink_led(milliseconds length);
    void set_led_blinks(uint8_t num_blinks) { _num_blinks = num_blinks; };

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
    uint8_t _num_blinks = 1;

    // Initialized flag
    bool _initialized = false;

    struct power_lock_t
    {
        bool fram;
        bool magnetometer;
        bool barometer;
        bool imu;
        bool thermometer;
        bool voc;
    };

    power_lock_t power_lock;

    static BusControl* _instance;
    static Mutex _mutex;

    const float LED_ENERGY = 0.001;

    bool _spi_power = false;
    bool _i2c_power = false;
};




#endif // BUSCONTROL_H_