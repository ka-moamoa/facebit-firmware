#include "FaceBitState.hpp"
#include "Barometer.hpp"
#include "Si7051.h"
#include "SPI.h"
#include "I2C.h"
#include "BCG.h"
#include "PinNames.h"
#include "CapCalc.h"
#include "MaskStateDetection.hpp"
#include "RespiratoryRate.hpp"
#include "ble_process.h"


events::EventQueue FaceBitState::ble_queue(16 * EVENTS_EVENT_SIZE);
Thread FaceBitState::_ble_thread(osPriorityNormal, 2048);


FaceBitState::FaceBitState(SmartPPEService *smart_ppe_ble, bool *imu_interrupt) :
_spi(SPI_MOSI, SPI_MISO, SPI_SCK),
_i2c(I2C_SDA0, I2C_SCL0),
_imu_cs(IMU_CS),
_smart_ppe_ble(smart_ppe_ble),
_imu_interrupt(imu_interrupt)
{
    _bus_control = BusControl::get_instance();
}

FaceBitState::~FaceBitState()
{
}

void FaceBitState::run()
{
    _spi.frequency(8000000); // fast, to reduce transaction time
    LowPowerTimer state_timer;

    state_timer.start();

    BLE &_ble = BLE::Instance();
    GattServerProcess _ble_process(ble_queue, _ble);
    _ble_process.on_init(callback(_smart_ppe_ble, &SmartPPEService::start));
    
    _ble_thread.start(callback(&_ble_process, &GattServerProcess::run));
    
    _force_update = true;
    _sync_data(&_ble_process);

    while(1)
    {
        uint32_t ts = state_timer.read_ms();

        update_state(ts);
        
        if (ts - _last_ble_ts > BLE_BROADCAST_PERIOD)
        {
            _sync_data(&_ble_process);
            _last_ble_ts = ts;
        }

        LOG_TRACE("sleeping for %lli", static_cast<long long int>(_sleep_duration.count()));
        ThisThread::sleep_for(_sleep_duration);
    }
}

void FaceBitState::update_state(uint32_t ts)
{
    switch(_mask_state)
    {
        case OFF_FACE_INACTIVE:
        {
            if (_new_mask_state)
            {
                _bus_control->spi_power(true);
                ThisThread::sleep_for(10ms);
            
                LSM6DSLSensor imu(&_spi, (PinName)IMU_CS);
                imu.init(NULL);

                imu.enable_x();
                imu.enable_wake_up_detection(_wakeup_int_pin);
                imu.set_wake_up_threshold(LSM6DSL_WAKE_UP_THRESHOLD_LOW);

                _bus_control->set_power_lock(BusControl::IMU, true);
                _bus_control->spi_power(false);
                
                _sleep_duration = INACTIVE_SLEEP_DURATION;
            }

            if (_get_imu_int())
            {
                LOG_INFO("%s", "MOTION DETECTED");
                _next_mask_state = OFF_FACE_ACTIVE;

                _bus_control->spi_power(true);
                ThisThread::sleep_for(10ms);

                LSM6DSLSensor imu(&_spi, (PinName)IMU_CS);
                imu.init(NULL);

                imu.disable_x();
                imu.disable_tilt_detection();

                _bus_control->set_power_lock(BusControl::IMU, false);
                _bus_control->spi_power(false);
            }
            break;
        }
        case OFF_FACE_ACTIVE:
        {
            if (_new_mask_state)
            {
                ACTIVE_STATE_ENTRY_TS = ts;
                _sleep_duration = ACTIVE_SLEEP_DURATION;
            }

            Barometer barometer(&_spi, (PinName)BAR_CS, (PinName)BAR_DRDY);
            MaskStateDetection mask_state(&barometer);

            MaskStateDetection::MASK_STATE_t mask_status;
            mask_status = mask_state.is_on(); // blocking call for ~5s

            if (mask_status == MaskStateDetection::ON)
            {
                LOG_INFO("%s", "MASK ON");
                _next_mask_state = ON_FACE;
            }
            else if (mask_status == MaskStateDetection::OFF)
            {
                LOG_INFO("%s", "MASK OFF");
            }
            else if (mask_status == MaskStateDetection::ERROR)
            {
                LOG_WARNING("%s", "MASK DETECTION ERROR");
            }

            if(ts - ACTIVE_STATE_ENTRY_TS > ACTIVE_STATE_TIMEOUT)
            {
                _next_mask_state = OFF_FACE_INACTIVE;
                _next_task_state = IDLE;
            }

            break;
        }
        
        case ON_FACE:
        {
        _sleep_duration = ON_FACE_SLEEP_DURATION;

            switch(_task_state)
            {
                case IDLE:
                {
                    if (ts - _last_rr_ts >= RR_PERIOD)
                    {
                        _next_task_state = MEASURE_RESPIRATION_RATE;
                    }
                    else if (ts - _last_hr_ts >= HR_PERIOD)
                    {
                        _next_task_state = MEASURE_HEART_RATE;
                    }
                    // else if(ts - _last_mf_ts > MASK_FIT_PERIOD)
                    // {
                    //     _next_task_state = MEASURE_MASK_FIT;
                    // }
                    // else if (ts - _last_ble_ts > BLE_BROADCAST_PERIOD)
                    // {
                    //     _next_task_state = BLE_BROADCAST;
                    // }

                    break;
                }

                case MEASURE_RESPIRATION_RATE:
                {
                    CapCalc* cap = CapCalc::get_instance();
                    Si7051 temp(&_i2c);
                    RespiratoryRate resp_rate(cap, temp);

                    _last_rr_ts = ts;

                    resp_rate.get_resp_rate();

                    RespiratoryRate::RR_t rr;

                    if (resp_rate.get_buffer_size())
                    {
                        rr = resp_rate.get_buffer_element();
                    }

                    FaceBitData rr_data;
                    rr_data.data_type = RESPIRATORY_RATE;
                    rr_data.timestamp = rr.timestamp;
                    rr_data.value = rr.rate;

                    data_buffer.push_back(rr_data);

                    _next_task_state = IDLE;

                    break;
                }

                case MEASURE_HEART_RATE:
                {
                    LOG_INFO("%s", "MEASURING HR")
                    _last_hr_ts = ts;
                    BCG bcg(&_spi, (PinName)IMU_INT1, (PinName)IMU_CS);

                    if(bcg.bcg(15s)) // blocking
                    {
                        for(int i = 0; i < bcg.get_buffer_size(); i++)
                        {
                            FaceBitData hr_data;
                            BCG::HR_t hr = bcg.get_buffer_element();

                            hr_data.data_type = HEART_RATE;
                            hr_data.timestamp = hr.timestamp;
                            hr_data.value = hr.rate;

                            // hr_data.data_type = HEART_RATE;
                            // hr_data.timestamp = time(NULL);
                            // hr_data.value = 3;

                            data_buffer.push_back(hr_data);
                        }
                    }
                    _next_task_state = IDLE;
                    break;
                }

                case MEASURE_MASK_FIT:
                {
                    
                    break;
                }

                default:
                    _next_task_state = IDLE;
                    break;
            }

            break;
        }

            default:
                _next_mask_state = OFF_FACE_INACTIVE; 
                break;
    }

    if (_mask_state == ON_FACE && _task_state != _next_task_state)
    {
        if (_next_task_state != IDLE)
        {
            /**
             * We want to run mask on/off detection before every
             * new task, so we don't waste energy on the task (and get
             * an inaccurate result) if the mask is off.
             */
            Barometer barometer(&_spi, (PinName)BAR_CS, (PinName)BAR_DRDY);
            MaskStateDetection mask_state(&barometer);

            MaskStateDetection::MASK_STATE_t mask_status;
            mask_status = mask_state.is_on(); // blocking call for ~5s

            if (mask_status == MaskStateDetection::ON)
            {
                LOG_INFO("%s", "MASK ON");
            }
            else if (mask_status == MaskStateDetection::OFF)
            {
                LOG_INFO("%s", "MASK OFF");
                _next_task_state = IDLE;
                _next_mask_state = OFF_FACE_ACTIVE;
            }
            else if (mask_status == MaskStateDetection::ERROR)
            {
                LOG_WARNING("%s", "MASK DETECTION ERROR");
                _next_task_state = IDLE; // don't run if we don't know
            }
        }

        _new_task_state = true;
        _task_state = _next_task_state;
        LOG_INFO("TASK STATE: %i", _task_state);

        _sleep_duration = 1ms;
    }

    if (_mask_state != _next_mask_state)
    {
        _force_update = true;

        _mask_state_change_ts = time(NULL);

        _new_mask_state = true;
        _mask_state = _next_mask_state;
        LOG_INFO("MASK STATE: %i", _mask_state);

        _sleep_duration = 10ms;
    }
    else
    {
        _new_mask_state = false;
    }
}

bool FaceBitState::_get_imu_int()
{
    bool tmp = *_imu_interrupt;
    *_imu_interrupt = false;
    return tmp;
}

bool FaceBitState::_sync_data(GattServerProcess *_ble_process)
{    
    if (data_buffer.size() == 0 && _force_update == false)
    {
        LOG_DEBUG("%s", "NO DATA TO SEND")
        return false;
    }

    LOG_DEBUG("%s", "BLE SYNC");

    // start ble
    _ble_thread.flags_set(START_BLE);

    // wait for connection
    LowPowerTimer ble_timeout;
    ble_timeout.start();
    while(!_ble_process->is_connected())
    {
        Thread::State state = _ble_thread.get_state();
        uint16_t size = _ble_process->event_queue_size;
        LOG_TRACE("Thread state = %u, equeue size = %u", state, size);
        
        if (ble_timeout.read_ms() > BLE_CONNECTION_TIMEOUT)
        {
            LOG_INFO("%s", "TIMEOUT BEFORE BLE CONNECTION");
            _ble_thread.flags_set(STOP_BLE);
            return false;
        }

        ThisThread::sleep_for(10ms);
    }

    // set mask on characteristic based on state
    _smart_ppe_ble->updateMaskOn(_mask_state_change_ts, _mask_state);
    _smart_ppe_ble->updateDataReady(SmartPPEService::MASK_ON);

    ble_timeout.reset();
    ble_timeout.start();
    while(1)
    { 
        SmartPPEService::data_ready_t data_ready =  _smart_ppe_ble->getDataReady();
        if (data_ready == SmartPPEService::NO_DATA)
        {
            break;
        }
        
        _smart_ppe_ble->updateDataReady(SmartPPEService::MASK_ON);

        if (ble_timeout.read_ms() > BLE_DRDY_TIMEOUT)
        {
            LOG_INFO("%s", "BLE DATA READY TIMEOUT (MASK ON)");
            _ble_thread.flags_set(STOP_BLE);
            return false;
        }

        ThisThread::sleep_for(100ms);
    }

    // sync timestamp
    uint64_t new_time = _smart_ppe_ble->getTime();
    if (new_time != 0)
    {
        set_time(_smart_ppe_ble->getTime());
        LOG_INFO("Time set to %lli", time(NULL));
    }

    if (data_buffer.size() == 0)
    {
        LOG_DEBUG("%s", "NO PHYSIO DATA TO SEND");
    }

    for (int i = 0; i < data_buffer.size(); i++)
    {
        LOG_DEBUG("DATA BUFFER HAS %u ELEMENTS", data_buffer.size());

        FaceBitData next_data_point = data_buffer.at(i);

        switch(next_data_point.data_type)
        {
            case HEART_RATE:
                LOG_DEBUG("WRITING HR = %u", next_data_point.value);
                _smart_ppe_ble->updateHeartRate(next_data_point.timestamp, next_data_point.value);
                _smart_ppe_ble->updateDataReady(_smart_ppe_ble->HEART_RATE);
                break;
            
            case RESPIRATORY_RATE:
                LOG_DEBUG("WRITING RESP RATE = %u", next_data_point.value);
                _smart_ppe_ble->updateRespiratoryRate(next_data_point.timestamp, next_data_point.value);
                _smart_ppe_ble->updateDataReady(_smart_ppe_ble->RESPIRATORY_RATE);
                ble_timeout.reset();
                ble_timeout.start();
                while(_smart_ppe_ble->getDataReady() != _smart_ppe_ble->NO_DATA)
                {
                    _smart_ppe_ble->updateDataReady(_smart_ppe_ble->RESPIRATORY_RATE);
                    if (ble_timeout.read_ms() > BLE_DRDY_TIMEOUT)
                    {
                        LOG_INFO("%s", "BLE DATA READY TIMEOUT (DATA)");
                        _ble_thread.flags_set(STOP_BLE);
                        return false;
                    }
                    ThisThread::sleep_for(100ms);
                }
                break;

            case MASK_FIT:
                // _smart_ppe_ble.updateRespiratoryRate(next_data_point.timestamp, next_data_point.value);
                break;
        }
    }

    if (data_buffer.size())
    {
        data_buffer.clear();
    }

    _force_update = false;

    _ble_thread.flags_set(STOP_BLE);

    return true;
}