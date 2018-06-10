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
#include <rom/rtc.h>

// Message counter, stored in RTC memory, survives deep sleep
RTC_DATA_ATTR uint8_t count = 0;

// Number of sleeping intervals left, stored in RTC memory, survives deep sleep
RTC_DATA_ATTR uint8_t sleep_intervals = 0;

// Whether to go deep sleep or power off
bool _poweroff = false;

// -----------------------------------------------------------------------------
// Application
// -----------------------------------------------------------------------------

void send() {

    char buffer[64];
    snprintf(buffer, sizeof(buffer),"[TTN] Sending: 0x%02X\n", count);
    screen_print(buffer, LIGHTGREY);

    uint8_t data[1] = { count };
    bool confirmed = (LORAWAN_CONFIRMED_EVERY > 0) & (count % LORAWAN_CONFIRMED_EVERY == 0);
    ttn_send(data, 1, LORAWAN_PORT, confirmed);

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
    if (EV_PENDING == message) screen_print("[TTN] Message discarded\n", RED);
    if (EV_QUEUED == message) screen_print("[TTN] Message queued\n");

    if (EV_TXCOMPLETE == message) {

        screen_print("[TTN] Message sent\n", LIGHTGREY);

        #if SLEEP_BETWEEN_MESSAGES

            // Show the going to sleep message on the screen
            char buffer[64];
            if (_poweroff) {
                snprintf(buffer, sizeof(buffer), "[M5S] Power off in %u seconds\n", (int) (MESSAGE_TO_SLEEP_DELAY / 1000));
            } else {
                snprintf(buffer, sizeof(buffer), "[M5S] Going to sleep in %u seconds\n", (int) (MESSAGE_TO_SLEEP_DELAY / 1000));
            }
            screen_print(buffer);
            delay(MESSAGE_TO_SLEEP_DELAY);
            screen_off();

            // Set the left most button to wake the board
            sleep_interrupt(BUTTON_A_PIN, LOW);

            // If power off sleep forever
            if (_poweroff) sleep_forever();

            // We sleep in blocks of 30' because of this:
            // http://forum.m5stack.com/topic/62/ip5306-automatic-standby
            // so we calculate the number of blocks to sleep

            // We sleep for the interval between messages minus the current millis
            // this way we distribute the messages evenly every TX_INTERVAL millis
            uint32_t sleep_for = TX_INTERVAL - millis();

            // How many sleep blocks do we have to sleep?
            sleep_intervals = sleep_for / SLEEP_INTERVAL;

            // Trigger the deep sleep mode
            // The first block might be shorter than SLEEP_INTERVAL
            sleep_millis(sleep_for % SLEEP_INTERVAL);

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

    // Awake from deep sleep (reason 5)?
    if (5 == rtc_get_reset_reason(0)) {

        // Is the button pressed (HIGH means "no")?
        if (digitalRead(BUTTON_A_PIN) == HIGH) {

            // If we are not done yet...
            if (sleep_intervals > 0) {

                // Update the number of intervals left
                --sleep_intervals;

                // Delay a bit so the IP5306 notices it
                delay(SLEEP_DELAY);

                // And go back to sleep
                sleep_interrupt(BUTTON_A_PIN, LOW);
                sleep_millis(SLEEP_INTERVAL - millis());

            }
        }
    }

    // Debug
    #if DEBUG_PORT
        DEBUG_PORT.begin(SERIAL_BAUD);
    #endif

    // Buttons
    pinMode(BUTTON_A_PIN, INPUT_PULLUP);
    pinMode(BUTTON_B_PIN, INPUT_PULLUP);
    pinMode(BUTTON_C_PIN, INPUT_PULLUP);

    // Setup the screen
    screen_setup();

    // Hello
    screen_print("M5STACK-RFM95 TTN EXAMPLE\n", BLUE);

    // TTN setup
    if (!ttn_setup()) {
        screen_print("[M5S] Could not find the radio module!\n", RED);
        delay(MESSAGE_TO_SLEEP_DELAY);
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
        _poweroff = true;
    }

    ttn_loop();

}
