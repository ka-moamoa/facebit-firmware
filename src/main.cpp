#include "mbed.h"
#include "LowPowerTicker.h"

#include "SWO.h"
#include "SWOLogger.h"

#include "BusControl.h"

#include "FaceBitState.hpp"

#include "mbed_mem_trace.h"

// #include "gatt_server_process.h"
// #include "SmartPPEService.h"

FaceBitState facebit;


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

int main()
{
    swo.claim();
    mbed_mem_trace_set_callback(mbed_mem_trace_default_callback);

    LOG_INFO("%s", "PROGRAM STARTING");
 
    BusControl::get_instance()->init();

    Thread thread1(osPriorityNormal, 512);
    thread1.start(blink);

    facebit.run();

    return 0;
}
