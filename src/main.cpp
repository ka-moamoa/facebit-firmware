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
#include "FRAM.h"
#include "Si7051.h"
#include "LPS22HBSensor.h"
#include "LSM6DSLSensor.h"
#include "CapCalc.h"

#include "SmartPPEService.h"

DigitalOut led(LED1);
BusControl *bus_control = BusControl::get_instance();

SWO_Channel SWO; // for SWO logging
I2C i2c(I2C_SDA0, I2C_SCL0);
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);

CapCalc cap_monitor(VCAP, VCAP_ENABLE, 3000);

SWO_Channel swo("channel");

using namespace std::literals::chrono_literals;

const static char DEVICE_NAME[] = "SMARTPPE";

static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);

Thread thread1;
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


// /* Schedule processing of events from the BLE middleware in the event queue. */
// void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
// {
//     event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
// }

void sensor_thread(SmartPPEService* smart_ppe_service)
{   
    ThisThread::sleep_for(1s);
    bus_control->init();

    while(1)
    {
        bus_control->spi_power(true);
        ThisThread::sleep_for(10ms);

        LPS22HBSensor barometer(&spi, BAR_CS);
        LSM6DSLSensor imu(&spi, IMU_CS);
        FRAM fram(&spi, FRAM_CS);
        Si7051 temp(&i2c);

        barometer.init(NULL);
        barometer.enable();

        imu.init(NULL);
        imu.enable_x();

        ThisThread::sleep_for(100ms);

        float pressure = 0;
        barometer.get_pressure(&pressure);
        LOG_DEBUG("pressure = %0.3f mbar", pressure);
        smart_ppe_service->updatePressure((uint32_t)(pressure*100));

        int32_t data[3] = { 0 };
        imu.get_x_axes(data);
        LOG_DEBUG("imu data: x = %li, y = %li, z = %li", data[0], data[1], data[2]);

        bus_control->spi_power(false);

        bus_control->i2c_power(true);
        ThisThread::sleep_for(100ms);

        temp.initialize();
        ThisThread::sleep_for(10ms);
        float temperature = temp.readTemperature();
        LOG_DEBUG("temperature = %0.3f C", temperature);
        smart_ppe_service->updateTemperature((uint32_t)(temperature*10000));

        bus_control->i2c_power(false);

        float voltage = cap_monitor.read_capacitor_voltage();
        LOG_DEBUG("capacitor voltage = %0.3f V", voltage);

        float joules = cap_monitor.calc_joules();
        LOG_DEBUG("energy = %f J", joules);

        ThisThread::sleep_for(100ms);
    }
}

int main()
{
    swo.claim();
    BLE &ble = BLE::Instance();
    // ble.onEventsToProcess(schedule_ble_events);

    SmartPPEService smart_ppe_ble;

    thread1.start(led_thread);
    thread2.start(callback(sensor_thread, &smart_ppe_ble));

    GattServerProcess ble_process(event_queue, ble);
    ble_process.on_init(callback(&smart_ppe_ble, &SmartPPEService::start));

    ble_process.start();

    return 0;
}

