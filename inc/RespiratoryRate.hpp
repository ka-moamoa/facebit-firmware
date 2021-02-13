#ifndef RESPIRATORYRATE_H_
#define RESPIRATORYRATE_H_

#include "Si7051.h"
#include "BusControl.h"
#include "CapCalc.h"

class RespiratoryRate
{
public:
    RespiratoryRate(CapCalc &cap, Si7051 &temp);
    ~RespiratoryRate();

    typedef struct
    {
        int rate;
        uint64_t timestamp;
    } RR_t;
    

    int calc_resp_rate(float samples[], int SAMPLE_SIZE, float mean);
    void get_resp_rate();
    
private:
    CapCalc &_cap;
    Si7051 &_temp;
    BusControl *bus_control;

    vector<RR_t> respiratory_rate_buffer;

    const int SENSING_ENERGY = 0.001;
    const float MIN_VOLTAGE = 2.3;

};


#endif // RESPIRATORYRATE_H_
