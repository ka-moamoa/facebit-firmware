/**
 * A Clock service that demonstrate the GattServer features.
 *
 * The clock service host three characteristics that model the current hour,
 * minute and second of the clock. The value of the second characteristic is
 * incremented automatically by the system.
 *
 * A client can subscribe to updates of the clock characteristics and get
 * notified when one of the value is changed. Clients can also change value of
 * the second, minute and hour characteristric.
 */
#include "SensorService.h"

SensorService::SensorService() :
    _barometer("485f4145-52b9-4644-af1f-7a6b9322490f", 0),
    _temperature("0a924ca7-87cd-4699-a3bd-abdcd9cf126a", 0),
    _sensor_service(
        /* uuid */ "51311102-030e-485f-b122-f8f381aa84ed",
        /* characteristics */ _sensor_characteristics,
        /* numCharacteristics */ sizeof(_sensor_characteristics) /
                                    sizeof(_sensor_characteristics[0])
    )
{
    /* update internal pointers (value, descriptors and characteristics array) */
    _sensor_characteristics[0] = &_barometer;
    _sensor_characteristics[1] = &_temperature;

    /* setup authorization handlers */
    _barometer.setWriteAuthorizationCallback(this, &SensorService::authorize_client_write);
    _temperature.setWriteAuthorizationCallback(this, &SensorService::authorize_client_write);
}

SensorService::~SensorService() {};

void SensorService::start(BLE &ble, events::EventQueue &event_queue)
{
    _server = &ble.gattServer();
    _event_queue = &event_queue;

    printf("Registering demo service\r\n");
    ble_error_t err = _server->addService(_sensor_service);

    if (err) {
        printf("Error %u during demo service registration.\r\n", err);
        return;
    }

    /* register handlers */
    _server->setEventHandler(this);

    printf("clock service registered\r\n");
    printf("service handle: %u\r\n", _sensor_service.getHandle());
    printf("barometer value handle %u\r\n", _barometer.getValueHandle());
    printf("thermometer value handle %u\r\n", _temperature.getValueHandle());
}

void SensorService::updateBarometer(float value)
{
    uint32_t pressurex1000 = value * 1000;

    ble_error_t err = _barometer.set(*_server, pressurex1000);
    if (err) {
        printf("write of the barometer value returned error %u\r\n", err);
        return;
    }
}

void SensorService::updateThermometer(float value)
{
    uint32_t tempx1000 = value * 1000;

    ble_error_t err = _temperature.set(*_server, tempx1000);
    if (err) {
        printf("write of the thermometer value returned error %u\r\n", err);
        return;
    }
}

void SensorService::onDataSent(const GattDataSentCallbackParams &params)
{
    printf("sent updates\r\n");
}

void SensorService::onDataWritten(const GattWriteCallbackParams &params)
{
    printf("data written:\r\n");
    printf("connection handle: %u\r\n", params.connHandle);
    printf("attribute handle: %u", params.handle);
    if (params.handle == _barometer.getValueHandle()) {
        printf(" (barometer characteristic)\r\n");
    } else if (params.handle == _temperature.getValueHandle()) {
        printf(" (temperature characteristic)\r\n");
    } else {
        printf("\r\n");
    }
    printf("write operation: %u\r\n", params.writeOp);
    printf("offset: %u\r\n", params.offset);
    printf("length: %u\r\n", params.len);
    printf("data: ");

    for (size_t i = 0; i < params.len; ++i) {
        printf("%02X", params.data[i]);
    }

    printf("\r\n");
}

void SensorService::onDataRead(const GattReadCallbackParams &params)
{
    printf("data read:\r\n");
    printf("connection handle: %u\r\n", params.connHandle);
    printf("attribute handle: %u", params.handle);
    if (params.handle == _barometer.getValueHandle()) {
        printf(" (barometer characteristic)\r\n");
    } else if (params.handle == _temperature.getValueHandle()) {
        printf(" (temperature characteristic)\r\n");
    } else {
        printf("\r\n");
    }
}

void SensorService::onUpdatesEnabled(const GattUpdatesEnabledCallbackParams &params)
{
    printf("update enabled on handle %d\r\n", params.attHandle);
}

void SensorService::onUpdatesDisabled(const GattUpdatesDisabledCallbackParams &params)
{
    printf("update disabled on handle %d\r\n", params.attHandle);
}

void SensorService::onConfirmationReceived(const GattConfirmationReceivedCallbackParams &params)
{
    printf("confirmation received on handle %d\r\n", params.attHandle);
}

void SensorService::authorize_client_write(GattWriteAuthCallbackParams *e)
{
    printf("characteristic %u write authorization\r\n", e->handle);

    if (e->offset != 0) {
        printf("Error invalid offset\r\n");
        e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
        return;
    }

    if (e->len != 1) {
        printf("Error invalid len\r\n");
        e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
        return;
    }

    e->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}