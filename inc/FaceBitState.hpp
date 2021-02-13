#ifndef FACEBITSTATE_H_
#define FACEBITSTATE_H_

#include "mbed.h"
#include "LSM6DSLSensor.h"
#include "BusControl.h"

using namespace std::chrono;

class FaceBitState
{
public:
    FaceBitState();
    ~FaceBitState();

    enum MASK_STATE_t
    {
        OFF_FACE_INACTIVE,
        OFF_FACE_ACTIVE,
        ON_FACE,
        MASK_STATE_LAST
    };

    enum TASK_STATE_t
    {
        IDLE,
        RESPIRATION_RATE,
        HEART_RATE,
        MASK_FIT,
        BLE_BROADCAST,
        TASK_STATE_LAST
    };

    void run();
    void update_state();
private:
    SPI _spi;
    I2C _i2c;
    BusControl *_bus_control;

    DigitalIn _imu_cs;
    InterruptIn _imu_int1;
    LSM6DSL_Interrupt_Pin_t _wakeup_int_pin = LSM6DSL_INT1_PIN;
    bool _imu_interrupt = false;

    MASK_STATE_t _mask_state = MASK_STATE_LAST;
    MASK_STATE_t _next_mask_state = OFF_FACE_INACTIVE;
    bool _new_mask_state = true;

    TASK_STATE_t _task_state = IDLE;
    TASK_STATE_t _next_task_state = IDLE;
    bool _new_task_state = false;

    milliseconds _sleep_duration = 1000ms;

    milliseconds INACTIVE_SLEEP_DURATION = 10000ms;
    milliseconds ACTIVE_SLEEP_DURATION = 5000ms;
    uint32_t ACTIVE_STATE_TIMEOUT = 60000;
    milliseconds ON_FACE_SLEEP_DURATION = 1000ms;


    void imu_int_handler();
    bool get_imu_int();

    LowPowerTimer _mask_state_timer;
};


#endif // FACEBITSTATE_H_
