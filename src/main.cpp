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
#include "CapCalc.h"

#include "BusControl.h"
#include "Barometer.hpp"

#include "SmartPPEService.h"

const int LED_ENERGY = 1;
const int SENSING_ENERGY = 1;

const bool RUN_LED = true;
const bool RUN_SENSING =false;

LowPowerTicker ticker1,ticker2;
CapCalc cap(VCAP, VCAP_ENABLE, 3000);
float available_energy = 0;

DigitalOut led(LED1);
BusControl *bus_control = BusControl::get_instance();

SWO_Channel SWO; // for SWO logging
I2C i2c(I2C_SDA0, I2C_SCL0);
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);

SWO_Channel swo("channel");

using namespace std::literals::chrono_literals;

const static char DEVICE_NAME[] = "SMARTPPE";

static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);

Thread thread1;
Thread thread2;


void led_thread()
{
    available_energy = cap.calc_joules();
    if (RUN_LED || (available_energy > LED_ENERGY))
    {
        led = !led;
    }
}
void sensor_thread(/*SmartPPEService* smart_ppe_service*/)
{
    available_energy = cap.calc_joules();
    float cap_voltage = cap.read_capacitor_voltage();
    if(RUN_SENSING || (available_energy > SENSING_ENERGY && cap_voltage > 2.3))
    {
        ThisThread::sleep_for(10ms);
        bus_control->init();

        bus_control->spi_power(true);
        Barometer barometer(&spi, BAR_CS, BAR_DRDY);
        if (!barometer.initialize())
        {
            LOG_WARNING("%s", "barometer failed to initialize");
            while (1)
            {
            };
        }
        if (barometer.update())
        {
            float pressure_data[FIFO_LENGTH];
            float temperature_data[FIFO_LENGTH];

            barometer.get_pressure_buffer(pressure_data, FIFO_LENGTH);
            barometer.get_temperature_buffer(temperature_data, FIFO_LENGTH);

            for (int i = 0; i < FIFO_LENGTH; i++)
            {
                printf("pressure[%i] = %f, temperature[%i] = %f\r\n", i, pressure_data[i], i, temperature_data[i]);
            }
        }
    }
}

int main()
{
    //swo.claim();
    // BLE &ble = BLE::Instance();
    // SmartPPEService smart_ppe_ble;

    ticker1.attach(led_thread, 1000ms);
    ticker2.attach(sensor_thread,1000ms /*callback(sensor_thread, &smart_ppe_ble)*/);

    // GattServerProcess ble_process(event_queue, ble);
    // ble_process.on_init(callback(&smart_ppe_ble, &SmartPPEService::start));

    // ble_process.start();

    return 0;
}
