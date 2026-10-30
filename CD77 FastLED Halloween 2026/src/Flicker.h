#pragma once
/*
  Flicker by Chemdoc77
  Based on code by u/locuester of Reddit
  https://www.reddit.com/r/FastLED/comments/diffut/flicker_effect/

  Non-blocking state machine version.
  States: FLICKER_IDLE -> FLICKER_ON / FLICKER_OFF (random each step).
  The old delay(random16(min,max)) is replaced by a millis() deadline.

  Usage (call every loop pass):
    CD77_flicker.update(CRGB::Purple, 5, 20);   // color, min ms, max ms between flips
*/
#include "Config.h"

class CD77_flicker_SM {
public:
  enum State : uint8_t { FLICKER_IDLE, FLICKER_ON, FLICKER_OFF };

  void reset() { _state = FLICKER_IDLE; _nextChange = 0; }

  void update(CRGB color, uint16_t ranMin, uint16_t ranMax) {
    uint32_t now = millis();
    if (_state != FLICKER_IDLE && (int32_t)(now - _nextChange) < 0) return;  // still waiting

    _state = (random8(2) == 1) ? FLICKER_ON : FLICKER_OFF;
    if (_state == FLICKER_ON) fill_solid(leds, NUM_LEDS, color);
    else                      FastLED.clear();
    FastLED.show();

    uint16_t wait = (ranMax > ranMin) ? random16(ranMin, ranMax) : ranMin;
    _nextChange = now + wait;
  }

  State state() const { return _state; }

private:
  State    _state      = FLICKER_IDLE;
  uint32_t _nextChange = 0;
};

CD77_flicker_SM CD77_flicker;   // original function name: CD77_flicker(...) -> CD77_flicker.update(...)
