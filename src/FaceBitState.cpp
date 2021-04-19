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
#include "LowPowerTimer.h"
#include "TARGET_SMARTPPE/PinNames.h"

events::EventQueue FaceBitState::ble_queue(16 * EVENTS_EVENT_SIZE);
Thread FaceBitState::_ble_thread(osPriorityNormal, 4096);

FaceBitState::FaceBitState(SmartPPEService *smart_ppe_ble, bool *imu_interrupt) :
_spi(SPI_MOSI, SPI_MISO, SPI_SCK),
_i2c(I2C_SDA0, I2C_SCL0),
_imu_cs(IMU_CS),
_smart_ppe_ble(smart_ppe_ble),
_imu_interrupt(imu_interrupt),
_fram(&_spi, FRAM_CS)
{
    _logger = Logger::get_instance();
    _bus_control = BusControl::get_instance();
}

FaceBitState::~FaceBitState()
{
}

void FaceBitState::run()
{
    _spi.frequency(8000000); // fast, to reduce transaction time

    _state_timer.start();

    while(1)
    {
        update_state();
        _bus_control->set_led_blinks((uint8_t)_mask_state + 1);
        
        if (_state_timer.read_ms() > BLE_BROADCAST_PERIOD)
        {
            _sync_data();
            _last_ble_ts = _state_timer.read_ms();
        }

        _logger->log(TRACE_TRACE, "sleeping for %lli", static_cast<long long int>(_sleep_duration.count()));
        ThisThread::sleep_for(_sleep_duration);
    }
}

void FaceBitState::update_state()
{
    switch(_mask_state)
    {
        case OFF_FACE:
        {
            if (_new_mask_state)
            {
                _sleep_duration = OFF_SLEEP_DURATION;
            }

            Barometer barometer(&_spi, (PinName)BAR_CS, (PinName)BAR_DRDY);
            MaskStateDetection mask_state(&barometer);

            MaskStateDetection::MASK_STATE_t mask_status;
            mask_status = mask_state.is_on(); // blocking call for ~5s

            if (mask_status == MaskStateDetection::ON)
            {
                _logger->log(TRACE_INFO, "%s", "MASK ON");
            _next_mask_state = ON_FACE;
            }
            else if (mask_status == MaskStateDetection::OFF)
            {
                _logger->log(TRACE_INFO, "%s", "MASK OFF");
            }
            else if (mask_status == MaskStateDetection::ERROR)
            {
                _logger->log(TRACE_WARNING, "%s", "MASK DETECTION ERROR");
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
                    uint32_t rr_time_over = _state_timer.read_ms() - _last_rr_ts;
                    uint32_t hr_time_over = _state_timer.read_ms() - _last_hr_ts;

                    if (_state_timer.read_ms() - _last_rr_ts >= RR_PERIOD && (rr_time_over >= hr_time_over))
                    {
                        _next_task_state = MEASURE_RESPIRATION_RATE;
                    }
                    else if (_state_timer.read_ms() - _last_hr_ts >= HR_PERIOD && (hr_time_over >= rr_time_over))
                    {
                        _next_task_state = MEASURE_HEART_RATE;
                    }

                    break;
                }

                case MEASURE_RESPIRATION_RATE:
                {
                    Si7051 temp(&_i2c);
                    Barometer barometer(&_spi, BAR_CS, BAR_DRDY);
                    RespiratoryRate resp_rate(temp, barometer);

                    _logger->log(TRACE_INFO, "RESP RATE MEASUREMENT");

                    _last_rr_ts = _state_timer.read_ms();

                    float rate = resp_rate.respiratory_rate(30, RespiratoryRate::THERMOMETER);

                    if(rate != -1)
                    {
                        _logger->log(TRACE_INFO, "Respiratory rate success");

                        FaceBitData rr_data;
                        rr_data.data_type = RESPIRATORY_RATE;
                        rr_data.timestamp = _state_timer.read_ms();
                        rr_data.value = rate;

                        data_buffer.push_back(rr_data);
                    }
                    else
                    {
                        _logger->log(TRACE_INFO, "Respiratory rate failure");
                        FaceBitData rr_failure;
                        rr_failure.data_type = RESPIRATORY_RATE;
                        rr_failure.timestamp = _state_timer.read_ms();
                        rr_failure.value = RESP_RATE_FAILURE;

                        data_buffer.push_back(rr_failure);
                    }

                    _next_task_state = IDLE;

                    break;
                }

                case MEASURE_HEART_RATE:
                {
                    _logger->log(TRACE_INFO, "%s", "MEASURING HR");
                    _last_hr_ts = _state_timer.read_ms();
                    BCG bcg(&_spi, (PinName)IMU_INT1, (PinName)IMU_CS);

                    if(bcg.bcg(30s)) // blocking
                    {
                        _logger->log(TRACE_INFO, "%s", "HR CAPTURED!");
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
                    else
                    {
                        FaceBitData hr_data;

                        hr_data.data_type = HEART_RATE;
                        hr_data.timestamp = time(NULL);
                        hr_data.value = HR_FAILURE;

                        data_buffer.push_back(hr_data);                       
                    }
                    _next_task_state = IDLE;
                    break;
                }

                default:
                    _next_task_state = IDLE;
                    break;
            }

            break;
        }

            default:
                _next_mask_state = OFF_FACE; 
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
                _logger->log(TRACE_INFO, "%s", "MASK ON");
            }
            else if (mask_status == MaskStateDetection::OFF)
            {
                _logger->log(TRACE_INFO, "%s", "MASK OFF");
                _next_task_state = IDLE;
                _next_mask_state = OFF_FACE;
            }
            else if (mask_status == MaskStateDetection::ERROR)
            {
                _logger->log(TRACE_WARNING, "%s", "MASK DETECTION ERROR");
                _next_task_state = IDLE; // don't run if we don't know
            }
        }

        _new_task_state = true;
        _task_state = _next_task_state;
        _logger->log(TRACE_INFO, "TASK STATE: %i", _task_state);

        _sleep_duration = 1ms;
    }

    if (_mask_state != _next_mask_state)
    {
        _force_update = true;

        _mask_state_change_ts = time(NULL);

        _new_mask_state = true;
        _mask_state = _next_mask_state;
        _logger->log(TRACE_INFO, "MASK STATE: %i", _mask_state);

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

bool FaceBitState::_sync_data()
{
    BLE &_ble = BLE::Instance();
    GattServerProcess _ble_process(ble_queue, _ble);
    _ble_process.on_init(callback(_smart_ppe_ble, &SmartPPEService::start));

    _ble_thread.start(callback(&_ble_process, &GattServerProcess::run));

    if (data_buffer.size() == 0 && _force_update == false)
    {
        _logger->log(TRACE_DEBUG, "%s", "NO DATA TO SEND");
        return false;
    }

    _logger->log(TRACE_DEBUG, "%s", "BLE SYNC");

    // start ble
    _ble_thread.flags_set(START_BLE);

    // wait for connection
    LowPowerTimer ble_timeout;
    ble_timeout.start();
    while(!_ble_process.is_connected())
    {
        Thread::State state = _ble_thread.get_state();
        uint16_t size = _ble_process.event_queue_size;
        // _logger->log(TRACE_TRACE, "Thread state = %u, equeue size = %u", state, size);
        
        if (ble_timeout.read_ms() > BLE_CONNECTION_TIMEOUT)
        {
            _logger->log(TRACE_INFO, "%s", "TIMEOUT BEFORE BLE CONNECTION");
            _ble_thread.flags_set(STOP_BLE);
            system_reset();
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
            _logger->log(TRACE_INFO, "%s", "BLE DATA READY TIMEOUT (MASK ON)");
            _ble_thread.flags_set(STOP_BLE);
            system_reset();
        }

        ThisThread::sleep_for(500ms);
    }

    // sync timestamp
    uint64_t new_time = _smart_ppe_ble->getTime();
    if (new_time != 0)
    {
        set_time(_smart_ppe_ble->getTime());
        _logger->log(TRACE_INFO, "Time set to %lli", time(NULL));
    }

    if (data_buffer.size() == 0)
    {
        _logger->log(TRACE_DEBUG, "%s", "NO PHYSIO DATA TO SEND");
    }

    for (int i = 0; i < data_buffer.size(); i++)
    {
        _logger->log(TRACE_DEBUG, "DATA BUFFER HAS %u ELEMENTS", data_buffer.size());

        FaceBitData next_data_point = data_buffer.at(i);

        switch(next_data_point.data_type)
        {
            case HEART_RATE:
                _logger->log(TRACE_DEBUG, "WRITING HR = %u", next_data_point.value);
                _smart_ppe_ble->updateHeartRate(next_data_point.timestamp, next_data_point.value);
                _smart_ppe_ble->updateDataReady(_smart_ppe_ble->HEART_RATE);
                
                ble_timeout.reset();
                ble_timeout.start();
                while(_smart_ppe_ble->getDataReady() != _smart_ppe_ble->NO_DATA)
                {
                    _smart_ppe_ble->updateDataReady(_smart_ppe_ble->HEART_RATE);
                    if (ble_timeout.read_ms() > BLE_DRDY_TIMEOUT)
                    {
                        _logger->log(TRACE_INFO, "%s", "BLE DATA READY TIMEOUT (DATA)");
                        _ble_thread.flags_set(STOP_BLE);
                        return false;
                    }
                    ThisThread::sleep_for(100ms);
                }
                break;
            
            case RESPIRATORY_RATE:
                _logger->log(TRACE_DEBUG, "WRITING RESP RATE = %u", next_data_point.value);
                _smart_ppe_ble->updateRespiratoryRate(next_data_point.timestamp, next_data_point.value);
                _smart_ppe_ble->updateDataReady(_smart_ppe_ble->RESPIRATORY_RATE);
                ble_timeout.reset();
                ble_timeout.start();
                while(_smart_ppe_ble->getDataReady() != _smart_ppe_ble->NO_DATA)
                {
                    _smart_ppe_ble->updateDataReady(_smart_ppe_ble->RESPIRATORY_RATE);
                    if (ble_timeout.read_ms() > BLE_DRDY_TIMEOUT)
                    {
                        _logger->log(TRACE_INFO, "%s", "BLE DATA READY TIMEOUT (DATA)");
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

    system_reset();

    return true;
}
