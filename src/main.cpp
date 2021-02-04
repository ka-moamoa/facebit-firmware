/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gatt_server_process.h"

#include "PinNames.h"
#include "mbed.h"

#include "SPI.h"
#include "I2C.h"
#include "SWO.h"
#include "SWOLogger.h"

#include "BusControl.h"
#include "Barometer.hpp"
#include "LSM6DSLSensor.h"

#include "SmartPPEService.h"

DigitalOut led(LED1);
BusControl *bus_control = BusControl::get_instance();

SWO_Channel SWO; // for SWO logging
// I2C i2c(I2C_SDA0, I2C_SCL0);
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);

SWO_Channel swo("channel");

using namespace std::literals::chrono_literals;

// const static char DEVICE_NAME[] = "SMARTPPE";

// static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

Thread *thread1;
Thread *thread2;

Timer timer;
Timer timer2;

// SmartPPEService smart_ppe_ble;

void led_thread()
{
    while(1)
    {
        led = 0;
        ThisThread::sleep_for(2000ms);
        led = 1;
        ThisThread::sleep_for(5ms);
    }
}

void sensor_thread()
{   


    // Barometer barometer(&spi, BAR_CS, BAR_DRDY);


    // float x_odr, g_odr;
    // imu.get_x_odr(&x_odr);
    // imu.get_g_odr(&g_odr);
    // printf("x_odr = %0.2f, g_odr = %0.2f\r\n", x_odr, g_odr);


    // if (!barometer.initialize() || !barometer.set_fifo_full_interrupt(true))
    // {
    //     LOG_WARNING("%s", "barometer failed to initialize");
    //     while(1) {};
    // }

    // if (!barometer.set_pressure_threshold(1100) || !barometer.enable_pressure_threshold(true, true, false))
    // {
    //     LOG_WARNING("%s", "barometer setup failed");
    //     while(1) {};
    // }


    //     if (barometer.update())
    //     {
    //         if (barometer.get_buffer_full())
    //         {
    //             LOG_DEBUG("buffer full. %u elements. timestamp = %llu. measurement frequency x100 = %lu", barometer.get_pressure_buffer_size(), barometer.get_drdy_timestamp(), barometer.get_measurement_frequencyx100());
    //             smart_ppe_ble.updatePressure(
    //                 barometer.get_drdy_timestamp(), 
    //                 barometer.get_measurement_frequencyx100(), 
    //                 barometer.get_pressure_array(), 
    //                 barometer.get_pressure_buffer_size());
    //             smart_ppe_ble.updateTemperature(
    //                 barometer.get_drdy_timestamp(), 
    //                 barometer.get_measurement_frequencyx100(),
    //                 barometer.get_temperature_array(), 
    //                 barometer.get_temp_buffer_size());
    //             smart_ppe_ble.updateDataReady(true);
    //             barometer.clear_buffers();
    //         }
    //     }
    //     ThisThread::sleep_for(200ms);
    // }
}

int main()
{
    swo.claim();

    ThisThread::sleep_for(10ms);

    bus_control->init();
    bus_control->spi_power(true);

    spi.frequency(8000000);

    ThisThread::sleep_for(10ms);

    LSM6DSLSensor imu(&spi, IMU_CS);

    imu.init(NULL);
    imu.set_x_odr(208.0);
    imu.set_g_odr(208.0);
    imu.set_x_fs(1.0);
    imu.set_g_fs(124.0);
    imu.enable_x();
    imu.enable_g();

    timer.start();
    timer2.start();
    const int sample_rate = 100; // Hz
    float sample_period = 1.0 / sample_rate;

    printf("acc X, acc Y, acc Z, gyr X, gyr Y, gyr Z, TS\r\n");

    while(1)
    {
        if (timer.read() >= sample_period)        
        {
            float acc[3] = {0};
            imu.get_x_axes_f(acc);

            float gyr[3] = {0};
            imu.get_g_axes_f(gyr);

            printf("%f, %f, %f, %f, %f, %f, %f\r\n", acc[0], acc[1], acc[2], gyr[0], gyr[1], gyr[2], timer2.read());
            
            timer.reset();
        }
    }



    // thread1 = new Thread(osPriorityNormal, 512);
    // thread2 = new Thread();

    // BLE &ble = BLE::Instance();

    // thread1->start(led_thread);
    // thread2->start(sensor_thread);

    // GattServerProcess ble_process(event_queue, ble);
    // ble_process.on_init(callback(&smart_ppe_ble, &SmartPPEService::start));

    // ble_process.start();

    return 0;
}

