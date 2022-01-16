/**
 * @file Barometer.hpp
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

#ifndef BAROMETER_H_
#define BAROMETER_H_

#include "LPS22HBSensor.h"
#include "Logger.h"
#include "BusControl.h"

class Barometer
{
public:
    Barometer(SPI *spi, PinName cs_pin, PinName int_pin);
    ~Barometer();

    const uint16_t MAX_ALLOWABLE_SIZE = 200; //This is a little arbitrary, just want to have a cap on the buffer size.

    bool initialize();
    bool set_frequency(uint8_t frequency);
    uint8_t get_frequency() { return _frequency; }
    bool update(bool force = false);

    bool set_fifo_full_interrupt(bool enable);
    bool enable_pressure_threshold(bool enable, bool high_pressure, bool low_pressure);
    bool set_pressure_threshold(int16_t hPa);

    bool get_high_pressure_event_flag() { return _high_pressure_event_flag; };

    void set_max_buffer_size(uint16_t size);
    uint16_t get_max_buffer_size() { return _max_buffer_size; };
    bool get_buffer_full() { return _pressure_buffer.size() >= _max_buffer_size; };

    uint8_t get_pressure_buffer_size() { return _pressure_buffer.size(); };
    uint8_t get_temp_buffer_size() { return _temperature_buffer.size(); };
    uint16_t* get_pressure_array() { return _pressure_buffer.data(); };
    uint16_t* get_temperature_array() { return _temperature_buffer.data(); };
    void clear_buffers() { _temperature_buffer.clear(); _pressure_buffer.clear(); };
    uint64_t get_delta_timestamp(bool broadcast);
    uint32_t get_measurement_frequencyx100() { return _measurement_frequencyx100; };

    float convert_to_hpa(uint16_t raw_data);

private:
    bool _initialized = false;
    bool _bar_data_ready = false;
    std::vector<uint16_t> _pressure_buffer;
    std::vector<uint16_t> _temperature_buffer;
    bool _high_pressure_event_flag = false;
    uint16_t _max_buffer_size = 96; // by default
    uint64_t _drdy_timestamp;
    uint64_t _last_timestamp = 0;
    uint64_t _last_broadcast_timestamp = 0;
    uint32_t _measurement_frequencyx100;
    uint8_t _frequency = 24.0; // default value, can change

    BusControl *_bus_control;
    LPS22HBSensor _barometer;
    InterruptIn _int_pin;
    LowPowerTimer _t_barometer;

    Logger* _logger;

    void bar_data_ready();
    bool read_buffered_data();

    const uint8_t BAROMETER_FIFO_SIZE = 32;
};

#endif //BAROMETER_H_