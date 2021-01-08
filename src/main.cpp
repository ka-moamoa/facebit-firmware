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

#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "gatt_server_process.h"

#include "BusControl.h"
#include "Si7051.h"
#include "LPS22HBSensor.h"
#include "FRAM.h"
#include "LSM6DSLSensor.h"

#include "SPI.h"

#include "PinNames.h"
#include "SWO.h"

SWO_Channel swo("channel");

using namespace std::literals::chrono_literals;

const static char DEVICE_NAME[] = "SMARTPPE";

static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);

Thread thread1;
Thread thread2;
DigitalOut led(LED1);
BusControl *bus_control = BusControl::get_instance();

void led_thread()
{
    while(1)
    {
        led = !led;
        ThisThread::sleep_for(100ms);
    }
}

class SmartPPEService : ble::GattServer::EventHandler {

    const char* PRESSURE_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8781";
    const char* TEMPERATURE_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8782";
    const char* AIR_QUALITY_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8783";
    const char* MAGNETOMETER_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8784";
    const char* IMU_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8785";
    const char* MICROPHONE_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8786";

public:
    SmartPPEService()
    {
        const UUID pressure_uuid(PRESSURE_UUID);
        const UUID temp_uuid(TEMPERATURE_UUID);
        const UUID air_q_uuid(AIR_QUALITY_UUID);
        const UUID mag_uuid(MAGNETOMETER_UUID);
        const UUID imu_uuid(IMU_UUID);
        const UUID mic_uuid(MICROPHONE_UUID);

        _pressure_characteristic = new ReadOnlyGattCharacteristic<uint32_t> (pressure_uuid, &p_initial_value,  GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        if (!_pressure_characteristic) {
            printf("Allocation of pressure characteristic failed\r\n");
        }

        _temperature_characteristic = new ReadOnlyGattCharacteristic<uint16_t> (temp_uuid, &initial_value, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        if (!_temperature_characteristic) {
            printf("Allocation of temperature characteristic failed\r\n");
        }

        _air_quality_characteristic = new ReadOnlyGattCharacteristic<uint16_t> (air_q_uuid, &initial_value, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        if (!_air_quality_characteristic) {
            printf("Allocation of air quality characteristic failed\r\n");
        }

        _mag_characteristic = new ReadOnlyGattCharacteristic<uint16_t> (mag_uuid, &initial_value, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        if (!_mag_characteristic) {
            printf("Allocation of magnetometer characteristic failed\r\n");
        }

        _imu_characteristic = new ReadOnlyGattCharacteristic<uint16_t> (imu_uuid, &initial_value, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        if (!_imu_characteristic) {
            printf("Allocation of imu characteristic failed\r\n");
        }

        _mic_characteristic = new ReadOnlyGattCharacteristic<uint16_t> (mic_uuid, &initial_value, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        if (!_mic_characteristic) {
            printf("Allocation of mic characteristic failed\r\n");
        }
    }

    ~SmartPPEService()
    {
    }

    void start(BLE &ble, events::EventQueue &event_queue)
    {
        const UUID uuid = "6243fabc-23e9-4b79-bd30-1dc57b8005d6";
        GattCharacteristic* charTable[] = { 
            _pressure_characteristic,  
            _temperature_characteristic,
            _air_quality_characteristic,
            _mag_characteristic,
            _imu_characteristic,
            _mic_characteristic };
        GattService smart_ppe_service(uuid, charTable, 6);

        _server = &ble.gattServer();

        _server->addService(smart_ppe_service);

        _server->setEventHandler(this);

        printf("Example service added with UUID 6243fabc-23e9-4b79-bd30-1dc57b8005d6\r\n");
    }

    void updatePressure(uint32_t pressurex100)
    {
        const uint8_t new_pressure[4] = {
            (pressurex100 >>24) & 0xFF,
            (pressurex100 >>16) & 0xFF,
            (pressurex100 >>8) & 0xFF,
            (pressurex100) & 0xFF };

        _server->write(_pressure_characteristic->getValueHandle(), new_pressure, 4);
    }

private:
    GattServer* _server = nullptr;

    ReadOnlyGattCharacteristic<uint32_t>* _pressure_characteristic = nullptr;
    ReadOnlyGattCharacteristic<uint16_t>* _temperature_characteristic = nullptr;
    ReadOnlyGattCharacteristic<uint16_t>* _air_quality_characteristic = nullptr;
    ReadOnlyGattCharacteristic<uint16_t>* _imu_characteristic = nullptr;
    ReadOnlyGattCharacteristic<uint16_t>* _mag_characteristic = nullptr;
    ReadOnlyGattCharacteristic<uint16_t>* _mic_characteristic = nullptr;

    uint16_t initial_value = 0;
    uint32_t p_initial_value = 0;
};

// /* Schedule processing of events from the BLE middleware in the event queue. */
// void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
// {
//     event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
// }

void sensor_thread(SmartPPEService* smart_ppe_service)
{
    SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);
    LPS22HBSensor barometer(&spi, BAR_CS);
    bus_control->init();
    bus_control->spi_power(true);
    ThisThread::sleep_for(200ms);
    barometer.init(NULL);
    barometer.enable();
    
    while(1)
    {
        ThisThread::sleep_for(200ms);
        float pressure = 0;
        barometer.get_pressure(&pressure);
        printf("pressure = %0.2f mbar\r\n", pressure);
        smart_ppe_service->updatePressure((uint32_t)(pressure*100));
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

