/*
 * Animated GIFs Display Code (originally for SmartMatrix LED panels)
 * Ported to ESP8266 GPN17-Badge
 *
 * Written by: Craig A. Lindley
 * Ported  by: Tobias Mädel (@manawyrm)
 *
 * Copyright (c) 2014 Craig A. Lindley
 * Refactoring by Louis Beaudoin (Pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <GPNBadge.hpp>
#include <FS.h>

#include "rboot.h"
#include "rboot-api.h"
#include "GifDecoder.h"
#include "FilenameFunctions.h"

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Badge badge;

#define DISPLAY_TIME_SECONDS 10

#define ENABLE_SCROLLING  1

// range 0-255
const int defaultBrightness = 255;

/* SmartMatrix configuration and memory allocation */
#define COLOR_DEPTH 24                  // known working: 24, 48 - If the sketch uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint8_t kMatrixWidth = 64;        // known working: 32, 64, 96, 128
const uint8_t kMatrixHeight = 64;       // known working: 16, 32, 48, 64

/* template parameters are maxGifWidth, maxGifHeight, lzwMaxBits
 * 
 * The lzwMaxBits value of 12 supports all GIFs, but uses 16kB RAM
 * lzwMaxBits can be set to 10 or 11 for small displays, 12 for large displays
 * All 32x32-pixel GIFs tested work with 11, most work with 10
 */
GifDecoder<kMatrixWidth, kMatrixHeight, 12> decoder;

int num_files;

void screenClearCallback(void) {
   // tft.fillScreen(BLACK);
}

void updateScreenCallback(void) {
  tft.writeFramebuffer();
}

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
    tft.drawPixel(x, y, tft.Color565(red,green,blue));
}

// Setup method runs once, when the sketch starts
void setup() {
    decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    decoder.setDrawPixelCallback(drawPixelCallback);

    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);

    //Serial.begin(115200);

    badge.init();
    badge.setBacklight(true);
    delay(300);
    badge.setGPIO(MQ3_EN,1);
    rboot_config rboot_config = rboot_get_config();
    SPIFFS.begin();
    File f = SPIFFS.open("/rom"+String(rboot_config.current_rom),"w");
    f.println("GIF-Badge\n");
    tft.begin();
    tft.setTextSize(3);
    tft.setRotation(2);
    tft.scroll(32);
    tft.fillScreen(WHITE);
    tft.setTextColor(RED);
    
    openGifFilenameByIndex("hi", 1);
    decoder.startDecoding();

}


void loop()
{
    decoder.decodeFrame();
    delay(1);
}
