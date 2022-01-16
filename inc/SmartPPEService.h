/**
 * @file SmartPPEService.h
 * @author Alexander Curtiss apcurtiss@gmail.com
 * @brief 
 * @version 0.1
 * @date 2022-01-16
 * 
 * @copyright Copyright (c) 2022 Ka Moamoa
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SMART_PPE_SERVICE_H_
#define SMART_PPE_SERVICE_H_

#include "mbed.h"
#include "events/mbed_events.h"
#include "ble/BLE.h"

class SmartPPEService : ble::GattServer::EventHandler {

    const char* PRESSURE_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8781";
    const char* TEMPERATURE_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8782";
    const char* DATA_READY_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8783";
    const char* RESPIRATORY_RATE_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8784";
    const char* BCG_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8785";
    const char* ON_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8786";
    const char* TIME_UUID = "0F1F34A3-4567-484C-ACA2-CC8F662E8787";

public:
    enum data_ready_t
    {
        PRESSURE = 1,
        TEMPERATURE = 2,
        ACCELEROMETER = 3,
        RESPIRATORY_RATE = 4,
        MASK_ON = 5,
        COUGH_SAMPLE = 6,
        HEART_RATE = 7,
        NO_DATA = 8
    };

    SmartPPEService()
    {
        const UUID pressure_uuid(PRESSURE_UUID);
        const UUID temp_uuid(TEMPERATURE_UUID);
        const UUID rr_uuid(RESPIRATORY_RATE_UUID);
        const UUID bcg_uuid(BCG_UUID);
        const UUID on_uuid(ON_UUID);
        const UUID data_ready_uuid(DATA_READY_UUID);
        const UUID time_uuid(TIME_UUID);

        _pressure = new ReadOnlyArrayGattCharacteristic<uint8_t, 213> (pressure_uuid, &_initial_value_uint8_t);
        if (!_pressure) {
            printf("Allocation of pressure characteristic failed\r\n");
        }

        _temperature = new ReadOnlyArrayGattCharacteristic<uint8_t, 213> (temp_uuid, &_initial_value_uint8_t);
        if (!_temperature) {
            printf("Allocation of temperature characteristic failed\r\n");
        }

        _respiratory_rate = new ReadOnlyArrayGattCharacteristic<uint8_t, 10> (rr_uuid, &_initial_value_uint8_t);
        if (!_respiratory_rate) {
            printf("Allocation of magnetometer characteristic failed\r\n");
        }

        _bcg = new ReadOnlyArrayGattCharacteristic<uint8_t, 10> (bcg_uuid, &_initial_value_uint8_t);
        if (!_bcg) {
            printf("Allocation of imu characteristic failed\r\n");
        }

        _mask_on = new ReadOnlyArrayGattCharacteristic<uint8_t, 10> (on_uuid, &_initial_value_uint8_t);
        if (!_mask_on) {
            printf("Allocation of mic characteristic failed\r\n");
        }

        _data_ready = new ReadWriteGattCharacteristic<uint8_t> (data_ready_uuid, &_initial_value_data_ready, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
        if (!_data_ready) {
            printf("Allocation of data quality characteristic failed\r\n");
        }

        _time = new ReadWriteGattCharacteristic<uint64_t> (time_uuid, &_initial_value_uint64_t);
        if (!_time) {
            printf("Allocation of time characteristic failed\r\n");
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
            _mask_on,
            _data_ready,
            _time};

        GattService smart_ppe_service(uuid, charTable, 7);

        _server = &ble.gattServer();

        _server->addService(smart_ppe_service);

        _server->setEventHandler(this);

        printf("FaceBit service added with UUID 6243fabc-23e9-4b79-bd30-1dc57b8005d6\r\n");
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

    void updateRespiratoryRate(uint64_t data_timestamp, uint16_t respiratory_rate)
    {
        uint8_t bytearray[10] = {0};
        uint64_t timestamp = data_timestamp;
        std::memcpy(bytearray, &timestamp, 8);

        uint16_t value = respiratory_rate;
        std::memcpy(&bytearray[8], &value, 2);

        _server->write(_respiratory_rate->getValueHandle(), bytearray, 10);
    }

    void updateHeartRate(uint64_t data_timestamp, uint16_t heart_rate)
    {
        uint8_t bytearray[10] = {0};
        uint64_t timestamp = data_timestamp;
        std::memcpy(bytearray, &timestamp, 8);

        uint16_t value = heart_rate;
        std::memcpy(&bytearray[8], &value, 2);

        _server->write(_bcg->getValueHandle(), bytearray, 10);
    }

    void updateMaskOn(uint64_t data_timestamp, uint16_t mask_on)
    {
        uint8_t bytearray[10] = {0};
        uint64_t timestamp = data_timestamp;
        std::memcpy(bytearray, &timestamp, 8);

        uint16_t value = mask_on;
        std::memcpy(&bytearray[8], &value, 2);

        _server->write(_mask_on->getValueHandle(), bytearray, 10);
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

    void updateTime(uint64_t epoch_time)
    {
        uint8_t bytearray[8] = {0};

        uint64_t time = epoch_time;
        std::memcpy(bytearray, &time, 8);

        _server->write(_time->getValueHandle(), bytearray, 8);
    }

    uint64_t getTime()
    {
        uint16_t length = 8;
        uint8_t epoch_time_array[8];

        _server->read(_time->getValueHandle(), epoch_time_array, &length);
        uint64_t epoch_time = 0;
        std::memcpy(&epoch_time, epoch_time_array, 8);

        return epoch_time;
    }

private:
    GattServer* _server = nullptr;

    ReadOnlyArrayGattCharacteristic<uint8_t, 213>* _pressure = nullptr;
    ReadOnlyArrayGattCharacteristic<uint8_t, 213>* _temperature = nullptr;
    ReadOnlyArrayGattCharacteristic<uint8_t, 10>* _respiratory_rate = nullptr;
    ReadOnlyArrayGattCharacteristic<uint8_t, 10>* _bcg = nullptr;
    ReadOnlyArrayGattCharacteristic<uint8_t, 10>* _mask_on = nullptr;
    ReadWriteGattCharacteristic<uint8_t>* _data_ready = nullptr;
    ReadWriteGattCharacteristic<uint64_t>* _time = nullptr;

    uint8_t _initial_value_data_ready = NO_DATA;
    uint8_t _initial_value_uint8_t = 0;
    uint16_t _initial_value_uint16_t = 0;
    uint32_t _initial_value_uint32_t = 0;
    uint64_t _initial_value_uint64_t = 0;
};

#endif // SMART_PPE_SERVICE_H_
