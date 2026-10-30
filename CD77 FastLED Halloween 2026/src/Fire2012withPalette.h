#pragma once
/*
  The following code is a slight modification of Mark Kriegsman's Fire2012WithPalette:
  https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino

  Non-blocking state machine version: FastLED.delay(1000 / FRAMES_PER_SECOND)
  is replaced by an internal frame timer, and the heat array lives in the class.

  COOLING : how much the air cools as it rises. Less = taller flames. Default 55, range 20-100
  SPARKING: chance (out of 255) a new spark is lit. Higher = more roaring. Default 120, range 50-200

  Usage (call every loop pass):
    Fire2012WithPalette.update(HeatColors_p, 75, 80);                          // palette, cooling, sparking
    Fire2012WithPalette.update(IceColors_p, 75, 80, 30);                       // optional 4th arg: frames per second

  Other palette ideas:
    CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White)
    CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White)
    CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::White)
*/
#include "Config.h"

bool gReverseDirection = false;

class Fire2012WithPalette_SM {
public:
  void reset() { for (int i = 0; i < NUM_LEDS; i++) _heat[i] = 0; _started = false; }

  void update(const CRGBPalette16& pal, uint8_t cooling, uint8_t sparking,
              uint8_t fps = FRAMES_PER_SECOND) {
    uint32_t now      = millis();
    uint16_t interval = 1000 / (fps ? fps : 1);
    if (_started && (now - _lastFrame) < interval) return;
    _started   = true;
    _lastFrame = now;

    random16_add_entropy(random(0, 65535));

    // Step 1. Cool down every cell a little
    for (int i = 0; i < NUM_LEDS; i++)
      _heat[i] = qsub8(_heat[i], random8(0, ((cooling * 10) / NUM_LEDS) + 2));

    // Step 2. Heat from each cell drifts 'up' and diffuses a little
    for (int k = NUM_LEDS - 1; k >= 2; k--)
      _heat[k] = (_heat[k - 1] + _heat[k - 2] + _heat[k - 2]) / 3;

    // Step 3. Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < sparking) {
      int y = random8(7);
      _heat[y] = qadd8(_heat[y], random8(160, 255));
    }

    // Step 4. Map from heat cells to LED colors
    for (int j = 0; j < NUM_LEDS; j++) {
      uint8_t colorindex = scale8(_heat[j], 240);
      int pixelnumber = gReverseDirection ? (NUM_LEDS - 1) - j : j;
      leds[pixelnumber] = ColorFromPalette(pal, colorindex);
    }
    FastLED.show();
  }

private:
  uint8_t  _heat[NUM_LEDS] = {0};
  bool     _started   = false;
  uint32_t _lastFrame = 0;
};

Fire2012WithPalette_SM Fire2012WithPalette;   // original: Fire2012WithPalette(cooling, sparking)
