/**
 * @file CapCalc.cpp
 * @author Saad Ahmed and Alexander Curtiss apcurtiss@gmail.com
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

#include "CapCalc.h"
#include "nrfx_saadc.h"
#include "PinNames.h"

CapCalc* CapCalc::_instance = nullptr; 
Mutex CapCalc::_mutex; 

CapCalc::CapCalc() :
_cap_voltage_en(VCAP_ENABLE),
_cap_voltage(VCAP)
{
    _cap_voltage_en = 0;

    nrf_saadc_channel_config_t channel_config = 
    {
        .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
        .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
        .gain       = NRF_SAADC_GAIN1_4,
        .reference  = NRF_SAADC_REFERENCE_INTERNAL, // this is set to vdd/4 by mbed, changing to internal reference here
        .acq_time   = NRF_SAADC_ACQTIME_10US,
        .mode       = NRF_SAADC_MODE_SINGLE_ENDED,
        .burst      = NRF_SAADC_BURST_DISABLED,
        .pin_p      = NRF_SAADC_INPUT_AIN1,
        .pin_n      = NRF_SAADC_INPUT_DISABLED
    };

    ret_code_t result = nrfx_saadc_channel_init(1, &channel_config);
    MBED_ASSERT(result == NRFX_SUCCESS);

    _cap_voltage.set_reference_voltage(2.4); // this is internal reference / gain = 0.6 / (1/4)
}

CapCalc::~CapCalc()
{
    _cap_voltage_en = 0;
}

CapCalc* CapCalc::get_instance()
{
    _mutex.lock();

    if (_instance == nullptr)
    {
        _instance = new CapCalc();
    }

    _mutex.unlock();

    return _instance;
}

float CapCalc::read_voltage()
{
    _cap_voltage_en = 1;
    ThisThread::sleep_for(1ms);

    float voltage = _cap_voltage.read_voltage() * 2.80; // 2.80 is a factor that arises from the voltage divider
    
    _cap_voltage_en = 0;

    return voltage;
}

float CapCalc::calc_joules()
{
    float voltage = read_voltage();

    float joules = 0.5 * ((float)_capacitance_uF / 1000000.0) * (voltage * voltage - 1.8*1.8);//the point at which the processor can run

    return joules;
}