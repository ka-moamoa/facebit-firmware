#include "mbed.h"
#include "rtos.h"
#include "nrf52.h"
#include "LowPowerTicker.h"

#include "SWO.h"
#include "Logger.h"

#include "BusControl.h"
#include "RespiratoryRate.hpp"
#include "CapCalc.h"
#include "FaceBitState.hpp"

#define FACEBIT_SWO
#define TRACE_LEVEL TRACE_TRACE

// #include "mbed_mem_trace.h"

InterruptIn imu_int(IMU_INT1, PullNone);
bool imu_interrupt = false;

SmartPPEService _smart_ppe_ble;
FaceBitState facebit(&_smart_ppe_ble, &imu_interrupt);

Logger* _logger;

#if defined(FACEBIT_SWO)
SWO_Channel swo("channel");
#elif defined(FACEBIT_UART)
static UnbufferedSerial serial(STDIO_UART_TX, NC);

FileHandle *mbed::mbed_override_console(int fd)
{
    return &serial;
}
#endif

void blink()
{
    while(1)
    {
        BusControl::get_instance()->blink_led(10ms);
        ThisThread::sleep_for(2s);
    }
}

void imu_int_handler()
{
    imu_interrupt = true;
}

Thread thread1(osPriorityNormal, 512);

int main() {
    _logger = Logger::get_instance();

#if defined(FACEBIT_SWO)
    _logger->initialize(&swo, TRACE_LEVEL);
#elif defined(FACEBIT_UART)
    _logger->initialize(&serial, TRACE_LEVEL);
#endif

    NRF_POWER->DCDCEN = 1;

    _logger->log(TRACE_DEBUG, "START");

    imu_int.rise(imu_int_handler);
 
    BusControl::get_instance()->init();

    thread1.start(blink);

    facebit.run();

    return 0;
}
