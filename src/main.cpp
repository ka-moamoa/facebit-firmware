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


DigitalOut led(LED1);

DigitalOut mag_cs(N_MAG_CS);
DigitalIn mag_drdy(MAG_DRDY);
DigitalOut mag_vcc(MAG_VCC);

DigitalOut fram_cs(N_MEM_CS);
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
    temp_vcc = 1;
    voc_vcc = 1;

    i2c_pu = 1;

    while(1)
    {
        led = !led;
        ThisThread::sleep_for(1s);
    }
    
    return 0;
}

