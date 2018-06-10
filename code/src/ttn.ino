/*

M5stack TTN Node
LMIC module

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

#include "general.h"
#include <hal/hal.h>
#include <SPI.h>
#include <vector>

#ifndef CFG_eu868
    // Currently, this sketch only supports EU868 network, edit your
    // lmic_project_config.h file in the library to change the setting
    #error "This script is meant to connect to TTN EU network at 868MHz"
#endif

// -----------------------------------------------------------------------------
// M5Stack RFM95 mapping
// http://github.com/xoseperez/m5stack-rfm95
// -----------------------------------------------------------------------------

#define SCK_GPIO        18
#define MISO_GPIO       19
#define MOSI_GPIO       23
#define NSS_GPIO        05
#define RESET_GPIO      36
#define DIO0_GPIO       26
#define DIO1_GPIO       16
#define DIO2_GPIO       17

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

// LMIC GPIO configuration
const lmic_pinmap lmic_pins = {
    .nss = NSS_GPIO,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RESET_GPIO,
    .dio = {DIO0_GPIO, DIO1_GPIO, DIO2_GPIO},
};

#if !defined(USE_ABP) && !defined(USE_OTAA)
    #error "Define either USE_ABP or USE_OTAA in credentials.h"
#endif

#ifdef USE_ABP
// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }
#endif

#ifdef USE_OTAA
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8); }
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16); }
#endif

std::vector<void(*)(uint8_t message)> _lmic_callbacks;

// -----------------------------------------------------------------------------
// Private methods
// -----------------------------------------------------------------------------

void _ttn_callback(uint8_t message) {
    for (uint8_t i=0; i<_lmic_callbacks.size(); i++) {
        (_lmic_callbacks[i])(message);
    }
}

// LMIC library will call this method when an event is fired
void onEvent(ev_t event) {

    if (EV_TXCOMPLETE == event) {

        if (LMIC.txrxFlags & TXRX_ACK) {
            _ttn_callback(EV_ACK);
        }

        if (LMIC.dataLen) {
            _ttn_callback(EV_RESPONSE);
        }

    }

    // Send message callbacks
    _ttn_callback(event);

}

// -----------------------------------------------------------------------------
// Public methods
// -----------------------------------------------------------------------------

void ttn_register(void (*callback)(uint8_t message)) {
    _lmic_callbacks.push_back(callback);
}

size_t ttn_response_len() {
    return LMIC.dataLen;
}

void ttn_response(uint8_t * buffer, size_t len) {
    for (uint8_t i = 0; i < LMIC.dataLen; i++) {
        buffer[i] = LMIC.frame[LMIC.dataBeg + i];
    }
}

bool ttn_setup() {

    // SPI interface
    SPI.begin(SCK_GPIO, MISO_GPIO, MOSI_GPIO, NSS_GPIO);

    // LMIC init
    return ( 1 == os_init_ex( (const void *) &lmic_pins ) );

}

void ttn_join() {

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    #ifdef USE_ABP

        // Set static session parameters. Instead of dynamically establishing a session
        // by joining the network, precomputed session parameters are be provided.
        uint8_t appskey[sizeof(APPSKEY)];
        uint8_t nwkskey[sizeof(NWKSKEY)];
        memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
        memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
        LMIC_setSession(0x1, DEVADDR, nwkskey, appskey);

        // Set up the channels used by the Things Network, which corresponds
        // to the defaults of most gateways. Without this, only three base
        // channels from the LoRaWAN specification are used, which certainly
        // works, so it is good for debugging, but can overload those
        // frequencies, so be sure to configure the full frequency range of
        // your network here (unless your network autoconfigures them).
        // Setting up channels should happen after LMIC_setSession, as that
        // configures the minimal channel set.
        LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
        LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
        LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

        // If using a mono-channel gateway disable all channels
        // but the one the gateway is listening to
        //LMIC_disableChannel(0);
        //LMIC_disableChannel(1);
        //LMIC_disableChannel(2);
        //LMIC_disableChannel(3);
        //LMIC_disableChannel(4);
        //LMIC_disableChannel(5);
        //LMIC_disableChannel(6);
        //LMIC_disableChannel(7);
        //LMIC_disableChannel(8);

        // TTN defines an additional channel at 869.525Mhz using SF9 for class B
        // devices' ping slots. LMIC does not have an easy way to define set this
        // frequency and support for class B is spotty and untested, so this
        // frequency is not configured here.

        // Disable link check validation
        LMIC_setLinkCheckMode(0);

        // TTN uses SF9 for its RX2 window.
        LMIC.dn2Dr = DR_SF9;

        // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
        LMIC_setDrTxpow(DR_SF12, 14);

        // Enable ADR
        //LMIC_setAdrMode(true);

        // Trigger a false joined
        _ttn_callback(EV_JOINED);

    #endif // USE_ABP

}

void ttn_send(uint8_t * data, size_t len, uint8_t port, bool confirmed){

    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        _ttn_callback(EV_PENDING);
        return;
    }

    // Prepare upstream data transmission at the next possible time.
    // Parameters are port, data, length, confirmed
    LMIC_setTxData2(port, data, len, confirmed ? 1 : 0);

    _ttn_callback(EV_QUEUED);

}

void ttn_loop() {
    os_runloop_once();
}
