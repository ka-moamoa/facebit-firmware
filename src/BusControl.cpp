/**
 * @file BusControl.cpp
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

#include "BusControl.h"
#include "rtos.h"
#include "nrf51_to_nrf52.h"
#include "nrf52_bitfields.h"
#include "TARGET_SMARTPPE/PinNames.h"

BusControl* BusControl::_instance = nullptr;
Mutex BusControl::_mutex;

BusControl::BusControl() :
_fram_vcc(FRAM_VCC),
_mag_vcc(MAG_VCC),
_bar_vcc(BAR_VCC),
_imu_vcc(IMU_VCC),
_fram_cs(FRAM_CS),
_mag_cs(MAG_CS),
_bar_cs(BAR_CS),
_imu_cs(IMU_CS),
_temp_vcc(TEMP_VCC),
_voc_vcc(VOC_VCC),
_i2c_pu(I2C_PULLUP),
_led(LED1)
{
    _logger = Logger::get_instance();
}

BusControl::~BusControl() {}

BusControl* BusControl::get_instance()
{
    _mutex.lock();

    if (_instance == nullptr)
    {
        _instance = new BusControl();
    }

    _mutex.unlock();

    return _instance;
}

void BusControl::init(void)
{
    if (!_initialized)
    {
        NRF_GPIO->PIN_CNF[IMU_VCC] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
        NRF_GPIO->PIN_CNF[TEMP_VCC] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
        NRF_GPIO->PIN_CNF[BAR_VCC] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
        NRF_GPIO->PIN_CNF[I2C_PULLUP] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
        NRF_GPIO->PIN_CNF[FRAM_VCC] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
        NRF_GPIO->PIN_CNF[MAG_VCC] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
        NRF_GPIO->PIN_CNF[LED1] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    
        power_lock.fram = false;
        power_lock.magnetometer = false;
        power_lock.barometer = false;
        power_lock.imu = false;

        _spi_power = false;
        _i2c_power = false;
    }

    _initialized = true;
}

void BusControl::set_power_lock(Devices device, bool lock)
{
    switch(device)
    {
        case FRAM:
            power_lock.fram = lock;
            break;
        case MAGNETOMETER:
            power_lock.magnetometer = lock;
            break;
        case BAROMETER:
            power_lock.barometer = lock;
            break;
        case IMU:
            power_lock.imu = lock;
            break;
        case THERMOMETER:
            power_lock.thermometer = lock;
            break;
        case VOC:
            power_lock.voc = lock;
            break;
    }
}

void BusControl::spi_power(bool power)
{
    if (!_initialized)
    {
        _logger->log(TRACE_WARNING, "%s", "Bus Control has not been initialized. Please run init().");
        return;
    }

    if (!power_lock.fram)
    {
        _fram_vcc = power;
        _fram_cs = power;
    }

    if (!power_lock.barometer)
    {
        _bar_vcc = power;
        _bar_cs = power;
    }

    if (!power_lock.magnetometer)
    {
        _mag_vcc = power;
        _mag_cs = power;
    }

    if (!power_lock.imu)
    {
        _imu_vcc = power;
        _imu_cs = power;
    }

    _spi_power = power;
}


void BusControl::i2c_power(bool power)
{
    if (!_initialized)
    {
        _logger->log(TRACE_WARNING, "%s", "Bus Control has not been initialized. Please run init().");
        return;
    }

    _temp_vcc = power;
    _voc_vcc = power;
    _i2c_pu = power;

    _i2c_power = power;
}

void BusControl::blink_led(milliseconds length)
{
    for (int i = 0; i < _num_blinks; i++)
    {
        _led = 1;
        ThisThread::sleep_for(length);
        _led = 0;
        ThisThread::sleep_for(250ms);
    }
}

bool BusControl::get_spi_power()
{
    return _spi_power;
}

bool BusControl::get_i2c_power()
{
    return _i2c_power;
}