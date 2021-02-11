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

#include "SPI.h"
#include "I2C.h"
#include "SWO.h"
#include "SWOLogger.h"
#include "CapCalc.h"
#include "BusControl.h"
#include "Barometer.hpp"
#include "Si7051.h"
#include "LowPowerTicker.h"
#include "SmartPPEService.h"

DigitalOut led(LED1);
BusControl *bus_control = BusControl::get_instance();

SWO_Channel SWO; // for SWO logging
I2C i2c(I2C_SDA0, I2C_SCL0);
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);

Si7051 temp(&i2c);
Barometer barometer(&spi, BAR_CS, BAR_DRDY);

SWO_Channel swo("channel");

using namespace std::literals::chrono_literals;

const static char DEVICE_NAME[] = "SMARTPPE";

static events::EventQueue task_queue(16 * EVENTS_EVENT_SIZE);

// Frequency configuration of each task
const std::chrono::milliseconds TIME_LED_TASK = 4000ms;
const std::chrono::milliseconds TIME_RESP_TASK = 1000ms; //10mins

static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);

LowPowerTicker lpt_led, lpt_resp_rate;

float cap_voltage = 0;

CapCalc cap(VCAP, VCAP_ENABLE, 3000);
float available_energy = 0;

const float MIN_VOLTAGE = 2.3;
const int LED_ENERGY = 0.001;
const int RESP_RATE_ENERGY = 0.001;

const bool RUN_LED = false;
const bool RUN_RESP_RATE = false;

//Thread *thread1, *thread2, *thread3;
Thread thread1, thread2;

SmartPPEService smart_ppe_ble;
void check_voltage()
{
    //printf("Checking Voltage...\n\r");
    available_energy = cap.calc_joules();
    cap_voltage = cap.read_capacitor_voltage();
    printf("Voltage: %0.2f\r\n", cap_voltage);
    printf("Energy: %0.2f\r\n", available_energy);
}

void led_thread()
{
    //while (1)
    printf("LED-Start\r\n");
    check_voltage();
    if (RUN_LED || (available_energy > LED_ENERGY))
    {
        led = !led;
    }
    printf("LED-end\r\n");
    check_voltage();
}
int calc_resp_rate(float samples[], int SAMPLE_SIZE, float mean)
{
    int i = 1;
    uint8_t max_count = 0;
    float local_max = 0;
    float max_average = 0;
    float delta = 0.5;
    while (i < SAMPLE_SIZE)
    {
        if (samples[i] - mean > 0)
        {
            if (local_max < samples[i])
            {
                local_max = samples[i];
            }
        }
        else if (samples[i] - mean <= 0)
        {
            if (local_max != 0)
            {
                if (max_average != 0 && abs(local_max - max_average) < delta)
                {
                    max_count = max_count + 1;
                    max_average = (max_average + local_max) / 2;
                    local_max = 0;
                }
                else if (max_average == 0)
                {
                    max_count = max_count + 1;
                    max_average = local_max;
                    local_max = 0;
                }
            }
        }
        i = i + 1;
    }
    return max_count;
}

void get_resp_rate_thread()
{
    printf("Resp-start\r\n");
    check_voltage();
    printf("Calculating Respitory Rate...\n\r");
    if (RUN_RESP_RATE || (available_energy > RESP_RATE_ENERGY && cap_voltage > MIN_VOLTAGE))
    {

        int resp_count = 0;
        //while (1)
        {
            //printf("Resp # %d", resp_count);
            int total_windows = 2;
            int breath_count = 0;
            for (int window = 0; window < total_windows; window++)
            {
                const int SAMPLE_SIZE = 300;
                float samples[SAMPLE_SIZE] = {};
                float sum = 0;

                for (int i = 0; i < SAMPLE_SIZE;)
                {
                    barometer.update();
                    while (!barometer.get_buffer_full())
                        barometer.update();
                    smart_ppe_ble.updatePressure(
                        barometer.get_delta_timestamp(true),
                        barometer.get_measurement_frequencyx100(),
                        barometer.get_pressure_array(),
                        barometer.get_pressure_buffer_size());

                    smart_ppe_ble.updateDataReady(smart_ppe_ble.PRESSURE);

                    barometer.clear_buffers();

                    temp.update();
                    while (!temp.getBufferFull())
                        temp.update();

                    smart_ppe_ble.updateTemperature(
                        temp.getDeltaTimestamp(true),
                        temp.getFrequencyx100(),
                        temp.getBuffer(),
                        temp.getBufferSize());
                    smart_ppe_ble.updateDataReady(smart_ppe_ble.TEMPERATURE);

                    uint16_t *buffer = temp.getBuffer();
                    for (int j = 0; j < temp.getBufferSize(); j++)
                    {
                        printf("Data %d\r\n", buffer[j]);
                        samples[i] = (buffer[j] / 100.0);
                        sum = sum + samples[i];
                        i++;
                    }
                    temp.clearBuffer();
                }
                ThisThread::sleep_for(50ms);
                // convert to floating points

                float mean = (sum) / SAMPLE_SIZE;
                printf("Mean %f\r\n", mean);
                breath_count += calc_resp_rate(samples, SAMPLE_SIZE, mean);
            }

            printf("Breath Count %d\r\n", breath_count);
            smart_ppe_ble.updateRespiratoryRate(temp.getDeltaTimestamp(true), breath_count);
            smart_ppe_ble.updateDataReady(smart_ppe_ble.RESPIRATORY_RATE);
            fflush(stdout);
            ThisThread::sleep_for(1ms);
            resp_count = resp_count + 1;
        }
    }
    printf("Resp-end\r\n");
    check_voltage();
}
void sensor_thread()
{
    bus_control->spi_power(true);
    spi.frequency(5000000);

    bus_control->i2c_power(true);

    ThisThread::sleep_for(1000ms);

    temp.initialize();

    if (!barometer.initialize() || !barometer.set_fifo_full_interrupt(true))
    {
        LOG_WARNING("%s", "barometer failed to initialize");
        while (1)
        {
        };
    }

    if (!barometer.set_pressure_threshold(1100) || !barometer.enable_pressure_threshold(true, true, false))
    {
        LOG_WARNING("%s", "barometer setup failed");
        while (1)
        {
        };
    }

    while (1)
    {
        barometer.update();
        temp.update();

        if (barometer.get_buffer_full())
        {
            LOG_DEBUG("barometer buffer full. %u elements. timestamp = %llu. measurement frequency x100 = %lu", barometer.get_pressure_buffer_size(), barometer.get_delta_timestamp(false), barometer.get_measurement_frequencyx100());

            smart_ppe_ble.updatePressure(
                barometer.get_delta_timestamp(true),
                barometer.get_measurement_frequencyx100(),
                barometer.get_pressure_array(),
                barometer.get_pressure_buffer_size());

            smart_ppe_ble.updateDataReady(smart_ppe_ble.PRESSURE);

            barometer.clear_buffers();
        }

        if (temp.getBufferFull())
        {
            LOG_DEBUG("temperature buffer full. %u elements. timestamp = %llu. measurement frequency x100 = %lu", temp.getBufferSize(), temp.getDeltaTimestamp(false), temp.getFrequencyx100());

            smart_ppe_ble.updateTemperature(
                temp.getDeltaTimestamp(true),
                temp.getFrequencyx100(),
                temp.getBuffer(),
                temp.getBufferSize());

            smart_ppe_ble.updateDataReady(smart_ppe_ble.TEMPERATURE);

            temp.clearBuffer();
        }
        ThisThread::sleep_for(1ms);
    }
}

void t1()
{
    task_queue.call(led_thread);
}
void t2()
{
    task_queue.call(get_resp_rate_thread);
}
void init_sensors()
{
    bus_control->init();
    ThisThread::sleep_for(10ms);
    bus_control->spi_power(true);
    spi.frequency(5000000);

    bus_control->i2c_power(true);

    ThisThread::sleep_for(1000ms);

    temp.initialize();
    if (!barometer.initialize() || !barometer.set_fifo_full_interrupt(true))
    {
        LOG_WARNING("%s", "barometer failed to initialize");
        while (1)
        {
        };
    }

    if (!barometer.set_pressure_threshold(1100) || !barometer.enable_pressure_threshold(true, true, false))
    {
        LOG_WARNING("%s", "barometer setup failed");
        while (1)
        {
        };
    }
}
int main()
{
    swo.claim();

    BLE &ble = BLE::Instance();
    init_sensors();
    //lpt_led.attach(t1, TIME_LED_TASK);
    get_resp_rate_thread();
    //lpt_resp_rate.attach(t2, TIME_RESP_TASK);

    thread1.start(callback(&task_queue, &EventQueue::dispatch_forever));

    GattServerProcess ble_process(event_queue, ble);
    ble_process.on_init(callback(&smart_ppe_ble, &SmartPPEService::start));
    ble_process.start();

    return 0;
}
