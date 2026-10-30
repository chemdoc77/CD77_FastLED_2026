/*
  Halloweend2026.cpp (converted from Halloweend2026.ino)
  CD77_Halloween_Fun_2019 by Chemdoc77
  Based on code from different sources. See each header for the sources.

  Board : Lolin D32 (ESP32)   LEDs: 256 x WS2812B on GPIO 12, FastLED brightness 75
*/
#include <Arduino.h>
#include <FastLED.h>

#include "Config.h"
CRGB leds[NUM_LEDS];

#include "ropestuff.h"
#include "Halloween_chase.h"
#include "Fire2012withPalette.h"
#include "Rope_Fire2012.h"
#include "Flicker.h"
#include "CD77_palettecrossfade.h"   // moving pattern that crossfades palettes (after ropestuff.h)
#include  "CD77_slowchangepalette.h"
FadeToPaletteAnimation FadetoPaletteAnimation(leds, NUM_LEDS);   // instance used in Performance() shows
#include "LED_Effects.h"
#include "Time_performance.h"   // must come last: it uses the effects above

void setup() {
  delay(1000);   // power-up hold, runs once at start-up only
  FastLED.addLeds<CHIPSET, LED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTS, MAX_MILLIAMPS);
  set_max_power_indicator_LED(POWER_INDICATOR_PIN);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  RestartPerformance();
}

void loop() {
  //Time Performance code
  gTimeCode = millis() - gTimeCodeBase;
  Performance12();   // or Performance(), Performance2(), Performancex2(), Performancex3(), Performance4c(), Performance5(), Performance7(), Performance8()
}
