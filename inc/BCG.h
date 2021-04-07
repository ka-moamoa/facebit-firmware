#ifndef BCG_H_
#define BCG_H_

#include "mbed.h"
#include "LSM6DSLSensor.h"
#include "BusControl.h"
#include "../iir-filter-kit/BiQuad.h"

using namespace std::chrono;

class BCG
{
public:
    typedef struct
    {
        uint8_t rate;
        time_t timestamp;
    } HR_t;

    BCG(SPI *spi, PinName int1_pin, PinName cs);
    ~BCG();

    bool bcg(const seconds num_seconds);
    float get_frequency() { return G_FREQUENCY; }

    uint8_t get_buffer_size() { return _HR.size(); };
    HR_t get_buffer_element();

private:
    BusControl *_bus_control;
    SPI *_spi;
    DigitalIn _g_drdy;
    PinName _cs;
    LowPowerTimer _sample_timer;

    Logger* _logger;

    const uint8_t HR_BUFFER_SIZE = 20; // how many heart rates we want to store on device
    vector<HR_t> _HR;

    const float G_FREQUENCY = 51.0; // Hz
    const float G_FULL_SCALE = 124.0; // max sensitivity

    /**
     * These next two variables will control
     * our "bcg valid" detection. NUM_EVENTS
     * describes how many sequential samples
     * we want to have a std deviation below
     * STD_DEV_THRESHOLD before we calculate 
     * a heart rate based on them.
     */
    uint8_t NUM_EVENTS = 4; // number of sequential events
    float STD_DEV_THRESHOLD = 5.0; // in BPM

    uint8_t MIN_HR = 45; // BPM below this limit are filtered out during the HR_isolation stage
    uint8_t MAX_HR = 150; // BPM above this limit are filtered out during the HR_isolation stage

    double _l2norm(double x, double y, double z);
};

#endif //BCG_H_

/*DINOSAURS
   *                               *     _
        /\     *            ___.       /  `)
    *  //\\    /\          ///\\      / /
      ///\\\  //\\/\      ////\\\    / /     /\
     ////\\\\///\\/\\.-~~-.///\\\\  / /     //\\
    /////\\\\///\\/         `\\\\\\/ /     ///\\
   //////\\\\// /            `\\\\/ /     ////\\
  ///////\\\\\//               `~` /\    /////\\
 ////////\\\\\/      ,_____,   ,-~ \\\__//////\\\
 ////////\\\\/  /~|  |/////|  |\\\\\\\\@//jro/\\
 //<           / /|__|/////|__|///////~|~/////\\
 ~~~     ~~   ` ~   ..   ~  ~    .     ~` `   '.
 ~ _  -  -~.    .'   .`  ~ .,    '.    ~~ .  '*/


// uint8_t calc_hr();

// void _l2norm(vector<float> &x, vector<float> &y, vector<float> &z, vector<float> &l2norm);'

// const float BCG_STEP_DUR = 2.5; // seconds
// const float HR_STEP_DUR = 2.5; // seconds