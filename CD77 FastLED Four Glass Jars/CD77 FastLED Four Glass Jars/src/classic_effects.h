// ============================================================
//  classic_effects.h  (glass_jar_4jars edition)
//  Three classic, non-blocking FastLED effects, in the same style
//  as basic_functions.h -- each operates on whatever contiguous
//  CRGB* range (a pointer + a count) it's constructed with, so
//  "all jars" and "one jar" versions need no special class variants,
//  just different constructor arguments:
//
//    ALL 4 JARS AT ONCE:  Twinkle twinkleAllJars(leds, NUM_LEDS);
//    ONE JAR ONLY:        Twinkle twinkleJar[NUM_JARS] = {
//                            Twinkle(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
//                            ...
//                          };
//
//  Contains three classes:
//    1. Twinkle       - random LEDs spark to a color and fade out,
//                       like fireflies/stars
//    2. TheaterChase  - classic marquee: every Nth LED lit, the
//                       whole pattern shifts by one LED each step
//    3. CometBounce   - a bright "head" with a fading tail, bouncing
//                       back and forth across the range (KITT-scanner
//                       style)
//
//  None of these take a runID -- like basic_functions.h, they use
//  their own reset() method instead (called from RestartPerformance())
//  to force a clean restart each lap.
// ============================================================

#pragma once
#include <FastLED.h>
#include "MatrixConfig.h" // for NUM_LEDS, NUM_JARS, MATRIX_ROWS, MATRIX_COLS

// ─────────────────────────────────────────────────────────────
//  1. Twinkle
//     Every tick, all LEDs in range fade toward black by fadeAmount,
//     then a new twinkle spawns at a random position with probability
//     spawnChance/255. Pass CRGB::Black as color for random-hue
//     twinkles instead of one fixed color.
//     Usage:
//       Twinkle twinkle(leds, NUM_LEDS);
//       twinkle.update(CRGB::White, 40, 60, 30); // called every loop()
// ─────────────────────────────────────────────────────────────
class Twinkle {
  CRGB* _leds;
  int   _numLeds;

  unsigned long _lastUpdate;

public:
  Twinkle(CRGB* leds, int numLeds)
    : _leds(leds), _numLeds(numLeds), _lastUpdate(0) {}

  // color            - twinkle color, or CRGB::Black for a random hue each spawn
  // fadeAmount        - 1-255, how much existing twinkles fade each tick
  //                      (higher = shorter-lived sparkles)
  // spawnChance       - 0-255, probability a new twinkle spawns each tick
  //                      (higher = busier/denser sparkle field)
  // updateIntervalMs  - non-blocking delay between animation ticks
  void update(CRGB color, uint8_t fadeAmount, uint8_t spawnChance, unsigned long updateIntervalMs) {
    unsigned long now = millis();
    if (now - _lastUpdate < updateIntervalMs) {
      return;
    }
    _lastUpdate = now;

    for (int i = 0; i < _numLeds; i++) {
      _leds[i].fadeToBlackBy(fadeAmount);
    }

    if (random8() < spawnChance) {
      int idx = random16(_numLeds);
      _leds[idx] = (color == CRGB::Black) ? CHSV(random8(), 200, 255) : color;
    }

    FastLED.show();
  }

  // Call from RestartPerformance() so this effect starts each lap
  // from a cleared range.
  void reset() {
    for (int i = 0; i < _numLeds; i++) {
      _leds[i] = CRGB::Black;
    }
    FastLED.show();
  }
};


// ─────────────────────────────────────────────────────────────
//  2. TheaterChase
//     Lights every Nth LED (spacing apart) in a solid color, and
//     shifts the whole pattern by one LED each step -- the classic
//     theater marquee chase.
//     Usage:
//       TheaterChase chase(leds, NUM_LEDS);
//       chase.update(CRGB::Blue, 3, 100); // called every loop()
// ─────────────────────────────────────────────────────────────
class TheaterChase {
  CRGB* _leds;
  int   _numLeds;

  uint8_t       _offset;
  unsigned long _lastStep;

public:
  TheaterChase(CRGB* leds, int numLeds)
    : _leds(leds), _numLeds(numLeds), _offset(0), _lastStep(0) {}

  // color          - chase color
  // spacing        - LEDs between each lit pixel (e.g. 3 = every 3rd LED lit)
  // stepIntervalMs - non-blocking delay between each shift step
  void update(CRGB color, uint8_t spacing, unsigned long stepIntervalMs) {
    unsigned long now = millis();
    if (now - _lastStep < stepIntervalMs) {
      return;
    }
    _lastStep = now;

    for (int i = 0; i < _numLeds; i++) {
      _leds[i] = (((i + _offset) % spacing) == 0) ? color : CRGB::Black;
    }
    FastLED.show();

    _offset = (_offset + 1) % spacing;
  }

  // Call from RestartPerformance() so the chase pattern starts each
  // lap from the same phase.
  void reset() {
    _offset = 0;
  }
};


// ─────────────────────────────────────────────────────────────
//  3. CometBounce
//     A bright "head" pixel moves along the range, leaving a fading
//     tail behind it, and reverses direction when it hits either end
//     -- like an old scanner/KITT effect.
//     Usage:
//       CometBounce comet(leds, NUM_LEDS);
//       comet.update(CRGB::Red, 60, 15); // called every loop()
// ─────────────────────────────────────────────────────────────
class CometBounce {
  CRGB* _leds;
  int   _numLeds;

  int           _position;
  bool          _forward;
  unsigned long _lastStep;

public:
  CometBounce(CRGB* leds, int numLeds)
    : _leds(leds), _numLeds(numLeds), _position(0), _forward(true), _lastStep(0) {}

  // color          - comet head/tail color
  // tailFade       - 1-255, how much the tail fades each tick
  //                   (higher = shorter tail)
  // stepIntervalMs - non-blocking delay between each position update (speed)
  void update(CRGB color, uint8_t tailFade, unsigned long stepIntervalMs) {
    unsigned long now = millis();
    if (now - _lastStep < stepIntervalMs) {
      return;
    }
    _lastStep = now;

    for (int i = 0; i < _numLeds; i++) {
      _leds[i].fadeToBlackBy(tailFade);
    }
    _leds[_position] = color;
    FastLED.show();

    if (_forward) {
      if (_position >= _numLeds - 1) {
        _forward = false;
        if (_position > 0) _position--;
      } else {
        _position++;
      }
    } else {
      if (_position <= 0) {
        _forward = true;
        if (_position < _numLeds - 1) _position++;
      } else {
        _position--;
      }
    }
  }

  // Call from RestartPerformance() so the comet starts each lap at
  // the beginning of its range, moving forward.
  void reset() {
    _position = 0;
    _forward  = true;
  }
};

// ---------- Effect instances ----------
// `leds` is defined in glass_jar_4jars.cpp. Same pattern as
// basic_functions.h: these classes don't know about jars at all, so
// "all jars" and "one jar" versions are just different constructor args.
extern CRGB leds[NUM_LEDS];

// All jars at once: operates across the full 256-LED strip.
Twinkle      twinkleAllJars(leds, NUM_LEDS);
TheaterChase theaterChaseAllJars(leds, NUM_LEDS);
CometBounce  cometBounceAllJars(leds, NUM_LEDS);

// One jar only: index 0-3 controls jar 0-3 independently.
Twinkle twinkleJar[NUM_JARS] = {
  Twinkle(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  Twinkle(&leds[1 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  Twinkle(&leds[2 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  Twinkle(&leds[3 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS)
};
TheaterChase theaterChaseJar[NUM_JARS] = {
  TheaterChase(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  TheaterChase(&leds[1 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  TheaterChase(&leds[2 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  TheaterChase(&leds[3 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS)
};
CometBounce cometBounceJar[NUM_JARS] = {
  CometBounce(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  CometBounce(&leds[1 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  CometBounce(&leds[2 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  CometBounce(&leds[3 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS)
};
