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
LowPowerTicker bcg_ticker;
Thread *thread1,thread2;
BCG bcg(&spi, IMU_INT1, IMU_CS);
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
void bcg_callback(){
    bcg.collect_data(512 + 128);
    bcg.calc_hr();
}

void t1()
{
    event_queue.call(&bcg_callback);
}
    
int main()
{
    swo.claim();


    //printf("starting\r\n");

    thread1 = new Thread(osPriorityNormal, 512);
    thread1->start(led_thread);

    bcg_ticker.attach(&t1,10000ms);
   
    thread2.start(callback(&event_queue, &EventQueue::dispatch_forever));
    
    while(1)
    {

    }

    return 0;
}

