/*
 * PROGETTO: wearable "TEO ZEN" Arc Reactor [MASTER BUILD V2]
 * HARDWARE: ESP32-C3 SuperMini + Concentric Rings + 0.42" OLED
 */

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

// --- FIX FASTLED ---
#undef min
#undef max
#include <FastLED.h>

// --- HARDWARE CONFIG ---
#define LED_PIN 10
#define BUTTON_PIN 3
#define NUM_LEDS 36
#define BRIGHTNESS 40
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// --- DISPLAY CONFIG (U8g2) ---
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// --- LED ARRAYS ---
CRGB leds[NUM_LEDS];
const int R1_START = 0;
const int R1_LEN = 8;
const int R2_START = 8;
const int R2_LEN = 12;
const int R3_START = 20;
const int R3_LEN = 16;

// --- STATO DEL SISTEMA ---
int currentMode = 1;
const int TOTAL_MODES = 10;
unsigned long lastButtonPress = 0;

// ==========================================
// RISORSE GRAFICHE (PROGMEM)
// ==========================================
#define FRAME_WIDTH 32
#define FRAME_HEIGHT 32
#define FRAME_COUNT 10

// Animazione 32x32 centrata perfettamente su schermo 72x40
const int ANIM_X = (72 - 32) / 2; // = 20 (Centrato orizzontale)
const int ANIM_Y = (40 - 32) / 2; // = 4  (Centrato verticale, nessun taglio!)

static const unsigned char PROGMEM frames[FRAME_COUNT][128] = {
    {0, 0, 0, 0, 0, 0, 48, 0, 0, 0, 120, 0, 0, 0, 204, 0, 0, 7, 131, 128, 0, 14, 1, 192, 0, 12, 120, 64, 0, 4, 204, 192, 0, 4, 132, 192, 0, 4, 132, 192, 0, 4, 204, 192, 0, 12, 120, 64, 0, 14, 1, 192, 1, 231, 131, 128, 1, 32, 204, 0, 7, 56, 120, 0, 60, 15, 51, 128, 32, 193, 2, 192, 51, 243, 14, 96, 18, 18, 120, 28, 18, 18, 64, 6, 18, 18, 71, 198, 51, 243, 100, 100, 32, 193, 100, 36, 60, 15, 100, 100, 7, 56, 103, 228, 1, 96, 67, 198, 1, 224, 112, 28, 0, 0, 60, 60, 0, 0, 6, 192, 0, 0, 3, 128, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 48, 0, 0, 0, 112, 0, 0, 0, 222, 0, 0, 1, 135, 128, 0, 15, 0, 192, 0, 8, 120, 192, 0, 12, 204, 128, 0, 4, 132, 192, 0, 4, 132, 192, 0, 4, 204, 64, 0, 4, 120, 64, 0, 12, 1, 192, 0, 231, 135, 0, 1, 160, 236, 0, 31, 56, 60, 0, 52, 14, 25, 192, 48, 199, 3, 192, 19, 241, 62, 96, 18, 19, 120, 28, 18, 18, 96, 14, 50, 18, 103, 194, 35, 242, 100, 102, 56, 195, 100, 100, 60, 15, 100, 100, 6, 62, 199, 204, 3, 96, 67, 196, 1, 192, 112, 4, 0, 0, 12, 124, 0, 0, 4, 192, 0, 0, 7, 128, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 96, 0, 0, 0, 240, 0, 0, 0, 159, 128, 0, 1, 131, 128, 0, 14, 0, 128, 0, 14, 120, 128, 0, 8, 204, 128, 0, 12, 132, 192, 0, 4, 132, 192, 0, 4, 204, 96, 0, 6, 120, 192, 0, 6, 1, 192, 0, 119, 135, 0, 9, 211, 246, 0, 31, 24, 28, 0, 16, 12, 8, 192, 24, 199, 3, 224, 17, 241, 63, 32, 18, 17, 96, 16, 50, 18, 32, 14, 98, 18, 39, 198, 35, 226, 100, 98, 56, 198, 100, 100, 12, 2, 204, 100, 6, 126, 198, 204, 2, 228, 99, 204, 3, 128, 48, 12, 0, 0, 28, 60, 0, 0, 5, 216, 0, 0, 7, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 64, 0, 0, 1, 224, 0, 0, 1, 191, 128, 0, 1, 128, 128, 0, 3, 1, 128, 0, 14, 120, 128, 0, 8, 204, 128, 0, 8, 132, 192, 0, 12, 132, 96, 0, 4, 204, 96, 0, 6, 120, 192, 0, 2, 1, 0, 0, 54, 3, 0, 15, 251, 242, 0, 31, 24, 30, 0, 24, 12, 0, 224, 24, 198, 27, 240, 17, 227, 63, 48, 18, 49, 160, 16, 114, 19, 48, 8, 98, 50, 39, 206, 51, 226, 100, 98, 24, 198, 228, 102, 12, 6, 204, 100, 4, 118, 230, 204, 7, 252, 99, 136, 3, 0, 16, 8, 0, 0, 8, 108, 0, 0, 11, 248, 0, 0, 14, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 194, 0, 0, 1, 127, 0, 0, 1, 1, 0, 0, 2, 1, 128, 0, 6, 120, 128, 0, 12, 204, 192, 0, 8, 132, 224, 0, 12, 132, 32, 0, 4, 76, 224, 0, 6, 120, 128, 0, 2, 1, 0, 0, 50, 3, 0, 15, 251, 251, 0, 11, 9, 142, 0, 8, 12, 0, 96, 24, 198, 29, 240, 17, 243, 55, 144, 50, 17, 144, 16, 98, 17, 177, 8, 98, 19, 39, 204, 49, 226, 100, 102, 24, 198, 204, 34, 12, 4, 140, 102, 4, 228, 230, 76, 15, 252, 35, 136, 2, 0, 16, 24, 0, 0, 24, 8, 0, 0, 31, 248, 0, 0, 12, 32, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 143, 0, 0, 3, 255, 0, 0, 3, 3, 0, 0, 2, 1, 128, 0, 6, 120, 128, 0, 4, 204, 224, 0, 12, 132, 32, 0, 8, 132, 96, 0, 12, 204, 192, 0, 6, 120, 128, 0, 3, 1, 0, 2, 1, 1, 0, 15, 255, 127, 0, 12, 37, 198, 0, 12, 4, 4, 0, 24, 198, 31, 240, 19, 226, 19, 216, 114, 19, 16, 24, 98, 17, 177, 8, 114, 51, 167, 204, 19, 226, 196, 102, 24, 198, 140, 34, 8, 12, 236, 98, 9, 204, 38, 206, 15, 252, 35, 136, 0, 16, 16, 24, 0, 0, 16, 16, 0, 0, 31, 240, 0, 0, 8, 96, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 30, 0, 0, 3, 250, 0, 0, 2, 3, 0, 0, 2, 1, 128, 0, 6, 120, 192, 0, 4, 204, 96, 0, 12, 132, 96, 0, 8, 132, 192, 0, 12, 204, 128, 0, 14, 120, 128, 0, 2, 1, 128, 3, 1, 1, 128, 7, 253, 191, 0, 4, 63, 226, 0, 12, 4, 4, 0, 25, 198, 15, 176, 51, 226, 9, 248, 98, 50, 24, 8, 98, 19, 177, 8, 18, 17, 231, 204, 19, 227, 196, 100, 24, 198, 204, 38, 8, 12, 108, 98, 27, 136, 38, 198, 15, 248, 51, 140, 0, 48, 48, 24, 0, 0, 52, 48, 0, 0, 31, 176, 0, 0, 0, 224, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 30, 0, 0, 3, 246, 0, 0, 6, 131, 0, 0, 6, 1, 128, 0, 6, 120, 192, 0, 4, 204, 96, 0, 4, 132, 96, 0, 12, 132, 192, 0, 8, 204, 128, 0, 14, 120, 128, 0, 3, 0, 128, 3, 129, 134, 128, 2, 228, 191, 0, 6, 62, 224, 0, 12, 2, 102, 0, 56, 198, 15, 128, 49, 226, 12, 248, 98, 18, 24, 12, 50, 19, 113, 136, 19, 49, 231, 204, 17, 227, 196, 100, 24, 199, 108, 36, 16, 12, 108, 102, 31, 152, 38, 194, 9, 208, 35, 142, 0, 112, 32, 24, 0, 0, 60, 48, 0, 0, 31, 160, 0, 0, 0, 224, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 60, 0, 0, 2, 228, 0, 0, 7, 131, 0, 0, 4, 1, 192, 0, 6, 120, 64, 0, 4, 204, 96, 0, 4, 132, 192, 0, 4, 132, 192, 0, 12, 204, 128, 0, 8, 120, 128, 0, 15, 0, 192, 3, 193, 131, 128, 3, 96, 223, 128, 6, 126, 112, 0, 28, 10, 103, 0, 56, 194, 7, 128, 35, 226, 12, 248, 50, 18, 24, 28, 18, 18, 113, 132, 18, 51, 199, 204, 17, 225, 196, 100, 16, 199, 108, 36, 52, 14, 100, 102, 31, 24, 38, 66, 1, 176, 99, 142, 0, 224, 96, 28, 0, 0, 60, 48, 0, 0, 7, 96, 0, 0, 1, 192, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 56, 0, 0, 0, 56, 0, 0, 0, 238, 0, 0, 7, 131, 128, 0, 12, 1, 192, 0, 12, 120, 64, 0, 4, 204, 192, 0, 4, 132, 192, 0, 4, 132, 192, 0, 4, 204, 128, 0, 12, 120, 192, 0, 15, 1, 192, 1, 199, 135, 128, 1, 96, 204, 0, 7, 58, 120, 0, 60, 15, 51, 128, 32, 195, 6, 128, 51, 226, 14, 224, 18, 18, 120, 60, 18, 18, 97, 134, 18, 50, 71, 196, 51, 227, 100, 100, 48, 193, 108, 100, 60, 15, 100, 100, 31, 56, 102, 198, 1, 160, 67, 134, 0, 224, 112, 28, 0, 0, 60, 112, 0, 0, 6, 192, 0, 0, 3, 192, 0, 0, 0, 0}};

// ==========================================
// FUNZIONE "GIUBBOTTO ANTIPROIETTILE" (LED)
// ==========================================
void SafeSetLED(int index, CRGB color)
{
  if (index >= 0 && index < NUM_LEDS)
  {
    leds[index] = color;
  }
}

// ==========================================
// FUNZIONI GRAFICHE DISPLAY
// ==========================================

// Mostra l'animazione dell'Omino (Walk)
void displayAnimGears()
{
  static int frame = 0;
  static unsigned long lastFrameTime = 0;

  // Usa 42ms per rispettare la velocità originale dell'animazione
  if (millis() - lastFrameTime > 42)
  {
    lastFrameTime = millis();

    u8g2.clearBuffer();

    // LA CORREZIONE È QUI: usiamo drawBitmap (formato Adafruit MSB)
    // Il terzo parametro deve essere la larghezza divisa per 8 (byte per riga)
    u8g2.drawBitmap(ANIM_X, ANIM_Y, FRAME_WIDTH / 8, FRAME_HEIGHT, frames[frame]);

    u8g2.sendBuffer();

    frame = (frame + 1) % FRAME_COUNT;
  }
}

// Funzione generica per mostrare i nomi (come placeholder in attesa delle tue grafiche future)
void displayPlaceholderTesto(const char *line1, const char *line2)
{
  static int lastDrawnMode = -1; // Memoria per non ridisegnare inutilmente

  if (currentMode != lastDrawnMode)
  {
    u8g2.clearBuffer();

    u8g2.setFont(u8g2_font_ncenB08_tr);
    int x1 = (72 - u8g2.getStrWidth(line1)) / 2;
    int x2 = (72 - u8g2.getStrWidth(line2)) / 2;

    u8g2.drawStr(x1, 15, line1);
    u8g2.drawStr(x2, 32, line2);

    u8g2.sendBuffer();
    lastDrawnMode = currentMode;
  }
}

// MACCHINA A STATI DEL DISPLAY
void updateDisplayState()
{
  // Resettiamo "lastDrawnMode" dell'altra funzione se siamo su un'animazione,
  // altrimenti se torniamo su un placeholder fisso non si aggiornerà.

  switch (currentMode)
  {
  case 1:
    displayAnimGears(); // L'animazione 48x48 pulita
    break;
  case 2:
    displayPlaceholderTesto("SPIRAL", "BOUNCE");
    break;
  case 3:
    displayPlaceholderTesto("INDEP.", "SPIN");
    break;
  case 4:
    displayPlaceholderTesto("STARRY", "SKY");
    break;
  case 5:
    displayPlaceholderTesto("RAINBOW", "SPIRAL");
    break;
  case 6:
    displayPlaceholderTesto("CENTER", "EXPLODE");
    break;
  case 7:
    displayPlaceholderTesto("BICYCLE", "SPOKES");
    break;
  case 8:
    displayPlaceholderTesto("GRAVITY", "FALL");
    break;
  case 9:
    displayPlaceholderTesto("EXPAND", "PULSE");
    break;
  case 10:
    displayPlaceholderTesto("VARIABLE", "PULSE");
    break;
  }
}

// ==========================================
// SETUP
// ==========================================
void setup()
{
  Serial.begin(115200);
  delay(2000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(5, 6);
  u8g2.begin();

  // Testo Iniziale
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr((72 - u8g2.getStrWidth("TEO ZEN")) / 2, 25, "TEO ZEN");
  u8g2.sendBuffer();

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 800);

  FastLED.clear();
  SafeSetLED(R1_START, CRGB::Green);
  FastLED.show();
  delay(800);
  FastLED.clear();
  FastLED.show();
}

// ==========================================
// GLI EFFETTI LED (1-10)
// ==========================================

void effect1_OppositeTrails()
{
  fadeToBlackBy(leds, NUM_LEDS, 50);
  int pos16 = (millis() / 40) % R3_LEN;
  SafeSetLED(R3_START + pos16, CRGB::Blue);
  int pos12 = (R2_LEN - 1) - ((millis() / 40) % R2_LEN);
  SafeSetLED(R2_START + pos12, CRGB::Blue);
  int pos8 = (millis() / 40) % R1_LEN;
  SafeSetLED(R1_START + pos8, CRGB::Blue);
}

void effect2_SpiralBounce()
{
  fadeToBlackBy(leds, NUM_LEDS, 60);
  int pos = beatsin16(20, 0, NUM_LEDS - 1);
  int invertedPos = (NUM_LEDS - 1) - pos;
  SafeSetLED(invertedPos, CRGB::White);
}

void effect3_IndependentSpin()
{
  fadeToBlackBy(leds, NUM_LEDS, 50);
  int pos8 = (millis() / 60) % R1_LEN;
  SafeSetLED(R1_START + pos8, CRGB::Red);
  int pos12 = (millis() / 90) % R2_LEN;
  SafeSetLED(R2_START + pos12, CRGB::Green);
  int pos16 = (millis() / 130) % R3_LEN;
  SafeSetLED(R3_START + pos16, CRGB::Blue);
}

void effect4_StarrySky()
{
  fadeToBlackBy(leds, NUM_LEDS, 5);
  if (random8() < 15)
  {
    int pixel = random8(NUM_LEDS);
    uint8_t starType = random8(4);
    CRGB starColor;
    switch (starType)
    {
    case 0:
      starColor = CRGB(255, 255, 255);
      break;
    case 1:
      starColor = CRGB(200, 220, 255);
      break;
    case 2:
      starColor = CRGB(255, 255, 200);
      break;
    case 3:
      starColor = CRGB(255, 200, 150);
      break;
    }
    SafeSetLED(pixel, starColor);
  }
}

void effect5_RainbowSpiral()
{
  uint8_t startHue = millis() / 50;
  fill_rainbow(leds, NUM_LEDS, startHue, 7);
}

void effect6_Explosion()
{
  fadeToBlackBy(leds, NUM_LEDS, 40);
  uint8_t phase = (millis() / 120) % 8;
  static uint8_t lastPhase = 255;

  if (phase != lastPhase)
  {
    lastPhase = phase;
    if (phase == 0)
      for (int i = 0; i < 3; i++)
        SafeSetLED(R1_START + random8(R1_LEN), CRGB::Red);
    else if (phase == 1)
      for (int i = 0; i < 5; i++)
        SafeSetLED(R2_START + random8(R2_LEN), CRGB::Purple);
    else if (phase == 2)
      for (int i = 0; i < 7; i++)
        SafeSetLED(R3_START + random8(R3_LEN), CRGB::Blue);
  }
}

void effect7_BicycleSpokes()
{
  fadeToBlackBy(leds, NUM_LEDS, 80);
  uint8_t angle = millis() / 4;
  uint8_t coldHue = 120 + beatsin8(10, 0, 60);

  for (int arm = 0; arm < 4; arm++)
  {
    uint8_t armAngle = angle + (arm * 64);
    int pos8 = scale8(armAngle, R1_LEN);
    int pos12 = scale8(armAngle, R2_LEN);
    int pos16 = scale8(armAngle, R3_LEN);

    SafeSetLED(R1_START + pos8, CHSV(coldHue + (arm * 15), 255, 255));
    SafeSetLED(R2_START + pos12, CHSV(coldHue + (arm * 15), 255, 255));
    SafeSetLED(R3_START + pos16, CHSV(coldHue + (arm * 15), 255, 255));
  }
}

void effect8_GravityFall()
{
  fadeToBlackBy(leds, NUM_LEDS, 70);
  uint8_t tick = (millis() / 70) % 18;
  CRGB dropColor = CRGB::Cyan;

  if (tick < 8)
  {
    int step = tick + 1;
    SafeSetLED(R3_START + step, dropColor);
    SafeSetLED(R3_START + ((16 - step) % 16), dropColor);
  }
  else if (tick < 14)
  {
    int step = tick - 8;
    int rightPos = 5 - step;
    int leftPos = 7 + step;
    if (step == 5)
    {
      rightPos = 0;
      leftPos = 0;
    }
    SafeSetLED(R2_START + rightPos, dropColor);
    SafeSetLED(R2_START + leftPos, dropColor);
  }
  else
  {
    int step = tick - 14 + 1;
    SafeSetLED(R1_START + step, dropColor);
    SafeSetLED(R1_START + ((8 - step) % 8), dropColor);
  }
}

void effect9_ExpandingPulse()
{
  fadeToBlackBy(leds, NUM_LEDS, 10);
  static unsigned long lastUpdate = 0;
  static int currentRing = 0;
  static int colorCycle = 0;

  int currentDelay = beatsin16(4, 150, 800);

  if (millis() - lastUpdate > currentDelay)
  {
    lastUpdate = millis();
    currentRing++;
    if (currentRing > 2)
    {
      currentRing = 0;
      colorCycle++;
      if (colorCycle > 2)
        colorCycle = 0;
    }
    CRGB waveColor;
    if (colorCycle == 0)
      waveColor = CRGB::Red;
    else if (colorCycle == 1)
      waveColor = CRGB::Green;
    else
      waveColor = CRGB::Blue;

    if (currentRing == 0)
      for (int i = 0; i < R1_LEN; i++)
        SafeSetLED(R1_START + i, waveColor);
    else if (currentRing == 1)
      for (int i = 0; i < R2_LEN; i++)
        SafeSetLED(R2_START + i, waveColor);
    else if (currentRing == 2)
      for (int i = 0; i < R3_LEN; i++)
        SafeSetLED(R3_START + i, waveColor);
  }
}

void effect10_VariablePulse()
{
  static uint32_t phaseAccumulator = 0;
  uint16_t currentSpeed = beatsin16(6, 30, 120);
  phaseAccumulator += currentSpeed;
  uint8_t brightness = quadwave8(phaseAccumulator >> 8);
  uint8_t currentHue = millis() / 40;
  for (int i = 0; i < NUM_LEDS; i++)
    SafeSetLED(i, CHSV(currentHue, 255, brightness));
}

// ==========================================
// LOOP PRINCIPALE
// ==========================================
void loop()
{
  // GESTIONE PULSANTE
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    if (millis() - lastButtonPress > 250)
    {
      currentMode++;
      if (currentMode > TOTAL_MODES)
        currentMode = 1;
      lastButtonPress = millis();

      // Resetta lo schermo
      u8g2.clearBuffer();
      u8g2.sendBuffer();

      FastLED.clear();
      SafeSetLED(R1_START, CRGB::Blue);
      SafeSetLED(R1_START + 1, CRGB::Blue);
      FastLED.show();
      delay(80);
      FastLED.clear();
      FastLED.show();
    }
  }

  // AGGIORNAMENTO SCHERMO (Dinamico)
  updateDisplayState();

  // ESECUZIONE EFFETTI LED
  switch (currentMode)
  {
  case 1:
    effect1_OppositeTrails();
    break;
  case 2:
    effect2_SpiralBounce();
    break;
  case 3:
    effect3_IndependentSpin();
    break;
  case 4:
    effect4_StarrySky();
    break;
  case 5:
    effect5_RainbowSpiral();
    break;
  case 6:
    effect6_Explosion();
    break;
  case 7:
    effect7_BicycleSpokes();
    break;
  case 8:
    effect8_GravityFall();
    break;
  case 9:
    effect9_ExpandingPulse();
    break;
  case 10:
    effect10_VariablePulse();
    break;
  }

  FastLED.show();
}