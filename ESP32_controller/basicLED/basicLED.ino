// Use this verions - https://github.com/teknynja/Adafruit_NeoPixel/tree/esp32_rmt_memory_allocation_fix_safe
// See: https://forum.arduino.cc/t/neopixel-crash-with-75-pixels-using-esp32-core-3-0-x/1273500/12
#include <Adafruit_NeoPixel.h>

#define PIN 12
#define N_LEDS 300 // 75 -80
#define MAX_BRIGHTNESS 127 // Choose a value between 2 and 255

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
}

void loop() {
  int r = 0;
  int g = 255;
  int b = 0;
  int brightness = 2; // 2 to 255 - 1 is not visible
  writeColor(r,g,b,brightness, 0, strip.numPixels());

  //chase(strip.Color(0, 255, 0, 10)); // Green
  // chase(strip.Color(0, 0, 255)); // Blue
}

void writeColor(int r, int g, int b, int brightness, int startLED, int stopLED) {
  if (brightness > MAX_BRIGHTNESS) {
    brightness = MAX_BRIGHTNESS;
  }

  // Apply brightness to RGB values directly
  r = (r * brightness) >> 8;
  g = (g * brightness) >> 8;
  b = (b * brightness) >> 8;
  for(uint16_t i = startLED; i < stopLED; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b)); // Draw new pixel
  }
  strip.show();
}

static void chase(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels()+4; i++) {
      
      strip.setPixelColor(i , c); // Draw new pixel
      strip.setPixelColor(i-4, 0); // Erase pixel a few steps back
      strip.show();
      delay(25);
  }
}