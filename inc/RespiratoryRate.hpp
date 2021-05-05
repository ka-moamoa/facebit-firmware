#ifndef RESPIRATORYRATE_H_
#define RESPIRATORYRATE_H_

#include "Si7051.h"
#include "BusControl.h"
#include "Barometer.hpp"
#include "Logger.h"
#include "../iir-filter-kit/BiQuad.h"

using namespace std::chrono;

class RespiratoryRate
{
public:
    typedef struct
    {
        int rate;
        uint64_t timestamp;
    } RR_t;

    typedef enum
    {
        THERMOMETER,
        BAROMETER
    } RespSource_t;

    RespiratoryRate(Si7051 &temp, Barometer &barometer);
    ~RespiratoryRate();

    RR_t get_buffer_element();
    uint8_t get_buffer_size() { return respiratory_rate_buffer.size(); };

    float respiratory_rate(const uint8_t num_seconds, RespSource_t source);
    
private:
    Si7051 &_temp;
    Barometer &_barometer;

    BusControl *_bus_control;
    Logger* _logger;

    vector<RR_t> respiratory_rate_buffer;

    const int8_t ERROR = -1;
    const uint8_t BUFFER = 0; // second
    const uint8_t FREQUENCY = 10; // hz
};


#endif // RESPIRATORYRATE_H_
