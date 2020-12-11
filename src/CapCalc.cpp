#include "CapCalc.h"
#include "nrfx_saadc.h"

CapCalc::CapCalc(PinName cap_voltage, PinName cap_voltage_en, uint32_t capacitance_uF) :
_cap_voltage(cap_voltage),
_cap_voltage_en(cap_voltage_en),
_capacitance_uF(capacitance_uF)
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

float CapCalc::read_capacitor_voltage()
{
    _cap_voltage_en = 1;
    ThisThread::sleep_for(1ms);

    float voltage = _cap_voltage.read_voltage() * 2.80; // 2.80 is a factor that arises from the voltage divider
    
    _cap_voltage_en = 0;

    return voltage;
}

float CapCalc::calc_joules()
{
    float voltage = read_capacitor_voltage();

    float joules = 0.5 * ((float)_capacitance_uF / 1000000.0) * voltage * voltage;

    return joules;
}