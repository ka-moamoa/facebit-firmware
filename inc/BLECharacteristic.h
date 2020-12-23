#ifndef RWNICHARACTERISTIC_H_
#define RWNICHARACTERISTIC_H_

#include "ble/BLE.h"
#include "ble/gatt/GattCharacteristic.h"

/* Read, Write, Notify, Indicate  Characteristic declaration helper.
*
* @tparam T type of data held by the characteristic.
*/
template<typename T>
class ReadWriteNotifyIndicateCharacteristic : public GattCharacteristic {
public:
    /**
     * Construct a characteristic that can be read or written and emit
     * notification or indication.
     *
     * @param[in] uuid The UUID of the characteristic.
     * @param[in] initial_value Initial value contained by the characteristic.
     */
    ReadWriteNotifyIndicateCharacteristic(const UUID & uuid, const T& initial_value) :
        GattCharacteristic(
            /* UUID */ uuid,
            /* Initial value */ &_value,
            /* Value size */ sizeof(_value),
            /* Value capacity */ sizeof(_value),
            /* Properties */ GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
                                GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE,
            /* Descriptors */ nullptr,
            /* Num descriptors */ 0,
            /* variable len */ false
        ),
        _value(initial_value) 
        {
            
        }

    /**
     * Get the value of this characteristic.
     *
     * @param[in] server GattServer instance that contain the characteristic
     * value.
     * @param[in] dst Variable that will receive the characteristic value.
     *
     * @return BLE_ERROR_NONE in case of success or an appropriate error code.
     */
    ble_error_t get(GattServer &server, T& dst) const
    {
        uint16_t value_length = sizeof(dst);
        return server.read(getValueHandle(), &dst, &value_length);
    }

    /**
     * Assign a new value to this characteristic.
     *
     * @param[in] server GattServer instance that will receive the new value.
     * @param[in] value The new value to set.
     * @param[in] local_only Flag that determine if the change should be kept
     * locally or forwarded to subscribed clients.
     */
    ble_error_t set(GattServer &server, const uint8_t &value, bool local_only = false) const
    {
        return server.write(getValueHandle(), &value, sizeof(value), local_only);
    }

private:
    uint8_t _value;
};

#endif // RWNICHARACTERISTIC_H_