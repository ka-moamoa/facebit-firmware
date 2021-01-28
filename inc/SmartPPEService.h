#ifndef SMART_PPE_SERVICE_H_
#define SMART_PPE_SERVICE_H_

#include "mbed.h"
#include "events/mbed_events.h"
#include "ble/BLE.h"

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

        _pressure_characteristic = new ReadOnlyArrayGattCharacteristic<uint16_t, 100> (pressure_uuid, &_initial_value_uint16_t);
        if (!_pressure_characteristic) {
            printf("Allocation of pressure characteristic failed\r\n");
        }

        _temperature_characteristic = new ReadOnlyArrayGattCharacteristic<uint16_t, 100> (temp_uuid, &_initial_value_uint16_t);
        if (!_temperature_characteristic) {
            printf("Allocation of temperature characteristic failed\r\n");
        }

        _air_quality_characteristic = new ReadOnlyGattCharacteristic<uint16_t> (air_q_uuid, &_initial_value_uint16_t, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        if (!_air_quality_characteristic) {
            printf("Allocation of air quality characteristic failed\r\n");
        }

        _mag_characteristic = new ReadOnlyGattCharacteristic<uint16_t> (mag_uuid, &_initial_value_uint16_t, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        if (!_mag_characteristic) {
            printf("Allocation of magnetometer characteristic failed\r\n");
        }

        _imu_characteristic = new ReadOnlyGattCharacteristic<uint16_t> (imu_uuid, &_initial_value_uint16_t, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        if (!_imu_characteristic) {
            printf("Allocation of imu characteristic failed\r\n");
        }

        _mic_characteristic = new ReadOnlyGattCharacteristic<uint16_t> (mic_uuid, &_initial_value_uint16_t, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
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

    void updatePressure(uint16_t *pressure_array, uint16_t size)
    {
        _server->write(_pressure_characteristic->getValueHandle(), (uint8_t*)pressure_array, size);
    }

    void updateTemperature(uint16_t *temperature_array, uint16_t size)
    {
        _server->write(_temperature_characteristic->getValueHandle(), (uint8_t*)temperature_array, size);
    }

    void updateDataReady(bool ready)
    {
        uint8_t tmp = ready;
        _server->write(_air_quality_characteristic->getValueHandle(), &tmp, 1);
    }

private:
    GattServer* _server = nullptr;

    ReadOnlyArrayGattCharacteristic<uint16_t, 100>* _pressure_characteristic = nullptr;
    ReadOnlyArrayGattCharacteristic<uint16_t, 100>* _temperature_characteristic = nullptr;
    ReadOnlyGattCharacteristic<uint16_t>* _air_quality_characteristic = nullptr;
    ReadOnlyGattCharacteristic<uint16_t>* _imu_characteristic = nullptr;
    ReadOnlyGattCharacteristic<uint16_t>* _mag_characteristic = nullptr;
    ReadOnlyGattCharacteristic<uint16_t>* _mic_characteristic = nullptr;

    uint16_t _initial_value_uint16_t = 0;
    uint32_t _initial_value_uint32_t = 0;
};

#endif // SMART_PPE_SERVICE_H_