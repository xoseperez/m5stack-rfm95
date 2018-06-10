/*

M5stack TTN Node

Copyright (C) 2018 by Xose Pérez <xose dot perez at gmail dot com>

This code requires LMIC library by Matthijs Kooijman
https://github.com/matthijskooijman/arduino-lmic

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#include <Arduino.h>
#include <M5Stack.h>
#include <lmic.h>

// Copy and rename credentials.sample.h to credentials.h
// and configure it with your node settings
#include "credentials.h"

void ttn_register(void (*callback)(uint8_t message));

// -----------------------------------------------------------------------------
// Configuration
// -----------------------------------------------------------------------------

//#define DEBUG_PORT            Serial      // Serial debug port
#define SERIAL_BAUD             115200      // Serial debug baud rate

#define TX_INTERVAL             60000       // Send message every these many millis

#define SLEEP_BETWEEN_MESSAGES  1           // Do sleep between messages
#define SLEEP_INTERVAL          30000       // Sleep for these many millis
#define SLEEP_DELAY             500         // Time between sleep blocks to keep IP5306 on
#define MESSAGE_TO_SLEEP_DELAY  2000        // Time after message before going to sleep

#define LORAWAN_PORT            1           // Port the messages will be sent to
#define LORAWAN_CONFIRMED_EVERY 0           // Send confirmed message every these many messages (0 means never)

// -----------------------------------------------------------------------------
// DEBUG
// -----------------------------------------------------------------------------

#ifdef DEBUG_PORT
#define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

// -----------------------------------------------------------------------------
// Custom messages
// -----------------------------------------------------------------------------

#define EV_QUEUED       100
#define EV_PENDING      101
#define EV_ACK          102
#define EV_RESPONSE     103
