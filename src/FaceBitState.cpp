#include "FaceBitState.hpp"
#include "Barometer.hpp"
#include "Si7051.h"
#include "SPI.h"
#include "I2C.h"
#include "BCG.h"
#include "PinNames.h"
#include "CapCalc.h"
#include "MaskStateDetection.hpp"

#include "gatt_server_process.h"

events::EventQueue FaceBitState::ble_queue(16 * EVENTS_EVENT_SIZE);
Thread FaceBitState::_ble_thread(osPriorityNormal, 4096);
BLE& FaceBitState::ble(BLE::Instance());

FaceBitState::FaceBitState() :
_spi(SPI_MOSI, SPI_MISO, SPI_SCK),
_i2c(I2C_SDA0, I2C_SCL0),
_imu_cs(IMU_CS),
_imu_int1(IMU_INT1)
{
    _bus_control = BusControl::get_instance();
    _imu_int1.rise(callback(this, &FaceBitState::_imu_int_handler));
}

FaceBitState::~FaceBitState()
{
}

void FaceBitState::run()
{
    _spi.frequency(8000000); // fast, to reduce transaction time
    LowPowerTimer _ble_timer;

    _ble_timer.start();

    _sync_data();

    while(1)
    {
        update_state();
        LOG_TRACE("sleeping for %lli", static_cast<long long int>(_sleep_duration.count()));
        ThisThread::sleep_for(_sleep_duration);
        
        if (_ble_timer.read_ms() - _last_ble_ts > BLE_BROADCAST_PERIOD && data_buffer.size() > 0)
        {
            _sync_data();
            _last_ble_ts = _ble_timer.read_ms();
        }
    }
}

void FaceBitState::update_state()
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
                imu.set_wake_up_threshold(LSM6DSL_WAKE_UP_THRESHOLD_MID);

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

            if(_mask_state_timer.read_ms() > ACTIVE_STATE_TIMEOUT)
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
                    uint32_t ts = _mask_state_timer.read_ms();

                    // if (ts - _last_rr_ts >= RR_PERIOD)
                    // {
                    //     _next_task_state = MEASURE_RESPIRATION_RATE;
                    // }
                    if (ts - _last_hr_ts >= HR_PERIOD)
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
                    
                    break;
                }

                case MEASURE_HEART_RATE:
                {
                    LOG_INFO("%s", "MEASURING HR")
                    _last_hr_ts = _mask_state_timer.read_ms();
                    BCG bcg(&_spi, (PinName)IMU_INT1, (PinName)IMU_CS);
                    if(bcg.bcg(10s)) // blocking
                    {
                        for(int i = 0; i < bcg.get_buffer_size(); i++)
                        {
                            FaceBitData hr_data;
                            BCG::HR_t hr = bcg.get_buffer_element();

                            hr_data.data_type = HEART_RATE;
                            hr_data.timestamp = hr.timestamp;
                            hr_data.value = hr.rate;

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
        _new_mask_state = true;
        _mask_state = _next_mask_state;
        LOG_INFO("MASK STATE: %i", _mask_state);

        _mask_state_timer.reset();
        _mask_state_timer.start();

        _sleep_duration = 1ms;
    }
    else
    {
        _new_mask_state = false;
    }
}

void FaceBitState::_imu_int_handler()
{
    _imu_interrupt = true;
}

bool FaceBitState::_get_imu_int()
{
    bool tmp = _imu_interrupt;
    _imu_interrupt = false;
    return tmp;
}


bool FaceBitState::_sync_data()
{    
    LOG_TRACE("%s", "BLE BROADCAST");

    GattServerProcess ble_process(ble_queue, ble);

    ble_process.on_init(callback(&_smart_ppe_ble, &SmartPPEService::start));

    _ble_thread.start(callback(&ble_process, &GattServerProcess::start));

    LowPowerTimer ble_timeout;
    ble_timeout.start();
    while(!ble_process.is_connected())
    {
        if (ble_timeout.read_ms() > BLE_CONNECTION_TIMEOUT)
        {
            LOG_INFO("%s", "NO BLE CONNECTION");
            ble_process.stop(); 
            _ble_thread.terminate();
            return false;
        }

        ThisThread::sleep_for(1s);
    }

    for (int i = 0; i < data_buffer.size(); i++)
    {
        FaceBitData next_data_point = data_buffer.at(i);

        switch(next_data_point.data_type)
        {
            case HEART_RATE:
                _smart_ppe_ble.updateRespiratoryRate(next_data_point.timestamp, next_data_point.value);
                _smart_ppe_ble.updateDataReady(_smart_ppe_ble.HEART_RATE);
                break;
            
            case RESPIRATORY_RATE:
                _smart_ppe_ble.updateHeartRate(next_data_point.timestamp, next_data_point.value);
                _smart_ppe_ble.updateDataReady(_smart_ppe_ble.RESPIRATORY_RATE);
                break;

            case MASK_FIT:
                // _smart_ppe_ble.updateRespiratoryRate(next_data_point.timestamp, next_data_point.value);
                break;
        }
    }

    ble_process.stop(); 
    _ble_thread.terminate();

    return true;
}