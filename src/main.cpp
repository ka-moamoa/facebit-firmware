/* mbed Microcontroller Library
 * Copyright (c) 2017-2019 ARM Limited
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

#include "SWO.h"
#include "SWOLogger.h"

#include "SensorService.h"
#include "ble/BLE.h"

#include "SPI.h"
#include "I2C.h"

#include "BusControl.h"
#include "Si7051.h"
#include "LPS22HBSensor.h"

SWO_Channel SWO("channel");
I2C i2c(I2C_SDA0, I2C_SCL0);
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);

BusControl *bus_control = BusControl::get_instance();

LPS22HBSensor barometer(&spi, BAR_CS);
Si7051 temp(&i2c);

float readBarometer()
{
    bus_control->spi_power(true);
    ThisThread::sleep_for(10ms);

    barometer.init(NULL);
    barometer.enable();

    ThisThread::sleep_for(50ms);

    float pressure = 0;
    barometer.get_pressure(&pressure);

    barometer.disable();

    bus_control->spi_power(false);

    return pressure;
}

float readThermometer()
{
    bus_control->i2c_power(true);
    ThisThread::sleep_for(100ms);

    temp.initialize();
    ThisThread::sleep_for(10ms);
    float temperature = temp.readTemperature();
    LOG_DEBUG("temperature = %0.3f C", temperature);

    bus_control->i2c_power(false);

    return temperature;
}

int main() {
    SWO.claim();

    BLE &ble = BLE::Instance();
    events::EventQueue event_queue;
    SensorService demo_service;

    while(1)
    {
        float pressure = readBarometer();
        demo_service.updateBarometer(pressure);
        ThisThread::sleep_for(50ms);
        float temperature = readThermometer();
        demo_service.updateThermometer(temperature);
        ThisThread::sleep_for(50ms);
    }

    /* this process will handle basic ble setup and advertising for us */
    GattServerProcess ble_process(event_queue, ble);

    /* once it's done it will let us continue with our demo */
    ble_process.on_init(callback(&demo_service, &SensorService::start));

    ble_process.start();



    return 0;
}
