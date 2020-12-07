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
#include "Si7051.h"

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
Si7051 sensor(&i2c);
SWO_Channel SWO;

bool fifoFull = false;

void fifo_full()
{
    fifoFull = true;
}

int main()
{
    NRF_GPIO->PIN_CNF[TEMP_VCC] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    NRF_GPIO->PIN_CNF[I2C_SDA0] |= (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    NRF_GPIO->PIN_CNF[I2C_SCL0] |= (GPIO_PIN_CNF_DRIVE_H0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode
    NRF_GPIO->PIN_CNF[I2C_PULLUP] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode

    temp_vcc = 1;
    voc_vcc = 1;

    i2c_pu = 1;

    ThisThread::sleep_for(50ms);

    sensor.reset();
    ThisThread::sleep_for(20ms);

    sensor.initialize();
    ThisThread::sleep_for(20ms);

    while(1)
    {
        led = !led;
        // uint8_t fw_ver = sensor.readFirmwareVersion();
        // LOG_DEBUG("firmware version: 0x%X", fw_ver);

        float temp = sensor.readTemperature();
        LOG_DEBUG("temp = %0.2f", temp);
        ThisThread::sleep_for(1s);
    }
}
