#include "mbed.h"
#include "LowPowerTicker.h"

#include "SWO.h"
#include "SWOLogger.h"

#include "BusControl.h"
#include "RespiratoryRate.hpp"
#include "CapCalc.h"
#include "FaceBitState.hpp"

// #include "mbed_mem_trace.h"

InterruptIn imu_int(IMU_INT1);
bool imu_interrupt = false;

SmartPPEService _smart_ppe_ble;
FaceBitState facebit(&_smart_ppe_ble, &imu_interrupt);

SWO_Channel swo("channel");

void blink()
{
    while(1)
    {
        BusControl::get_instance()->blink_led();
        ThisThread::sleep_for(1s);
    }
}

void imu_int_handler()
{
<<<<<<< HEAD
    imu_interrupt = true;
=======
    while(1)
    {
        rr.get_resp_rate();
        ThisThread::sleep_for(5s);
    }
>>>>>>> resp-rate-application
}

Thread thread1(osPriorityNormal, 512);

int main(){
    swo.claim();

    LOG_INFO("%s", "PROGRAM STARTING");

    imu_int.rise(imu_int_handler);
 
    BusControl::get_instance()->init();

    thread1.start(blink);

    facebit.run();

    return 0;
}
