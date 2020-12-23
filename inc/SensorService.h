#ifndef CLOCKSERVICE_H_
#define CLOCKSERVICE_H_

#include "platform/Callback.h"
#include "events/EventQueue.h"
#include "ble/BLE.h"
#include "gatt_server_process.h"
#include "BLECharacteristic.h"

using mbed::callback;
using namespace std::literals::chrono_literals;

class SensorService : public ble::GattServer::EventHandler 
{
public:
    SensorService();
    ~SensorService();

    void start(BLE &ble, events::EventQueue &event_queue);

    void updateBarometer(float value);
    void updateThermometer(float value);

    // GattServer::EventHandler
private:
    /**
     * Handler called when a notification or an indication has been sent.
     */
    void onDataSent(const GattDataSentCallbackParams &params) override;
    
    /**
     * Handler called after an attribute has been written.
     */
    void onDataWritten(const GattWriteCallbackParams &params) override;


    /**
     * Handler called after an attribute has been read.
     */
    void onDataRead(const GattReadCallbackParams &params) override;

    /**
     * Handler called after a client has subscribed to notification or indication.
     *
     * @param handle Handle of the characteristic value affected by the change.
     */
    void onUpdatesEnabled(const GattUpdatesEnabledCallbackParams &params) override;

    /**
     * Handler called after a client has cancelled his subscription from
     * notification or indication.
     *
     * @param handle Handle of the characteristic value affected by the change.
     */
    void onUpdatesDisabled(const GattUpdatesDisabledCallbackParams &params) override;

    /**
     * Handler called when an indication confirmation has been received.
     *
     * @param handle Handle of the characteristic value that has emitted the
     * indication.
     */
    void onConfirmationReceived(const GattConfirmationReceivedCallbackParams &params) override;

    /**
     * Handler called when a write request is received.
     *
     * This handler verify that the value submitted by the client is valid before
     * authorizing the operation.
     */
    void authorize_client_write(GattWriteAuthCallbackParams *e);

    GattServer *_server = nullptr;
    events::EventQueue *_event_queue = nullptr;

    GattService _sensor_service;
    GattCharacteristic* _sensor_characteristics[2];

    ReadWriteNotifyIndicateCharacteristic<uint32_t> _barometer;
    ReadWriteNotifyIndicateCharacteristic<uint32_t> _temperature;
};



#endif // CLOCKSERVICE_H_