/*

M5stack TTN Node
Screen module

Copyright (C) 2018 by Xose Pérez <xose dot perez at gmail dot com>

This code is based on the M5Stack TFT_Terminal example:
https://github.com/m5stack/M5Stack/blob/master/examples/Advanced/Display/TFT_Terminal/TFT_Terminal.ino

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

#define TEXT_HEIGHT         16      // Height of text to be printed and scrolled
                                    // The scrolling area must be a integral multiple of TEXT_HEIGHT
#define TEXT_WIDTH          9       // Witdh of a character
#define TOP_FIXED_AREA      0       // Number of lines in top fixed area (lines counted from top of screen)
#define BOT_FIXED_AREA      0       // Number of lines in bottom fixed area (lines counted from bottom of screen)
#define XMAX                240     // Bottom of screen area
#define YMAX                240     // Right of screen area
#define DEFAULT_COLOR       GREEN   // Default text colot

// The initial y coordinate of the top of the scrolling area
uint16_t _screen_start_y = 16;

// The initial y coordinate of the top of the bottom text line
//uint16_t _screen_pos_y = YMAX - BOT_FIXED_AREA - TEXT_HEIGHT;
uint16_t _screen_pos_y = 0;

// Keep track of the drawing x coordinate
uint16_t _screen_pos_x = 0;

// Keeps brightness value
uint8_t _screen_brightness = 0;

// TTNCat logo
extern const unsigned char ttncat_logo[];

// -----------------------------------------------------------------------------

int _screen_scroll_line() {

    int yTemp = _screen_start_y; // Store the old _screen_start_y, this is where we draw the next line

    // Use the record of line lengths to optimise the rectangle size we need to erase the top line
    // M5.Lcd.fillRect(0,_screen_start_y,blank[(_screen_start_y-TOP_FIXED_AREA)/TEXT_HEIGHT],TEXT_HEIGHT, TFT_BLACK);
    M5.Lcd.fillRect(0 ,_screen_start_y, XMAX, TEXT_HEIGHT, TFT_BLACK);

    // Change the top of the scroll area
    _screen_start_y += TEXT_HEIGHT;

    // The value must wrap around as the screen memory is a circular buffer
    // if (_screen_start_y >= YMAX - BOT_FIXED_AREA) _screen_start_y = TOP_FIXED_AREA + (_screen_start_y - YMAX + BOT_FIXED_AREA);
    if (_screen_start_y >= YMAX) _screen_start_y = 0;

    // Now we can scroll the display
    _screen_scroll_address(_screen_start_y);

    return  yTemp;

}

// Setup a portion of the screen for vertical scrolling
// We are using a hardware feature of the display, so we can only scroll in portrait orientation
void _screen_scroll_area(uint16_t tfa, uint16_t bfa) {
    M5.Lcd.writecommand(ILI9341_VSCRDEF); // Vertical scroll definition
    M5.Lcd.writedata(tfa >> 8);           // Top Fixed Area line count
    M5.Lcd.writedata(tfa);
    M5.Lcd.writedata((YMAX-tfa-bfa)>>8);  // Vertical Scrolling Area line count
    M5.Lcd.writedata(YMAX-tfa-bfa);
    M5.Lcd.writedata(bfa >> 8);           // Bottom Fixed Area line count
    M5.Lcd.writedata(bfa);
}

// Setup the vertical scrolling start address pointer
void _screen_scroll_address(uint16_t vsp) {
    M5.Lcd.writecommand(ILI9341_VSCRSADD); // Vertical scrolling pointer
    M5.Lcd.writedata(vsp>>8);
    M5.Lcd.writedata(vsp);
}

// -----------------------------------------------------------------------------

void screen_brightness(uint8_t brightness) {
    M5.Lcd.setBrightness(_screen_brightness);
    _screen_brightness = brightness;
}

void screen_brightness(uint8_t target, uint32_t slowness) {
    uint8_t step = _screen_brightness > target ? -1 : 1;
    uint8_t brightness = _screen_brightness;
    while (true) {
        M5.Lcd.setBrightness(brightness);
        if (brightness == target) break;
        brightness += step;
        delay(slowness);
    }
    _screen_brightness = target;

}

void screen_on(uint8_t slowness) {
    screen_brightness(255, slowness);
}

void screen_on() {
    screen_on(1);
}

void screen_off(uint8_t slowness) {
    screen_brightness(0, slowness);
}

void screen_off() {
    screen_off(1);
}

void screen_setup() {

    // Init screen
    M5.Lcd.begin();

    // Set screen off
    screen_brightness(0);

    // Show logo on power on
    if (1 == rtc_get_reset_reason(0) || 12 == rtc_get_reset_reason(0)) {
        M5.Lcd.drawBitmap(0, 0, 320, 240, (uint16_t *) ttncat_logo);
        screen_on(10);
        delay(1000);
        screen_off(10);
        delay(1000);
    }

    // Setup the TFT display
    M5.Lcd.fillScreen(TFT_BLACK);

    // Change colour for scrolling zone text
    M5.Lcd.setTextColor(DEFAULT_COLOR, TFT_BLACK);

    // Max brightness
    screen_on();

    // Setup scroll area
    // _screen_scroll_area(TOP_FIXED_AREA, BOT_FIXED_AREA);
    _screen_scroll_area(0, 0);

}


void screen_putchar(char data) {

    if (data == '\n' || _screen_pos_x > 311) {
        _screen_pos_x = 0;
        _screen_pos_y = _screen_scroll_line();
    }

    if (data > 31 && data < 128) {
        _screen_pos_x += M5.Lcd.drawChar(data, _screen_pos_x, _screen_pos_y, 2);
    }

}

void screen_print(const char * str, uint16_t color) {
    DEBUG_MSG(str);
    M5.Lcd.setTextColor(color, TFT_BLACK);
    while(*str) screen_putchar(*str++);
    M5.Lcd.setTextColor(DEFAULT_COLOR, TFT_BLACK);
}

void screen_print(const char * str) {
    screen_print(str, DEFAULT_COLOR);
}
