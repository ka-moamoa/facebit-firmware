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

const float MIN_VOLTAGE = 2.3;

const int LED_ENERGY = 0.001;
const int SENSING_ENERGY = 0.001;

const bool RUN_LED = false;
const bool RUN_SENSING = false;

// Frequency configuration of each task
 const std::chrono::milliseconds LED_TASK= 1000ms;
 const std::chrono::milliseconds SENSING_TASK= 1000ms;

LowPowerTicker lp_ticker_led,lp_ticker_sensor;

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

Mutex stdio_mutex;
Thread thread1;
Thread thread2;
EventFlags event_flags;
float cap_voltage = 0;

void check_voltage()
{
    available_energy = cap.calc_joules();
    cap_voltage = cap.read_capacitor_voltage();
    printf("\r\nVoltage: %f\r\n",cap_voltage);
    printf("Energy: %f\r\n",available_energy);
}

void led_thread()
{
    check_voltage();
    printf("\r\nLED\r\n");
    if (RUN_LED || (available_energy > LED_ENERGY))
    {
        led = !led;
    }
    fflush(stdout);
}
void sensor_thread(/*SmartPPEService* smart_ppe_service*/)
{
    check_voltage();
    printf("Sensing\n\r");
    if (RUN_SENSING || (available_energy > SENSING_ENERGY && cap_voltage > MIN_VOLTAGE))
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
    fflush(stdout);
}

void t1()
{
    event_queue.call(led_thread);
}
void t2()
{
    event_queue.call(sensor_thread);
}

int main()
{
    swo.claim();
    // BLE &ble = BLE::Instance();
    // SmartPPEService smart_ppe_ble;
    
    lp_ticker_led.attach(t1, 1000ms);
    lp_ticker_sensor.attach(t2, 1000ms);

    //thread1.start(check_voltage);
    
    thread2.start(callback(&event_queue, &EventQueue::dispatch_forever));
    //event_queue.dispatch();
    //GattServerProcess ble_process(event_queue, ble);
    //ble_process.on_init(callback(&smart_ppe_ble, &SmartPPEService::start));
    // ble_process.start();

    return 0;
}
