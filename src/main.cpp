/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
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

#include "gatt_server_process.h"

#include "PinNames.h"
#include "mbed.h"
#include "LowPowerTicker.h"
#include "SPI.h"
#include "SWO.h"
#include "SWOLogger.h"

#include "BusControl.h"
#include "BCG.h"

DigitalOut led(LED1);

SWO_Channel SWO; // for SWO logging
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);

SWO_Channel swo("channel");

using namespace std::literals::chrono_literals;
// LowPowerTicker bcg_ticker;
Thread t1;
void led_thread()
{
    while(1)
    {
        led = 0;
        ThisThread::sleep_for(2000ms);
        led = 1;
        ThisThread::sleep_for(100ms);
    }
}
static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);
// void bcg_callback(){
//     bcg.collect_data(256);
//     bcg.calc_hr();
// }

// void t1()
// {
//     event_queue.call(&bcg, &BCG::calc_hr);
// }
    
int main()
{
    swo.claim();
    
    t1.start(led_thread);
    
    LOG_INFO("%s", "starting collection in 5 seconds...");
    ThisThread::sleep_for(5s);
    
    BCG bcg(&spi, IMU_INT1, IMU_CS);

    uint16_t num_samples = 10 * bcg.get_frequency(); // 10 seconds of data

    while(1)
    {
        bcg.bcg(num_samples);
        printf("\r\n\n\n\n\n");
        ThisThread::sleep_for(5s);
    }

    return 0;
}

