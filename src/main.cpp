#include "mbed.h"
#include "LowPowerTicker.h"

#include "SWO.h"
#include "SWOLogger.h"

#include "BusControl.h"

#include "FaceBitState.hpp"

FaceBitState facebit;

static events::EventQueue task_queue(16 * EVENTS_EVENT_SIZE);
Thread thread1;

SWO_Channel swo("channel");

void blink()
{
    while(1)
    {
        BusControl::get_instance()->blink_led();
        ThisThread::sleep_for(1s);
    }
}

int main()
{
    BusControl::get_instance()->init();

    swo.claim();
    LOG_INFO("%s", "PROGRAM STARTING");

    thread1.start(blink);

    facebit.run();

    return 0;
}
