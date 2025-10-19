#include <FastLED.h>
#include <SoftwareSerial.h>

#define LED_PIN     6
#define MIC_PIN     A0
#define NUM_LEDS    30
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
SoftwareSerial bluetooth(2, 3);

int brightness = 64;
int soundThreshold = 100;
bool soundMode = true;
int direction = 0; // 0=all, 1=left, 2=right, 3=center

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  bluetooth.begin(9600);
  Serial.begin(9600);
}

void loop() {
  // Control por sonido
  if (soundMode) {
    int soundLevel = analogRead(MIC_PIN);
    if (soundLevel > soundThreshold) {
      int intensity = map(soundLevel, soundThreshold, 1023, 0, 255);
      CRGB color = CHSV(map(soundLevel, 0, 1023, 0, 255), 255, intensity);
      
      applyDirection(color);
      FastLED.show();
      delay(50);
    } else {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  }
  
  // Control Bluetooth
  if (bluetooth.available()) {
    String command = bluetooth.readString();
    command.trim();
    
    if (command.startsWith("BRIGHTNESS:")) {
      brightness = command.substring(11).toInt();
      FastLED.setBrightness(brightness);
    } else if (command.startsWith("DIRECTION:")) {
      direction = command.substring(10).toInt();
    } else if (command == "SOUND_ON") {
      soundMode = true;
    } else if (command == "SOUND_OFF") {
      soundMode = false;
      fill_solid(leds, NUM_LEDS, CRGB::Black);
    }
    
    FastLED.show();
  }
}

void applyDirection(CRGB color) {
  switch(direction) {
    case 0: // All
      fill_solid(leds, NUM_LEDS, color);
      break;
    case 1: // Left half
      fill_solid(leds, NUM_LEDS/2, color);
      fill_solid(leds + NUM_LEDS/2, NUM_LEDS/2, CRGB::Black);
      break;
    case 2: // Right half
      fill_solid(leds, NUM_LEDS/2, CRGB::Black);
      fill_solid(leds + NUM_LEDS/2, NUM_LEDS/2, color);
      break;
    case 3: // Center
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      fill_solid(leds + NUM_LEDS/4, NUM_LEDS/2, color);
      break;
  }
}
