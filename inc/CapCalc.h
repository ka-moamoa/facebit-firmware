/**
 * @file CapCalc.h
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