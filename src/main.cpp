/* mbed Microcontroller Library
 * Copyright (c) 2017-2019 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "SWO.h"
#include "logger.h"
// #include "FRAM.h"
#include "SPI.h"
#include "I2C.h"
#include "PinNames.h"
#include "LPS22HBSensor.h"
#include "LSM6DSLSensor.h"

#define BLINKING_RATE_MS 1000
#define SPI_TYPE_LPS22HB LPS22HBSensor::SPI4W

DigitalOut led(LED1);

DigitalOut mag_cs(MAG_CS);
DigitalIn mag_drdy(MAG_DRDY);
DigitalOut mag_vcc(MAG_VCC);

DigitalOut fram_cs(MEM_CS);
DigitalOut fram_vcc(MEM_VCC);

DigitalOut bar_vcc(BAR_VCC);
DigitalOut bar_cs(BAR_CS);

DigitalOut mic_vcc(MIC_VCC);
DigitalOut i2s_sd(SD);
DigitalOut i2s_ws(WS);
DigitalOut i2s_sck(I2S_SCK);

DigitalOut temp_vcc(TEMP_VCC);

DigitalOut acc_vcc(ACC_VCC_EN);

DigitalOut voc_vcc(VOC_VCC);

AnalogIn vcap(VCAP);
DigitalOut vcap_en(VCAP_ENABLE);
DigitalIn n_backup(N_BACKUP);

DigitalOut imu_vcc(IMU_VCC);
DigitalIn imu_int1(IMU_INT1);

DigitalOut i2c_pu(I2C_PULLUP);

I2C i2c(I2C_SDA0, I2C_SCL0);
SWO_Channel SWO;

bool fifoFull = false;

void fifo_full()
{
    fifoFull = true;
}

int main()
{
    NRF_GPIO->PIN_CNF[IMU_VCC] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    // NRF_GPIO->PIN_CNF[SPI_MOSI] |= (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    // NRF_GPIO->PIN_CNF[SPI_MISO] |= (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    // NRF_GPIO->PIN_CNF[SPI_SCK] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    // NRF_GPIO->PIN_CNF[N_IMU_CS] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    

    fram_vcc = 1;
    bar_vcc = 1;
    mag_vcc = 1;
    imu_vcc = 1;

    fram_cs = 1;
    mag_cs = 1;
    bar_cs = 1;

    SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);
    spi.format(8, 0);
    spi.frequency(1000000);

    ThisThread::sleep_for(100ms);

    LSM6DSLSensor imu(&spi, IMU_CS);
    imu.init(NULL);
    ThisThread::sleep_for(100ms);
    imu.enable_x();

    ThisThread::sleep_for(100ms);

    while (true)
    {
        led = !led;

        uint8_t id;
        imu.read_id(&id);
        LOG_DEBUG("Read ID = 0x%X", id);

        int32_t data[3] = { 0 };
        imu.get_x_axes(data);
        LOG_DEBUG("imu data: x = %li, y = %li, z = %li", data[0], data[1], data[2]);

        ThisThread::sleep_for(500ms);
    }
}
