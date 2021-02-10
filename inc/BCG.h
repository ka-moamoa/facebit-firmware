#ifndef BCG_H_
#define BCG_H_

#include "mbed.h"
#include "LSM6DSLSensor.h"
#include "BusControl.h"
#include <vector>
#include "../iir-filter-kit/BiQuad.h"

class BCG
{
public:
    BCG(SPI *spi, PinName int1_pin, PinName cs);
    ~BCG();

    float bcg(const uint16_t num_samples);
    float get_frequency() { return G_FREQUENCY; }
private:
    BusControl *_bus_control;
    SPI *_spi;
    DigitalIn _g_drdy;
    PinName _cs;
    LowPowerTimer _sample_timer;

    vector<float> _bcg;

    float G_FREQUENCY = 104.0; // Hz
    float G_FULL_SCALE = 124.0; // max sensitivity

    float BCG_STEP_DUR = 2.0; // seconds
    float HR_STEP_DUR = 2.5; // seconds

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

// void _l2norm(vector<float> &x, vector<float> &y, vector<float> &z, vector<float> &l2norm);