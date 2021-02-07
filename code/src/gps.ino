/*

Bluetooth GPS module

Copyright (C) 2021 by Xose Pérez <xose dot perez at gmail dot com>

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

#include <TinyGPS++.h>
TinyGPSPlus GPS_Decoder;

#ifdef GPS_VIA_BLUETOOTH

#include "BluetoothSerial.h"
BluetoothSerial GPS_Serial;

uint8_t address[6] = GPS_ADDRESS;

bool gps_connect() {
    GPS_Serial.begin("m5tracker", true);
    GPS_Serial.setPin(GPS_PIN);
    return GPS_Serial.connect(address);
}

bool gps_connected() {
    return GPS_Serial.connected();
}

void gps_disconnect() {
    GPS_Serial.end();
}

void gps_prime(uint32_t ms) {
    uint32_t last = millis();
    while (millis() - last < ms) gps_loop();    
}

void gps_loop() {
    if (GPS_Serial.available()) {
        GPS_Decoder.encode(GPS_Serial.read());
    }
}

bool gps_valid() {
    return GPS_Decoder.location.isValid();
}

double gps_longitude() {
    if (!GPS_Decoder.location.isValid()) return 0;
    return GPS_Decoder.location.lng();
}

double gps_latitude() {
    if (!GPS_Decoder.location.isValid()) return 0;
    return GPS_Decoder.location.lat();
}

double gps_altitude() {
    if (!GPS_Decoder.altitude.isValid()) return 0;
    return GPS_Decoder.altitude.meters();
}

float gps_hdop() {
    if (!GPS_Decoder.hdop.isValid()) return 0;
    return GPS_Decoder.hdop.hdop();
}

unsigned char gps_sats() {
    if (!GPS_Decoder.satellites.isValid()) return 0;
    return GPS_Decoder.satellites.value();
}

#endif

#ifdef GPS_VIA_SERIAL

// GPS shield
bool gps_connect() {
    // TODO
    return false;
}

void gps_disconnect() {
    // TODO
}

void gps_prime(uint32_t ms) {
    // TODO
}

void gps_loop() {
    // TODO
}

bool gps_valid() {
    // TODO
    return false;
}

double gps_longitude() {
    // TODO
    return 0;
}

double gps_latitude() {
    // TODO
    return 0;
}

double gps_altitude() {
    // TODO
    return 0;
}

float gps_hdop() {
    // TODO
    return 0;
}

unsigned char gps_sats() {
    return 0;
}

#endif