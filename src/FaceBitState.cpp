#include "FaceBitState.hpp"
#include "Barometer.hpp"
#include "Si7051.h"
#include "SPI.h"
#include "I2C.h"
#include "BCG.h"
#include "PinNames.h"
#include "CapCalc.h"

    // SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);
    // I2C i2c(I2C_SDA0, I2C_SCL0);

    // Si7051 temp(&i2c);
    // Barometer barometer(&spi, BAR_CS, BAR_DRDY);


FaceBitState::FaceBitState() :
_spi(SPI_MOSI, SPI_MISO, SPI_SCK),
_i2c(I2C_SDA0, I2C_SCL0),
_imu_cs(IMU_CS),
_imu_int1(IMU_INT1)
{
    _bus_control = BusControl::get_instance();
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
        ThisThread::sleep_for(5s);
    }
}

void FaceBitState::update_state()
{
    if (_mask_state != _next_mask_state)
    {
        _new_mask_state = true;
        _mask_state = _next_mask_state;
        LOG_INFO("MASK STATE: %i", _mask_state);
    }
    else
    {
        _new_mask_state = false;
    }
    

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

                imu.enable_wake_up_detection(_wakeup_int_pin);

                _bus_control->set_power_lock(BusControl::IMU, true);
                _bus_control->spi_power(false);
            }

            InterruptIn motion_int(IMU_INT1);
            if (motion_int)
            {
                LOG_INFO("%s", "MOTION DETECTED");
                _next_mask_state = OFF_FACE_ACTIVE;
            }
            else
            {
                sleep_duration = 5000ms;
            }

            break;
        }
        case OFF_FACE_ACTIVE:

            break;
        
        case ON_FACE:

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
                _mask_state = OFF_FACE_INACTIVE; 
                break;
    }
}