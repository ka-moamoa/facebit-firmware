#ifndef CAPCALC_H_
#define CAPCALC_H_

#include "mbed.h"

class CapCalc
{
public:
    static CapCalc* get_instance();

    float read_voltage();
    float calc_joules();

    CapCalc(CapCalc &other) = delete;
    void operator=(const CapCalc &) = delete;

private:
    CapCalc();
    ~CapCalc();

    static CapCalc *_instance;
    static Mutex _mutex;

    AnalogIn _cap_voltage;
    DigitalOut _cap_voltage_en;
    uint32_t _capacitance_uF = 3000;
};

#endif // CAPCALC_H_