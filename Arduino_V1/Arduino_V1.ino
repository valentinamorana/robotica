#include <FastLED.h>
#include <SoftwareSerial.h>

// ----------------- Config -----------------
#define LED_PIN     6
#define MIC_PIN     A0
#define NUM_LEDS    60
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// Bluetooth (SoftwareSerial evita usar pines 0 y 1)
SoftwareSerial bluetooth(2, 3);  // RX=D2, TX=D3

int  brightness      = 64;
int  soundThreshold  = 100;

// Modos
enum { MODE_SOUND = 0, MODE_MANUAL = 1 };
int modeSel = MODE_SOUND;   // arranca en modo sonido

int  direction       = 0;   // 0=all, 1=left, 2=right, 3=center

// ----------------- Funciones auxiliares -----------------
void applyDirection(CRGB color) {
  switch (direction) {
    case 0: // All
      fill_solid(leds, NUM_LEDS, color);
      break;
    case 1: // Left half
      fill_solid(leds, NUM_LEDS / 2, color);
      fill_solid(leds + NUM_LEDS / 2, NUM_LEDS / 2, CRGB::Black);
      break;
    case 2: // Right half
      fill_solid(leds, NUM_LEDS / 2, CRGB::Black);
      fill_solid(leds + NUM_LEDS / 2, NUM_LEDS / 2, color);
      break;
    case 3: // Center
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      fill_solid(leds + NUM_LEDS / 4, NUM_LEDS / 2, color);
      break;
  }
}

String readLineFrom(Stream &port) {
  if (!port.available()) return "";
  String s = port.readStringUntil('\n');
  s.trim();
  if (s.length() >= 2 && s.startsWith("\"") && s.endsWith("\"")) {
    s = s.substring(1, s.length() - 1);
  }
  s.toUpperCase();
  return s;
}

// ----------------- Procesador de comandos -----------------
void applyCmd(const String &cmdRaw) {
  if (!cmdRaw.length()) return;
  String cmd = cmdRaw;
  cmd.trim();
  cmd.toUpperCase();

  Serial.print("CMD: ");
  Serial.println(cmd);

  // --- Colores básicos ---
  if (cmd == "RED")   { modeSel = MODE_MANUAL; fill_solid(leds, NUM_LEDS, CRGB::Red);   FastLED.show(); return; }
  if (cmd == "GREEN") { modeSel = MODE_MANUAL; fill_solid(leds, NUM_LEDS, CRGB::Green); FastLED.show(); return; }
  if (cmd == "BLUE")  { modeSel = MODE_MANUAL; fill_solid(leds, NUM_LEDS, CRGB::Blue);  FastLED.show(); return; }
  if (cmd == "OFF")   { modeSel = MODE_MANUAL; fill_solid(leds, NUM_LEDS, CRGB::Black); FastLED.show(); return; }

  // --- RGB personalizado: RGB:r,g,b ---
  if (cmd.startsWith("RGB:")) {
    int p1 = cmd.indexOf(':'), p2 = cmd.indexOf(',', p1 + 1), p3 = cmd.indexOf(',', p2 + 1);
    if (p1 > 0 && p2 > p1 && p3 > p2) {
      int r = constrain(cmd.substring(p1 + 1, p2).toInt(), 0, 255);
      int g = constrain(cmd.substring(p2 + 1, p3).toInt(), 0, 255);
      int b = constrain(cmd.substring(p3 + 1).toInt(), 0, 255);
      modeSel = MODE_MANUAL;
      fill_solid(leds, NUM_LEDS, CRGB(r, g, b));
      FastLED.show();
      return;
    }
  }

  // --- Brillo ---
  if (cmd.startsWith("BRIGHTNESS:")) {
    int val = constrain(cmd.substring(11).toInt(), 0, 255);
    brightness = val;
    FastLED.setBrightness(brightness);
    FastLED.show();
    Serial.print("Brillo = "); Serial.println(brightness);
    return;
  }

  // --- Dirección ---
  if (cmd.startsWith("DIRECTION:")) {
    int val = constrain(cmd.substring(10).toInt(), 0, 3);
    direction = val;
    Serial.print("Dirección = "); Serial.println(direction);
    return;
  }

  // --- SOUND / MIC ON - OFF ---
  String c = cmd;
  c.replace("_", "");
  while (c.indexOf("  ") != -1) c.replace("  ", " ");

  if (c == "SOUND ON" || c == "SOUNDON" || c == "MIC ON" || c == "MICON") {
    modeSel = MODE_SOUND;
    Serial.println("🔊 Modo: SONIDO (reactivo al micrófono)");
    return;
  }

  if (c == "SOUND OFF" || c == "SOUNDOFF" || c == "MIC OFF" || c == "MICOFF") {
    modeSel = MODE_MANUAL;
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    Serial.println("🔇 Modo: MANUAL (micrófono desactivado)");
    return;
  }

  // --- HELP ---
  if (cmd == "HELP") {
    Serial.println("📘 COMANDOS DISPONIBLES:");
    Serial.println("  RED / GREEN / BLUE / OFF");
    Serial.println("  RGB:r,g,b (ej: RGB:255,0,50)");
    Serial.println("  BRIGHTNESS:n (0..255)");
    Serial.println("  DIRECTION:n (0..3)");
    Serial.println("  SOUND ON / SOUND OFF / MIC ON / MIC OFF");
    Serial.println("  HELP (muestra esta lista)");
    return;
  }

  Serial.println("❌ Comando no reconocido. Escribí HELP para ver opciones.");
}

// ----------------- Lector de comandos -----------------
void pollCommands() {
  String c = readLineFrom(Serial);
  if (c.length()) applyCmd(c);

  String cb = readLineFrom(bluetooth);
  if (cb.length()) applyCmd(cb);
}

// ----------------- Setup -----------------
void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
         .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);

  Serial.begin(9600);
  bluetooth.begin(9600);

  Serial.println(" Sistema listo. Escribí HELP para ver los comandos.");
  Serial.println("Modo inicial: SONIDO (reactivo al micrófono).");
}

// ----------------- Loop -----------------
void loop() {
  // Escucha comandos todo el tiempo
  pollCommands();

  // Solo si estamos en modo sonido
  if (modeSel == MODE_SOUND) {
    int soundLevel = analogRead(MIC_PIN);
    if (soundLevel > soundThreshold) {
      int intensity = map(soundLevel, soundThreshold, 1023, 0, 255);
      CRGB color = CHSV(map(soundLevel, 0, 1023, 0, 255), 255, intensity);
      applyDirection(color);
      FastLED.show();
      delay(40);
    } else {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
    }
  }
}
