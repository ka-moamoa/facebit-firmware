#include "FaceBitState.hpp"
#include "Barometer.hpp"
#include "Si7051.h"
#include "SPI.h"
#include "I2C.h"
#include "BCG.h"
#include "PinNames.h"
#include "CapCalc.h"
#include "MaskStateDetection.hpp"

FaceBitState::FaceBitState() :
_spi(SPI_MOSI, SPI_MISO, SPI_SCK),
_i2c(I2C_SDA0, I2C_SCL0),
_imu_cs(IMU_CS),
_imu_int1(IMU_INT1)
{
    _bus_control = BusControl::get_instance();
    _imu_int1.rise(callback(this, &FaceBitState::imu_int_handler));
}

FaceBitState::~FaceBitState()
{
}

void FaceBitState::run()
{
    _spi.frequency(8000000); // fast, to reduce transaction time

    while(1)
    {
        update_state();
        LOG_TRACE("sleeping for %lli", static_cast<long long int>(_sleep_duration.count()));
        ThisThread::sleep_for(_sleep_duration);
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

                _bus_control->set_power_lock(BusControl::IMU, true);
                _bus_control->spi_power(false);
                
                _sleep_duration = INACTIVE_SLEEP_DURATION;
            }

            if (get_imu_int())
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
            }

            break;
        }
        
        case ON_FACE:

        _sleep_duration = ON_FACE_SLEEP_DURATION;

            switch(_task_state)
            {
                case IDLE:

                    break;

                case RESPIRATION_RATE:

                    break;

                case HEART_RATE:

                    break;

                case MASK_FIT:

                    break;

                case BLE_BROADCAST:

                    break;

                default:
                    _task_state = IDLE;
                    break;
            }

            break;

            default:
                _next_mask_state = OFF_FACE_INACTIVE; 
                break;
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

void FaceBitState::imu_int_handler()
{
    _imu_interrupt = true;
}

bool FaceBitState::get_imu_int()
{
    bool tmp = _imu_interrupt;
    _imu_interrupt = false;
    return tmp;
}