#include "BusControl.h"
#include "PinNames.h"

BusControl* BusControl::_instance = nullptr;
Mutex BusControl::_mutex;

BusControl::BusControl() :
_fram_vcc(FRAM_VCC),
_mag_vcc(MAG_VCC),
_bar_vcc(BAR_VCC),
_imu_vcc(IMU_VCC),
_fram_cs(FRAM_CS),
_mag_cs(MAG_CS),
_bar_cs(BAR_CS),
_imu_cs(IMU_CS),
_temp_vcc(TEMP_VCC),
_voc_vcc(VOC_VCC),
_i2c_pu(I2C_PULLUP)
{
    NRF_GPIO->PIN_CNF[IMU_VCC] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    NRF_GPIO->PIN_CNF[TEMP_VCC] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    NRF_GPIO->PIN_CNF[I2C_PULLUP] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
}

BusControl::~BusControl() {}

BusControl* BusControl::get_instance()
{
    _mutex.lock();

    if (_instance == nullptr)
    {
        _instance = new BusControl();
    }

    _mutex.unlock();

    return _instance;
}

void BusControl::spi_power(bool power)
{
    _fram_vcc = power;
    _bar_vcc = power;
    _mag_vcc = power;
    _imu_vcc = power;

    _fram_cs = power;
    _bar_cs = power;
    _mag_cs = power;
    _imu_cs = power;
}


void BusControl::i2c_power(bool power)
{
    _temp_vcc = power;
    _voc_vcc = power;
    _i2c_pu = power;
}