#ifndef FACEBITSTATE_H_
#define FACEBITSTATE_H_

#include "mbed.h"
#include "LSM6DSLSensor.h"
#include "BusControl.h"
#include <vector>

#include "gatt_server_process.h"
#include "SmartPPEService.h"

using namespace std::chrono;

class FaceBitState
{
public:
    FaceBitState(SmartPPEService *smart_ppe_ble, bool *imu_interrupt);
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
        MEASURE_RESPIRATION_RATE,
        MEASURE_HEART_RATE,
        MEASURE_MASK_FIT,
        TASK_STATE_LAST
    };

    enum FACEBIT_DATA_TYPES_t
    {
        HEART_RATE,
        RESPIRATORY_RATE,
        MASK_FIT
    };

    void run();
    void update_state(uint32_t ts);
private:
    SPI _spi;
    I2C _i2c;
    BusControl *_bus_control;

    SmartPPEService* _smart_ppe_ble;
    static Thread _ble_thread;
    static events::EventQueue ble_queue;
    bool _force_update;

    DigitalIn _imu_cs;
    LSM6DSL_Interrupt_Pin_t _wakeup_int_pin = LSM6DSL_INT1_PIN;
    bool *_imu_interrupt;

    struct FaceBitData
    {
        FACEBIT_DATA_TYPES_t data_type;
        uint64_t timestamp;
        uint8_t value;
    };

    vector<FaceBitData> data_buffer;

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

    const uint32_t RR_PERIOD = 5 * 60 * 1000; // 5 min
    const uint32_t HR_PERIOD = 10000;//1 * 60 * 1000; // 1 min
    const uint32_t MASK_FIT_PERIOD = 5000;//1 * 60 * 1000; // 1 min
    const uint32_t BLE_BROADCAST_PERIOD = 20000;//5 * 60 * 1000; // 5 min
    const uint32_t BLE_CONNECTION_TIMEOUT = 5000;
    const uint32_t BLE_DRDY_TIMEOUT = 5000;

    uint32_t ACTIVE_STATE_ENTRY_TS = 0;

    uint64_t _mask_state_change_ts = 0; 

    uint32_t _last_rr_ts = 0;
    uint32_t _last_hr_ts = 0;
    uint32_t _last_mf_ts = 0;
    uint32_t _last_ble_ts = 0;

    bool _ble_initialized = false;

    bool _get_imu_int();
    bool _sync_data(GattServerProcess *_ble_process);
};


#endif // FACEBITSTATE_H_
