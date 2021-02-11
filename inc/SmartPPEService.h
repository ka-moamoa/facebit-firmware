#ifndef SMART_PPE_SERVICE_H_
#define SMART_PPE_SERVICE_H_

#include "mbed.h"
#include "events/mbed_events.h"
#include "ble/BLE.h"

class SmartPPEService : ble::GattServer::EventHandler {

    const char* PRESSURE_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8781";
    const char* TEMPERATURE_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8782";
    const char* DATA_READY_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8783";
    const char* MAGNETOMETER_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8784";
    const char* IMU_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8785";
    const char* MICROPHONE_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8786";

public:
    enum data_ready_t
    {
        PRESSURE = 1,
        TEMPERATURE = 2,
        ACCELEROMETER = 3,
        RESPIRATORY_RATE = 4,
        MASK_FIT = 5,
        COUGH_SAMPLE = 6,
        HEART_RATE = 7,
        NO_DATA = 8
    };

    SmartPPEService()
    {
        const UUID pressure_uuid(PRESSURE_UUID);
        const UUID temp_uuid(TEMPERATURE_UUID);
        const UUID rr_uuid(MAGNETOMETER_UUID);
        const UUID bcg_uuid(IMU_UUID);
        const UUID fit_uuid(MICROPHONE_UUID);
        const UUID data_ready_uuid(DATA_READY_UUID);

        _pressure = new ReadOnlyArrayGattCharacteristic<uint8_t, 213> (pressure_uuid, &_initial_value_uint8_t);
        if (!_pressure) {
            printf("Allocation of pressure characteristic failed\r\n");
        }

        _temperature = new ReadOnlyArrayGattCharacteristic<uint8_t, 213> (temp_uuid, &_initial_value_uint8_t);
        if (!_temperature) {
            printf("Allocation of temperature characteristic failed\r\n");
        }

        _respiratory_rate = new ReadOnlyArrayGattCharacteristic<uint8_t, 9> (rr_uuid, &_initial_value_uint8_t);
        if (!_respiratory_rate) {
            printf("Allocation of magnetometer characteristic failed\r\n");
        }

        _bcg = new ReadOnlyArrayGattCharacteristic<uint8_t, 9> (bcg_uuid, &_initial_value_uint8_t);
        if (!_bcg) {
            printf("Allocation of imu characteristic failed\r\n");
        }

        _fit = new ReadOnlyArrayGattCharacteristic<uint8_t, 9> (fit_uuid, &_initial_value_uint8_t);
        if (!_fit) {
            printf("Allocation of mic characteristic failed\r\n");
        }

        _data_ready = new ReadOnlyGattCharacteristic<uint8_t> (data_ready_uuid, &_initial_value_uint8_t);
        if (!_data_ready) {
            printf("Allocation of data quality characteristic failed\r\n");
        }
    }

    ~SmartPPEService()
    {
    }

    void start(BLE &ble, events::EventQueue &event_queue)
    {
        const UUID uuid = "6243fabc-23e9-4b79-bd30-1dc57b8005d6";
        GattCharacteristic* charTable[] = { 
            _pressure,  
            _temperature,
            _respiratory_rate,
            _bcg,
            _fit,
            _data_ready };
        GattService smart_ppe_service(uuid, charTable, 6);

        _server = &ble.gattServer();

        _server->addService(smart_ppe_service);

        _server->setEventHandler(this);

        printf("Example service added with UUID 6243fabc-23e9-4b79-bd30-1dc57b8005d6\r\n");
    }

    void updatePressure(uint64_t data_timestamp, uint32_t measurement_frequencyx100, uint16_t *pressure_array, uint8_t size)
    {
        if (size > 100)
        {
            size = 100;
        }

        uint8_t bytearray[208] = {0};
        uint64_t timestamp = data_timestamp;
        std::memcpy(bytearray, &timestamp, 8);

        uint32_t tmp_frequency = measurement_frequencyx100;
        std::memcpy(&bytearray[8], &tmp_frequency, 4);

        uint8_t num_samples = size;
        std::memcpy(&bytearray[12], &num_samples, 1);

        for (int i = 0; i < size; i++)
        {
            bytearray[13 + i*2] = (uint8_t)((pressure_array[i] >> 8) & 0xFF);
            bytearray[13 + (i*2)+1] = (uint8_t)(pressure_array[i] & 0xFF);
        }
        _server->write(_pressure->getValueHandle(), bytearray, (size * 2) + 13);
    }

    void updateTemperature(uint64_t data_timestamp, uint32_t measurement_frequencyx100, uint16_t *temperature_array, uint8_t size)
    {
        if (size > 100)
        {
            size = 100;
        }

        uint8_t bytearray[208] = {0};
        uint64_t timestamp = data_timestamp;
        std::memcpy(bytearray, &timestamp, 8);

        uint32_t tmp_frequency = measurement_frequencyx100;
        std::memcpy(&bytearray[8], &tmp_frequency, 4);

        uint8_t num_samples = size;
        std::memcpy(&bytearray[12], &num_samples, 1);

        for (int i = 0; i < size; i++)
        {
            bytearray[13 + i*2] = (uint8_t)((temperature_array[i] >> 8) & 0xFF);
            bytearray[13 + (i*2)+1] = (uint8_t)(temperature_array[i] & 0xFF);
        }
        _server->write(_temperature->getValueHandle(), bytearray, (size * 2) + 13);
    }

    void updateRespiratoryRate(uint64_t data_timestamp, uint8_t respiratory_rate)
    {
        uint8_t bytearray[9] = {0};
        uint64_t timestamp = data_timestamp;
        std::memcpy(bytearray, &timestamp, 8);

        uint8_t rr = respiratory_rate;
        std::memcpy(&bytearray[8], &rr, 1);

        _server->write(_respiratory_rate->getValueHandle(), bytearray, 9);
    }

    void updateDataReady(data_ready_t type)
    {
        uint8_t tmp = (uint8_t)type;
        _server->write(_data_ready->getValueHandle(), &tmp, 1);
    }

    data_ready_t getDataReady()
    {
        uint16_t length = 1;
        uint8_t data_ready = -1;
        _server->read(_data_ready->getValueHandle(), &data_ready, &length);

        return static_cast<data_ready_t>( data_ready );
    }

private:
    GattServer* _server = nullptr;

    ReadOnlyArrayGattCharacteristic<uint8_t, 213>* _pressure = nullptr;
    ReadOnlyArrayGattCharacteristic<uint8_t, 213>* _temperature = nullptr;
    ReadOnlyArrayGattCharacteristic<uint8_t, 9>* _respiratory_rate = nullptr;
    ReadOnlyArrayGattCharacteristic<uint8_t, 9>* _bcg = nullptr;
    ReadOnlyArrayGattCharacteristic<uint8_t, 9>* _fit = nullptr;
    ReadOnlyGattCharacteristic<uint8_t>* _data_ready = nullptr;

    uint8_t _initial_value_uint8_t = 0;
    uint16_t _initial_value_uint16_t = 0;
    uint32_t _initial_value_uint32_t = 0;
};

#endif // SMART_PPE_SERVICE_H_