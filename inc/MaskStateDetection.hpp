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

    const float DETECTION_WINDOW = 5.0;
    const uint16_t ON_THRESHOLD = 15; // 0.15 mbar, this from https://gitlab.com/ka-moamoa/smart-ppe/facebit-companion-ios/-/blob/master/data-exploration/mask-on-off.ipynb
};




#endif // MASKSTATEDETECTION_H_
