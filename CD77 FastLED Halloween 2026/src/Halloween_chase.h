#pragma once
/* Created by Chemdoc77 based on code by Zaphod Beeblewurdle in:
   https://plus.google.com/107029944060954069417/posts/6r7QakiLLvm
   and based on code by Jason Coon's Color Pulse in:
   https://plus.google.com/+JasonCoon1/posts/gpN7pEqRQUe

   Random-color idea based on Adafruit's Random Eyes sketch (W. Earl 10/16/11):
   https://learn.adafruit.com/random-spooky-led-eyes/assembly?view=all

   Non-blocking state machine version. FastLED.delay(50) between frames is now
   a built-in frame timer, and EVERY_N_MILLIS_I is now a millis() period timer.

   The original function names are kept; each is now an object with an update() call.
   Usage (call every loop pass; optional last argument = ms between frames, default 50):
     CD77_Halloween_Colors.update(600);                          // new spooky color every 600 ms
     CD77_Chase_Halloween_fixed.update(300, 3);                  // wait ms, dots  (uses the spooky color)
     CD77_Chase_Halloween_random.update(300, 900, 3);            // wait1 ms, wait2 ms, dots (uses the spooky color)
     CD77_Chase_Halloween_fixed_color.update(CRGB::Purple, 700, 8); // color, wait ms, dots
*/
#include "Config.h"

// ---------------------------------------------------------------------------
// Random orange/red/yellow color generator
// ---------------------------------------------------------------------------
class CD77_Halloween_Colors_SM {
public:
  void reset() { _color = CRGB::Red; _last = millis(); }

  void update(uint16_t waitMs) {
    uint32_t now = millis();
    if (now - _last >= waitMs) {
      _last  = now;
      // skew toward red/orange/yellow for extra creepiness.
      // (No map() to BRIGHTNESS any more: FastLED.setBrightness() already scales.)
      _color = CRGB(random8(150, 255), random8(100), 0);
    }
  }

  CRGB color() const { return _color; }

private:
  CRGB     _color = CRGB::Red;
  uint32_t _last  = 0;
};

// ---------------------------------------------------------------------------
// Chase: pixels shift one step per frame; a burst of 'dots' lit pixels is
// injected at LED 0 every 'period' ms, black in between.
// ---------------------------------------------------------------------------
class CD77_Chase_SM {
public:
  void reset() { _started = false; }

  // Fixed period between bursts
  void update(CRGB color, uint16_t periodMs, uint8_t dots, uint16_t frameMs = 50) {
    run(color, false, periodMs, periodMs, dots, frameMs);
  }

  // Random period (minMs..maxMs) between bursts, re-rolled each burst
  void updateRandom(CRGB color, uint16_t minMs, uint16_t maxMs, uint8_t dots, uint16_t frameMs = 50) {
    run(color, true, minMs, maxMs, dots, frameMs);
  }

private:
  bool     _started     = false;
  uint32_t _lastFrame   = 0;
  uint32_t _periodStart = 0;
  uint16_t _period      = 500;   // first burst window, same 500 ms default as the original
  uint8_t  _dotsDone    = 0;

  void run(CRGB color, bool rnd, uint16_t a, uint16_t b, uint8_t dots, uint16_t frameMs) {
    uint32_t now = millis();
    if (!_started) {
      _started     = true;
      _lastFrame   = now - frameMs;   // draw first frame immediately
      _periodStart = now;
      _period      = 500;
      _dotsDone    = 0;
    }
    if (now - _lastFrame < frameMs) return;   // not time for the next frame yet
    _lastFrame = now;

    // shift pixels
    for (int i = NUM_LEDS - 1; i > 0; i--) leds[i] = leds[i - 1];

    // time to start a new burst?
    if (now - _periodStart >= _period) {
      _periodStart = now;
      _period      = (rnd && b > a) ? random16(a, b) : a;
      _dotsDone    = 0;
    }

    if (_dotsDone < dots) { leds[0] = color; _dotsDone++; }
    else                  { leds[0] = CRGB::Black; }

    FastLED.show();
  }
};

CD77_Halloween_Colors_SM CD77_Halloween_Colors;

// ---------------------------------------------------------------------------
// The three original chase functions, each with its own state machine
// ---------------------------------------------------------------------------
class CD77_Chase_Halloween_fixed_SM {            // CD77_Chase_Halloween_fixed(wait1, dots)
public:
  void reset() { _c.reset(); }
  void update(uint16_t wait1, uint8_t dots, uint16_t frameMs = 50) {
    _c.update(CD77_Halloween_Colors.color(), wait1, dots, frameMs);
  }
private:
  CD77_Chase_SM _c;
};

class CD77_Chase_Halloween_random_SM {           // CD77_Chase_Halloween_random(wait1, wait2, dots)
public:
  void reset() { _c.reset(); }
  void update(uint16_t wait1, uint16_t wait2, uint8_t dots, uint16_t frameMs = 50) {
    _c.updateRandom(CD77_Halloween_Colors.color(), wait1, wait2, dots, frameMs);
  }
private:
  CD77_Chase_SM _c;
};

class CD77_Chase_Halloween_fixed_color_SM {      // CD77_Chase_Halloween_fixed_color(color1, wait1, dots)
public:
  void reset() { _c.reset(); }
  void update(CRGB color1, uint16_t wait1, uint8_t dots, uint16_t frameMs = 50) {
    _c.update(color1, wait1, dots, frameMs);
  }
private:
  CD77_Chase_SM _c;
};

CD77_Chase_Halloween_fixed_SM       CD77_Chase_Halloween_fixed;
CD77_Chase_Halloween_random_SM      CD77_Chase_Halloween_random;
CD77_Chase_Halloween_fixed_color_SM CD77_Chase_Halloween_fixed_color;
