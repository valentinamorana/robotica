#include <FastLED.h>
#include <SoftwareSerial.h>

#define LED_PIN     6
#define NUM_LEDS    30
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
SoftwareSerial bluetooth(2, 3); // RX, TX

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  bluetooth.begin(9600);
  Serial.begin(9600);
}

void loop() {
  if (bluetooth.available()) {
    String command = bluetooth.readString();
    command.trim();
    
    if (command == "RED") {
      fill_solid(leds, NUM_LEDS, CRGB::Red);
    } else if (command == "GREEN") {
      fill_solid(leds, NUM_LEDS, CRGB::Green);
    } else if (command == "BLUE") {
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
    } else if (command == "OFF") {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
    } else if (command.startsWith("RGB:")) {
      // Formato: RGB:255,0,0
      int r = command.substring(4, command.indexOf(',')).toInt();
      int g = command.substring(command.indexOf(',') + 1, command.lastIndexOf(',')).toInt();
      int b = command.substring(command.lastIndexOf(',') + 1).toInt();
      fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
    }
    
    FastLED.show();
  }
}
