#include "mbed.h"
#include "LowPowerTicker.h"

#include "SWO.h"
#include "SWOLogger.h"

#include "BusControl.h"
<<<<<<< HEAD

#include "FaceBitState.hpp"

FaceBitState facebit;

static events::EventQueue task_queue(16 * EVENTS_EVENT_SIZE);
Thread thread1;

SWO_Channel swo("channel");

void blink()
{
    while(1)
    {
        BusControl::get_instance()->blink_led();
        ThisThread::sleep_for(1s);
    }
}

int main()
{
    BusControl::get_instance()->init();

    swo.claim();
    LOG_INFO("%s", "PROGRAM STARTING");

    thread1.start(blink);

    facebit.run();
=======
#include "Barometer.hpp"
#include "Si7051.h"

#include "SmartPPEService.h"
#include "BCG.h"

DigitalOut led(LED1);
BusControl *bus_control = BusControl::get_instance();

SWO_Channel SWO; // for SWO logging
// I2C i2c(I2C_SDA0, I2C_SCL0);
SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);

// Si7051 temp(&i2c);
// Barometer barometer(&spi, BAR_CS, BAR_DRDY);
BCG bcg(&spi, IMU_INT1, IMU_CS);

SWO_Channel swo("channel");

using namespace std::literals::chrono_literals;

const static char DEVICE_NAME[] = "SMARTPPE";

static events::EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

Thread *thread1;
// Thread *thread2;
Thread *thread3;

namespace smartppe
{
    BLE &ble = BLE::Instance();
}

GattServerProcess ble_process(event_queue, smartppe::ble);
SmartPPEService smart_ppe_ble;

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

// void sensor_thread()
// {   
//     bus_control->spi_power(true);
//     spi.frequency(5000000);

//     bus_control->i2c_power(true);

//     ThisThread::sleep_for(1000ms);

//     temp.initialize();
    
//     if (!barometer.initialize() || !barometer.set_fifo_full_interrupt(true))
//     {
//         LOG_WARNING("%s", "barometer failed to initialize");
//         while(1) {};
//     }

//     if (!barometer.set_pressure_threshold(1100) || !barometer.enable_pressure_threshold(true, true, false))
//     {
//         LOG_WARNING("%s", "barometer setup failed");
//         while(1) {};
//     }

//     while(1)
//     {
//         barometer.update();
//         temp.update();

//         if (barometer.get_buffer_full())
//         {
//             LOG_DEBUG("barometer buffer full. %u elements. timestamp = %llu. measurement frequency x100 = %lu", barometer.get_pressure_buffer_size(), barometer.get_delta_timestamp(false), barometer.get_measurement_frequencyx100());
            
//             smart_ppe_ble.updatePressure(
//                 barometer.get_delta_timestamp(true), 
//                 barometer.get_measurement_frequencyx100(), 
//                 barometer.get_pressure_array(), 
//                 barometer.get_pressure_buffer_size());
            
//             smart_ppe_ble.updateDataReady(smart_ppe_ble.PRESSURE);


//             barometer.clear_buffers();
//         }

//         if (temp.getBufferFull())
//         {
//             LOG_DEBUG("temperature buffer full. %u elements. timestamp = %llu. measurement frequency x100 = %lu", temp.getBufferSize(), temp.getDeltaTimestamp(false), temp.getFrequencyx100());

//             smart_ppe_ble.updateTemperature(
//                 temp.getDeltaTimestamp(true), 
//                 temp.getFrequencyx100(),
//                 temp.getBuffer(),
//                 temp.getBufferSize());

//             smart_ppe_ble.updateDataReady(smart_ppe_ble.TEMPERATURE);

//             temp.clearBuffer();
//         }

//         ThisThread::sleep_for(1ms);
//     }
// }

void bcg_thread()
{   
    while(!ble_process.is_connected())
    {
        ThisThread::sleep_for(1s);
    }

    ThisThread::sleep_for(1s);

    Timer ble_send_timeout;
    while(1)
    {
        smart_ppe_ble.updateTime(time(NULL));
        set_time(smart_ppe_ble.getTime());
        LOG_INFO("%s", "taking bcg reading...");
        if (bcg.bcg(20s))
        {
            ble_send_timeout.start();
            while(bcg.get_buffer_size() > 0 && ble_send_timeout.read_ms() < 5000)
            {
                if (smart_ppe_ble.getDataReady() == smart_ppe_ble.NO_DATA)
                {
                    BCG::HR_t hr = bcg.get_buffer_element();
                    LOG_INFO("Sending HR = %u", hr.rate);
                    smart_ppe_ble.updateHeartRate((uint64_t)hr.timestamp, hr.rate);
                }
                
                smart_ppe_ble.updateDataReady(smart_ppe_ble.HEART_RATE);
                ThisThread::sleep_for(20ms);
            }
            ble_send_timeout.stop();
            ble_send_timeout.reset();
        }
        ThisThread::sleep_for(1s);        
    }
}

int main()
{
    swo.claim();
    
    bus_control->init();
    ThisThread::sleep_for(10ms);

    thread1 = new Thread(osPriorityNormal, 512);
    // thread2 = new Thread();
    thread3 = new Thread();

    thread1->start(led_thread);
    // thread2->start(sensor_thread);
    thread3->start(bcg_thread);

    ble_process.on_init(callback(&smart_ppe_ble, &SmartPPEService::start));

    ble_process.start();
>>>>>>> data-collection

    return 0;
}
