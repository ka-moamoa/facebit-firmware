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
#include "Si7051.h"

#include "SmartPPEService.h"

DigitalOut led(LED1);
BusControl *bus_control = BusControl::get_instance();

SWO_Channel SWO; // for SWO logging
I2C i2c(I2C_SDA0, I2C_SCL0);
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);

Si7051 temp(&i2c);
Barometer barometer(&spi, BAR_CS, BAR_DRDY);

SWO_Channel swo("channel");

using namespace std::literals::chrono_literals;

const static char DEVICE_NAME[] = "SMARTPPE";

static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);


Thread *thread1;
Thread *thread2;

SmartPPEService smart_ppe_ble;

void led_thread()
{
    while(1)
    {
        led = 0;
        ThisThread::sleep_for(2000ms);
        led = 1;
        ThisThread::sleep_for(10ms);
    }
}

void sensor_thread()
{   
    bus_control->spi_power(true);
    spi.frequency(5000000);

    bus_control->i2c_power(true);

    ThisThread::sleep_for(1000ms);

    temp.initialize();
    
    if (!barometer.initialize() || !barometer.set_fifo_full_interrupt(true))
    {
        LOG_WARNING("%s", "barometer failed to initialize");
        while(1) {};
    }

    if (!barometer.set_pressure_threshold(1100) || !barometer.enable_pressure_threshold(true, true, false))
    {
        LOG_WARNING("%s", "barometer setup failed");
        while(1) {};
    }

    while(1)
    {
        barometer.update();
        temp.update();

        if (barometer.get_buffer_full())
        {
            LOG_DEBUG("barometer buffer full. %u elements. timestamp = %llu. measurement frequency x100 = %lu", barometer.get_pressure_buffer_size(), barometer.get_drdy_timestamp(), barometer.get_measurement_frequencyx100());
            
            smart_ppe_ble.updatePressure(
                barometer.get_drdy_timestamp(), 
                barometer.get_measurement_frequencyx100(), 
                barometer.get_pressure_array(), 
                barometer.get_pressure_buffer_size());
            
            smart_ppe_ble.updateDataReady(smart_ppe_ble.PRESSURE);


            barometer.clear_buffers();
        }

        if (temp.getBufferFull())
        {
            uint32_t measurement_frequencyx100 = temp.getFrequency() * 100.0;

            LOG_DEBUG("temperature buffer full. %u elements. timestamp = %llu. measurement frequency x100 = %lu", temp.getBufferSize(), temp.getLastMeasurementTimestamp(), measurement_frequencyx100);

            smart_ppe_ble.updateTemperature(
                temp.getLastMeasurementTimestamp(), 
                measurement_frequencyx100,
                temp.getBuffer(),
                temp.getBufferSize());

            smart_ppe_ble.updateDataReady(smart_ppe_ble.TEMPERATURE);

            temp.clearBuffer();
        }

        ThisThread::sleep_for(1ms);
    }
}

int main()
{
    swo.claim();

    bus_control->init();
    ThisThread::sleep_for(10ms);

    thread1 = new Thread(osPriorityNormal, 512);
    thread2 = new Thread();

    BLE &ble = BLE::Instance();

    thread1->start(led_thread);
    thread2->start(sensor_thread);

    GattServerProcess ble_process(event_queue, ble);
    ble_process.on_init(callback(&smart_ppe_ble, &SmartPPEService::start));

    ble_process.start();

    return 0;
}

