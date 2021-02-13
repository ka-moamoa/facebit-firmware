#include "Broadcast.hpp"
#include "gatt_server_process.h"
#include "SmartPPEService.h"

    // ble_process.on_init(callback(&smart_ppe_ble, &SmartPPEService::start));

    // ble_process.start();

Broadcast::Broadcast()
{
}

Broadcast::~Broadcast()
{
}

bool Broadcast::broadcast_data()
{
    events::EventQueue ble_queue(16 * EVENTS_EVENT_SIZE);

    BLE &ble = BLE::Instance();
    GattServerProcess ble_process(ble_queue, ble);
    SmartPPEService smart_ppe_ble;

}