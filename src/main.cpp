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
#include "FRAM.h"
#include "SPI.h"
#include "I2C.h"
#include "PinNames.h"

DigitalOut led(LED1);

DigitalOut mag_cs(N_MAG_CS);
DigitalIn mag_drdy(MAG_DRDY);
DigitalOut mag_vcc(MAG_VCC);

DigitalOut fram_vcc(MEM_VCC);

DigitalOut bar_vcc(BAR_VCC);
DigitalOut bar_cs(BAR_CS);
DigitalIn bar_drdy(BAR_DRDY);

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
DigitalOut imu_cs(N_IMU_CS);

DigitalOut i2c_pu(I2C_PULLUP);

I2C i2c(I2C_SDA0, I2C_SCL0);
SWO_Channel SWO;

int main()
{
    NRF_GPIO->PIN_CNF[IMU_VCC] |= (GPIO_PIN_CNF_DRIVE_S0H1 << GPIO_PIN_CNF_DRIVE_Pos); // set to high drive mode

    fram_vcc = 1;
    bar_vcc = 1;
    mag_vcc = 1;
    imu_vcc = 1;

    bar_cs = 1;
    mag_cs = 1;
    imu_cs = 1;

    SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);
    spi.format(8, 0);
    spi.frequency(1000000);

    FRAM fram(&spi, N_MEM_CS);

    char rx_buffer[5] = {0};
    fram.read_bytes(0x00, rx_buffer, 5);
    LOG_DEBUG("bytes0  = 0x%X, 0x%X, 0x%X, 0x%X, 0x%X", rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3], rx_buffer[4]);

    const char tx[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    fram.write_bytes(0x01, tx, 4);

    char rx_buffer1[4] = {0};
    fram.read_bytes(0x01, rx_buffer1, 4);
    LOG_DEBUG("bytes1 = 0x%X, 0x%X, 0x%X, 0x%X", rx_buffer1[0], rx_buffer1[1], rx_buffer1[2], rx_buffer1[3]);

    char rx_buffer2[5] = {0};
    fram.read_bytes(0x00, rx_buffer2, 5);
    LOG_DEBUG("bytes2 = 0x%X, 0x%X, 0x%X, 0x%X, 0x%X", rx_buffer2[0], rx_buffer2[1], rx_buffer2[2], rx_buffer2[3], rx_buffer2[4]);

    return 0;
}
