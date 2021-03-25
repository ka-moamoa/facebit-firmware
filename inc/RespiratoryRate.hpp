#ifndef RESPIRATORYRATE_H_
#define RESPIRATORYRATE_H_

#include "Si7051.h"
#include "BusControl.h"
#include "CapCalc.h"
#include "Logger.h"

class RespiratoryRate
{
public:
    typedef struct
    {
        int rate;
        uint64_t timestamp;
    } RR_t;

    RespiratoryRate(CapCalc *cap, Si7051 &temp);
    ~RespiratoryRate();

    RR_t get_buffer_element();
    uint8_t get_buffer_size() { return respiratory_rate_buffer.size(); };

    bool get_resp_rate();
    
private:
    CapCalc *_cap;
    Si7051 &_temp;
    BusControl *_bus_control;
    LowPowerTimer _temp_timer;
    Logger* _logger;

    vector<RR_t> respiratory_rate_buffer;

    float calc_resp_rate(float samples[], int SAMPLE_SIZE, float mean);

    const int SENSING_ENERGY = 0.001;
    const float MIN_VOLTAGE = 2.3;

};


#endif // RESPIRATORYRATE_H_
