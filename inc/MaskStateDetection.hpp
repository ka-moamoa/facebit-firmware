/**
 * @file MaskStateDetection.hpp
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

#ifndef MASKSTATEDETECTION_H_
#define MASKSTATEDETECTION_H_

#include "Barometer.hpp"

class MaskStateDetection
{
public:
    MaskStateDetection(Barometer* barometer);
    ~MaskStateDetection();

    typedef enum 
    {
        ON,
        OFF,
        ERROR
    } MASK_STATE_t;

    MASK_STATE_t is_on();

private:
    Barometer* _barometer;
    Logger* _logger;

    const float DETECTION_WINDOW = 10.0; // seconds
    const int SAMPLING_FREQUENCY = 10; // hz
    const uint16_t ON_THRESHOLD = 10; // 0.15 mbar, this from https://gitlab.com/ka-moamoa/smart-ppe/facebit-companion-ios/-/blob/master/data-exploration/mask-on-off.ipynb
};




#endif // MASKSTATEDETECTION_H_
