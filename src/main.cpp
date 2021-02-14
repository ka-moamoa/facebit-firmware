#include "mbed.h"
#include "LowPowerTicker.h"

#include "SWO.h"
#include "SWOLogger.h"

#include "BusControl.h"
#include "RespiratoryRate.hpp"
#include "CapCalc.h"
#include "FaceBitState.hpp"

#include "mbed_mem_trace.h"

// #include "gatt_server_process.h"
// #include "SmartPPEService.h"

FaceBitState facebit;
CapCalc *cap = CapCalc::get_instance();
I2C i2c(I2C_SDA0, I2C_SCL0);
Si7051 temp(&i2c);
RespiratoryRate rr(*cap,temp);

<<<<<<< HEAD
=======
static events::EventQueue task_queue(16 * EVENTS_EVENT_SIZE);
Thread thread1,thread2;
>>>>>>> resp-rate-application

SWO_Channel swo("channel");

// SmartPPEService smart_ppe_ble;

void blink()
{
    while(1)
    {
        BusControl::get_instance()->blink_led();
        ThisThread::sleep_for(1s);
    }
}
void resp_rate()
{
    //while(1)
    {
        rr.get_resp_rate();
        ThisThread::sleep_for(20s);
    }
}

int main()
{
    swo.claim();
    mbed_mem_trace_set_callback(mbed_mem_trace_default_callback);

    LOG_INFO("%s", "PROGRAM STARTING");
 
    BusControl::get_instance()->init();

<<<<<<< HEAD
    Thread thread1(osPriorityNormal, 512);
    thread1.start(blink);
=======
    //thread1.start(blink);
    thread2.start(resp_rate);
>>>>>>> resp-rate-application

    //facebit.run();

    return 0;
}
