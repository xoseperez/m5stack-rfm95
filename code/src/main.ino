/*

M5stack TTN Node
Main module

Copyright (C) 2018 by Xose Pérez <xose dot perez at gmail dot com>

This sketch requires M5Stack library by M5Stack
https://github.com/m5stack/M5Stack

This sketch requires LMIC library by Matthijs Kooijman
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

#include "general.h"

// Message counter, stored in RTC memory, survives deep sleep
RTC_DATA_ATTR uint8_t count = 0;
bool _forever = false;

// -----------------------------------------------------------------------------
// Application
// -----------------------------------------------------------------------------

void send() {

    char buffer[64];
    snprintf(buffer, sizeof(buffer),"[TTN] Sending: 0x%02X\n", count);
    screen_print(buffer, LIGHTGREY);

    uint8_t data[1] = { count };
    ttn_send(data, 1, 1, 0);

    count++;

}

void callback(uint8_t message) {

    if (EV_JOINING == message) screen_print("[TTN] Joining...\n");
    if (EV_JOINED == message) screen_print("[TTN] Joined!\n");
    if (EV_JOIN_FAILED == message) screen_print("[TTN] Join failed\n", RED);
    if (EV_REJOIN_FAILED == message) screen_print("[TTN] Rejoin failed\n", RED);
    if (EV_RESET == message) screen_print("[TTN] Reset\n", BLUE);
    if (EV_LINK_DEAD == message) screen_print("[TTN] Link dead\n", RED);
    if (EV_ACK == message) screen_print("[TTN] ACK received\n");

    if (EV_TXCOMPLETE == message) {

        screen_print("[TTN] Message sent\n", LIGHTGREY);

        #if SLEEP_BETWEEN_MESSAGES

            char buffer[64];
            snprintf(buffer, sizeof(buffer), "[M5S] Going to sleep in %u seconds\n", (int) (PRE_SLEEP_DELAY / 1000));
            screen_print(buffer);

            delay(PRE_SLEEP_DELAY);
            sleep_interrupt(BUTTON_A_PIN, LOW);
            if (_forever) {
                sleep_forever();
            } else {
                sleep_millis(TX_INTERVAL - millis());
            }

        #endif

    }

    if (EV_RESPONSE == message) {

        screen_print("[TTN] Response: ");

        size_t len = ttn_response_len();
        uint8_t data[len];
        ttn_response(data, len);

        char buffer[6];
        for (uint8_t i=0; i<len; i++) {
            snprintf(buffer, sizeof(buffer), "0x%02X ", data[i]);
            screen_print(buffer);
        }

        screen_print("\n");

    }

}

void setup() {

    // Debug
    #if DEBUG_PORT
        DEBUG_PORT.begin(SERIAL_BAUD);
    #endif

    // initialize the M5Stack object
    M5.begin();

    // Setup the screen
    screen_setup();

    // Hello
    screen_print("M5STACK-RFM95 TTN EXAMPLE\n", BLUE);

    // TTN setup
    if (!ttn_setup()) {
        screen_print("[M5S] Could not find the radio module!\n", RED);
        delay(PRE_SLEEP_DELAY);
        sleep_forever();
    }

    ttn_register(callback);
    ttn_join();

}

void loop() {

    // Send every TX_INTERVAL millis
    static uint32_t last = 0;
    if (0 == last || millis() - last > TX_INTERVAL) {
        last = millis();
        send();
    }

    // Send on button press
    M5.update();
    if (M5.BtnA.wasPressed()) {
        send();
    }
    if (M5.BtnC.wasPressed()) {
        screen_print("[M5S] Set to sleep forever\n");
        _forever = true;
    }

    ttn_loop();

}