/**
 * @file main.cpp
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

#include "mbed.h"
#include "rtos.h"
#include "nrf52.h"
#include "LowPowerTicker.h"

#include "SWO.h"
#include "Logger.h"

#include "BusControl.h"
#include "RespiratoryRate.hpp"
#include "CapCalc.h"
#include "FaceBitState.hpp"

#define FACEBIT_UART
// #define FACEBIT_SWO
#define TRACE_LEVEL TRACE_INFO

// #include "mbed_mem_trace.h"

InterruptIn imu_int(IMU_INT1, PullNone);
bool imu_interrupt = false;

SmartPPEService _smart_ppe_ble;
FaceBitState facebit(&_smart_ppe_ble, &imu_interrupt);

Logger* _logger;

#if defined(FACEBIT_SWO)
SWO_Channel swo("channel");
#elif defined(FACEBIT_UART)
static UnbufferedSerial serial(STDIO_UART_TX, NC);

FileHandle *mbed::mbed_override_console(int fd)
{
    return &serial;
}
#endif

void blink()
{
    BusControl::get_instance()->set_led_blinks(1);
    while(1)
    {
        BusControl::get_instance()->blink_led(10ms);
        ThisThread::sleep_for(2s);
    }
}

void imu_int_handler()
{
    imu_interrupt = true;
}

Thread thread1(osPriorityNormal, 512);

int main() {
    _logger = Logger::get_instance();

#if defined(FACEBIT_SWO)
    _logger->initialize(&swo, TRACE_LEVEL);
#elif defined(FACEBIT_UART)
    _logger->initialize(&serial, TRACE_LEVEL);
#endif

    NRF_POWER->DCDCEN = 1;

    _logger->log(TRACE_DEBUG, "START");

    imu_int.rise(imu_int_handler);
 
    BusControl::get_instance()->init();

    thread1.start(blink);

    facebit.run();

    return 0;
}
