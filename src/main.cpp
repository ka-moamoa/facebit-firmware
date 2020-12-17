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

#include "mbed.h"
#include "events/mbed_events.h"
#include "ble/BLE.h"
#include "SWO.h"
#include "SWOLogger.h"
#include "Heartrate.h"

SWO_Channel SWO("channel");

static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);

/* Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
{
    event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

int main()
{
    LOG_DEBUG("%s", "BLE Test Starting...");

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(schedule_ble_events);
}
