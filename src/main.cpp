#include "mbed.h"
#include "LowPowerTicker.h"

#include "SWO.h"
#include "SWOLogger.h"

#include "BusControl.h"
#include "RespiratoryRate.hpp"
#include "CapCalc.h"
#include "FaceBitState.hpp"

FaceBitState facebit;
CapCalc *cap = CapCalc::get_instance();
I2C i2c(I2C_SDA0, I2C_SCL0);
Si7051 temp(&i2c);
RespiratoryRate rr(*cap,temp);

static events::EventQueue task_queue(16 * EVENTS_EVENT_SIZE);
Thread thread1,thread2;

SWO_Channel swo("channel");

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
    while(1)
    {
        rr.get_resp_rate();
        ThisThread::sleep_for(5s);
    }
}

int main()
{
    BusControl::get_instance()->init();

    swo.claim();
    LOG_INFO("%s", "PROGRAM STARTING");

    //thread1.start(blink);
    thread2.start(resp_rate);

    //facebit.run();

    return 0;
}
