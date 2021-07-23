#ifndef FACEBITSTATE_H_
#define FACEBITSTATE_H_

#include "mbed.h"
#include "LSM6DSLSensor.h"
#include "BusControl.h"
#include <vector>

#include "gatt_server_process.h"
#include "SmartPPEService.h"
#include "Logger.h"
#include "FRAM.h"

using namespace std::chrono;

class FaceBitState
{
public:
    FaceBitState(SmartPPEService *smart_ppe_ble, bool *imu_interrupt);
    ~FaceBitState();

    enum MASK_STATE_t
    {
        OFF_FACE,
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
    void update_state();
private:
    SPI _spi;
    I2C _i2c;
    BusControl* _bus_control;
    Logger* _logger;
    // FRAM _fram;
    LowPowerTimer _state_timer;


    SmartPPEService* _smart_ppe_ble;
    static Thread _ble_thread;
    static events::EventQueue ble_queue;
    bool _force_update;

    DigitalIn _imu_cs;
    LSM6DSL_Interrupt_Pin_t _wakeup_int_pin = LSM6DSL_INT1_PIN;
    bool *_imu_interrupt;

    struct FaceBitData
    {
        uint64_t timestamp;
        FACEBIT_DATA_TYPES_t data_type;
        uint16_t value;
    };

    vector<FaceBitData> data_buffer;

    MASK_STATE_t _mask_state = MASK_STATE_LAST;
    MASK_STATE_t _next_mask_state = OFF_FACE;
    bool _new_mask_state = true;

    TASK_STATE_t _task_state = IDLE;
    TASK_STATE_t _next_task_state = IDLE;
    bool _new_task_state = false;

    milliseconds _sleep_duration = 1000ms;

    milliseconds OFF_SLEEP_DURATION = 5000ms;
    milliseconds ON_FACE_SLEEP_DURATION = 5000ms;

    const uint32_t RR_PERIOD = 0 * 10 * 1000; // 1 min
    const uint32_t HR_PERIOD = 0 * 40 * 1000; // 1 min
    const uint32_t BLE_BROADCAST_PERIOD = 2 * 60 * 1000; // 2 min

    const uint32_t BLE_CONNECTION_TIMEOUT = 5000;
    const uint32_t BLE_DRDY_TIMEOUT = 5000;

    uint64_t _mask_state_change_ts = 0; 

    uint32_t _last_rr_ts = 0;
    uint32_t _last_hr_ts = 0;
    uint32_t _last_mf_ts = 0;
    uint32_t _last_ble_ts = 0;

    const uint8_t RESP_RATE_FAILURE = 1;
    const uint8_t HR_FAILURE = 1;

    bool _ble_initialized = false;

    const uint8_t BUFFER_SIZE_ADDR = 10;
    const uint8_t CURRENT_TIME_ADDR = 12;
    const uint8_t DATA_BUFFER_ADDR = 20;
    const uint8_t INITIALIZE_ADDR = 1;
    const char INITIALIZE_STR[4] = {0xAB, 0xAF, 0xFA, 0xAA};

    bool _get_imu_int();
    bool _sync_data();
    // bool _store_data_buffer();
    // uint64_t _retrieve_time();
    // bool _store_time();
    // bool _initialize_fram();
};


#endif // FACEBITSTATE_H_
