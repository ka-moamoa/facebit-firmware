#ifndef RESPIRATORYRATE_H_
#define RESPIRATORYRATE_H_

#include "Si7051.h"
#include "BusControl.h"
#include "CapCalc.h"

class RespiratoryRate
{
public:
    RespiratoryRate(CapCalc *cap, Si7051 &temp);
    ~RespiratoryRate();

    typedef struct
    {
        int rate;
        uint64_t timestamp;
    } RR_t;
    
    typedef struct
    {
        int breath_count;
        uint64_t duration;
    } RR_d;

    void get_resp_rate();
    
private:
    CapCalc *_cap;
    Si7051 &_temp;
    BusControl *_bus_control;

    vector<RR_t> respiratory_rate_buffer;

    RespiratoryRate::RR_d calc_resp_rate(float samples[], int SAMPLE_SIZE, float mean);

    const int SENSING_ENERGY = 0.001;
    const float MIN_VOLTAGE = 2.3;

};


#endif // RESPIRATORYRATE_H_
