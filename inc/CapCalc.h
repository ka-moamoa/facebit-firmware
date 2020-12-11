#ifndef CAPCALC_H_
#define CAPCALC_H_

#include "mbed.h"

class CapCalc
{
private:
    AnalogIn _cap_voltage;
    DigitalOut _cap_voltage_en;
    uint32_t _capacitance_uF;

public:
    CapCalc(PinName cap_voltage, PinName cap_voltage_en, uint32_t capacitance_uF);
    ~CapCalc();
    
    float read_capacitor_voltage();
    float calc_joules();
};

#endif // CAPCALC_H_