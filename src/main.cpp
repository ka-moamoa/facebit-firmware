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

#include "SmartPPEService.h"

#define BUFFER_SIZE 50

DigitalOut led(LED1);
BusControl *bus_control = BusControl::get_instance();

SWO_Channel SWO; // for SWO logging
I2C i2c(I2C_SDA0, I2C_SCL0);
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);

SWO_Channel swo("channel");

using namespace std::literals::chrono_literals;

const static char DEVICE_NAME[] = "SMARTPPE";

static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);

Thread thread1(osPriorityNormal, 256);
Thread thread2;

void led_thread()
{
    while(1)
    {
        led = 0;
        ThisThread::sleep_for(1000ms);
        led = 1;
        ThisThread::sleep_for(10ms);
    }
}

void process_data(float *pressure_data)
{
    // for (int i = 0; i < FIFO_LENGTH; i++)
    // {
    //     LOG_INFO("pressure[%i] = %0.2f", i, pressure_data[i]);
    // }
}

void sensor_thread(SmartPPEService* smart_ppe_service)
{   
    ThisThread::sleep_for(10ms);
    bus_control->init();

    bus_control->spi_power(true);
    Barometer barometer(&spi, BAR_CS, BAR_DRDY);
    
    if (!barometer.initialize() || !barometer.set_fifo_full_interrupt(true))
    {
        LOG_WARNING("%s", "barometer failed to initialize");
        while(1) {};
    }

    if (!barometer.set_pressure_threshold(1010) || !barometer.enable_pressure_threshold(true, true, false))
    {
        LOG_WARNING("%s", "barometer setup failed");
        while(1) {};
    }
    

    while(1)
    {
        if (barometer.update())
        {
            if (barometer.get_high_pressure_event_flag())
            {
                LOG_INFO("%s", "High pressure event detected!");
            }

            if (barometer.get_pressure_buffer_size() > BUFFER_SIZE)
            {
                LOG_DEBUG("%s", "pressure_buffer full!");

                for (uint16_t i = 0; i < barometer.get_pressure_buffer_size(); i++)
                {
                    smart_ppe_service->updatePressure(barometer.get_pressure_buffer_element());
                    smart_ppe_service->updatePressure(barometer.get_temp_buffer_element());
                }
            }
        }
    }

    ThisThread::sleep_for(1ms);
}

int main()
{
    swo.claim();
    BLE &ble = BLE::Instance();
    SmartPPEService smart_ppe_ble;

    thread1.start(led_thread);
    thread2.start(callback(sensor_thread, &smart_ppe_ble));

    GattServerProcess ble_process(event_queue, ble);
    ble_process.on_init(callback(&smart_ppe_ble, &SmartPPEService::start));

    ble_process.start();

    return 0;
}

