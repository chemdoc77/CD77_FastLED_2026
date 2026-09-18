// ============================================================
//  basic_functions.h  (glass_jar_4jars edition)
//  Non-blocking FastLED state machine functions, adapted for
//  the 4-jar daisy-chained strip in glass_jar_4jars.cpp.
//
//  Contains three classes:
//    1. SolidColor   - fills a range of LEDs with one color
//    2. BlinkColor   - blinks a range of LEDs on/off with a
//                      color, an on-time (x ms) and an off-time (y ms)
//    3. ChaseOn      - turns LEDs on one at a time within a range,
//                      with an x millisecond delay between each new LED
//
//  None of these three classes know anything about rows, columns,
//  or jars -- each one just operates on whatever contiguous CRGB*
//  range (a pointer + a count) it's constructed with. That turns out
//  to be exactly what's needed for both "all jars" and "one jar"
//  versions on the 4-jar chain, with NO changes to the class logic
//  itself:
//
//    ALL 4 JARS AT ONCE -- construct with the full shared array:
//      SolidColor solidColorAllJars(leds, NUM_LEDS);      // NUM_LEDS = 256
//
//    ONE JAR ONLY -- construct with a pointer into that jar's own
//    contiguous 64-LED slice (jar 0 = leds[0..63], jar 1 = leds[64..127],
//    etc.), same pattern used for the other *PerJar effects in this sketch:
//      SolidColor solidColorJar[NUM_JARS] = {
//        SolidColor(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
//        SolidColor(&leds[1 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
//        SolidColor(&leds[2 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
//        SolidColor(&leds[3 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS)
//      };
//
//  Same pattern for BlinkColor and ChaseOn -- see glass_jar_4jars.cpp
//  for the actual declarations used in this sketch.
//
//  NOTE on ChaseOn specifically: it walks LEDs by physical INDEX within
//  whatever range it's given, not by row/column. Given all 256 LEDs
//  (the "all jars" case), the chase sweeps through jar 0's LEDs, then
//  jar 1's, then jar 2's, then jar 3's, end to end, as one continuous
//  256-step chase -- not four synchronized per-jar chases. Given one
//  jar's 64-LED slice (the "one jar" case), it chases across just that
//  jar, same as the original single-strip version.
//
//  Pattern (from prior chats): all state lives inside the class.
//  Nothing except the call to .update(...) and its arguments belongs
//  in the .ino/.cpp file's loop().
// ============================================================

#pragma once
#include <FastLED.h>
#include "MatrixConfig.h" // for NUM_LEDS, NUM_JARS, MATRIX_ROWS, MATRIX_COLS

// ─────────────────────────────────────────────────────────────
//  1. SolidColor
//     Fills every LED in the given range with a single CRGB color.
//     Usage:
//       SolidColor solid(leds, NUM_LEDS);
//       solid.update(CRGB::Red);   // called every loop()
// ─────────────────────────────────────────────────────────────
class SolidColor {
  CRGB* _leds;
  int   _numLeds;

  CRGB  _lastColor;
  bool  _firstRun;

public:
  SolidColor(CRGB* leds, int numLeds)
    : _leds(leds), _numLeds(numLeds), _lastColor(CRGB::Black), _firstRun(true) {}

  // color - the CRGB::color to fill this range with
  void update(CRGB color) {
    // Only rewrite this range when the requested color actually changes,
    // so this stays cheap to call every loop() pass.
    if (_firstRun || color != _lastColor) {
      fill_solid(_leds, _numLeds, color);
      FastLED.show();
      _lastColor = color;
      _firstRun  = false;
    }
  }

  // Call from RestartPerformance() so this effect re-initializes
  // itself from scratch the next time update() is called.
  void reset() {
    _firstRun = true;
  }
};


// ─────────────────────────────────────────────────────────────
//  2. BlinkColor
//     Turns this range of LEDs on to a color for x ms, then off
//     for y ms, then repeats - fully non-blocking (millis based).
//     Usage:
//       BlinkColor blink(leds, NUM_LEDS);
//       blink.update(CRGB::Blue, 500, 250); // called every loop()
// ─────────────────────────────────────────────────────────────
class BlinkColor {
  CRGB* _leds;
  int   _numLeds;

  bool          _isOn;
  unsigned long _lastChange;
  bool          _firstRun;

public:
  BlinkColor(CRGB* leds, int numLeds)
    : _leds(leds), _numLeds(numLeds), _isOn(false), _lastChange(0), _firstRun(true) {}

  // color  - the CRGB::color to show while "on"
  // xOnMs  - how many milliseconds this range stays on
  // yOffMs - how many milliseconds this range stays off
  void update(CRGB color, unsigned long xOnMs, unsigned long yOffMs) {
    unsigned long now = millis();

    if (_firstRun) {
      // Kick off in the ON state the first time update() is called
      fill_solid(_leds, _numLeds, color);
      FastLED.show();
      _isOn       = true;
      _lastChange = now;
      _firstRun   = false;
      return;
    }

    if (_isOn && (now - _lastChange >= xOnMs)) {
      // Time to turn off
      fill_solid(_leds, _numLeds, CRGB::Black);
      FastLED.show();
      _isOn       = false;
      _lastChange = now;
    } else if (!_isOn && (now - _lastChange >= yOffMs)) {
      // Time to turn back on
      fill_solid(_leds, _numLeds, color);
      FastLED.show();
      _isOn       = true;
      _lastChange = now;
    }
  }

  // Call from RestartPerformance() so this effect re-initializes
  // itself from scratch (back to the ON state) the next time
  // update() is called.
  void reset() {
    _firstRun = true;
  }
};


// ─────────────────────────────────────────────────────────────
//  3. ChaseOn
//     Turns LEDs on one at a time (index 0, 1, 2, ... within
//     whatever range this instance was given) with an x
//     millisecond delay before the next LED is lit. Once the
//     last LED in range has been turned on, it waits one more
//     interval and then restarts from a cleared range.
//     Usage:
//       ChaseOn chase(leds, NUM_LEDS);
//       chase.update(CRGB::Green, 100); // called every loop()
// ─────────────────────────────────────────────────────────────
class ChaseOn {
  CRGB* _leds;
  int   _numLeds;

  int           _currentLed;
  unsigned long _lastStep;
  bool          _firstRun;

public:
  ChaseOn(CRGB* leds, int numLeds)
    : _leds(leds), _numLeds(numLeds), _currentLed(0), _lastStep(0), _firstRun(true) {}

  // color   - the CRGB::color each LED turns on to
  // xStepMs - milliseconds to wait before turning on the next LED
  void update(CRGB color, unsigned long xStepMs) {
    unsigned long now = millis();

    if (_firstRun) {
      // Start from a cleared range and light LED 0 (of this range) immediately
      fill_solid(_leds, _numLeds, CRGB::Black);
      _leds[0] = color;
      FastLED.show();
      _currentLed = 0;
      _lastStep   = now;
      _firstRun   = false;
      return;
    }

    if (now - _lastStep >= xStepMs) {
      _lastStep = now;

      if (_currentLed < _numLeds - 1) {
        // Light the next LED
        _currentLed++;
        _leds[_currentLed] = color;
        FastLED.show();
      } else {
        // Reached the end of this range - pause a beat, then restart the chase
        fill_solid(_leds, _numLeds, CRGB::Black);
        _leds[0] = color;
        FastLED.show();
        _currentLed = 0;
      }
    }
  }

  // Call from RestartPerformance() so this effect re-initializes
  // itself from a cleared range, starting again at index 0 (of
  // this range), the next time update() is called.
  void reset() {
    _firstRun = true;
  }
};

// ---------- Effect instances ----------
// `leds` is defined in glass_jar_4jars.cpp. None of these three classes
// know about rows/columns/jars, so "all jars" and "one jar" versions
// need no special class variants -- just different constructor args.
extern CRGB leds[NUM_LEDS];

// All jars at once: fills/blinks/chases across the full 256-LED strip.
SolidColor solidColorAllJars(leds, NUM_LEDS);
BlinkColor blinkColorAllJars(leds, NUM_LEDS);
ChaseOn    chaseOnAllJars(leds, NUM_LEDS);

// One jar only: index 0-3 controls jar 0-3 independently. Each jar is a
// contiguous MATRIX_ROWS*MATRIX_COLS-LED block within the shared leds[]
// array, so &leds[jar * MATRIX_ROWS * MATRIX_COLS] points straight at
// that jar's own LEDs.
SolidColor solidColorJar[NUM_JARS] = {
  SolidColor(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  SolidColor(&leds[1 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  SolidColor(&leds[2 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  SolidColor(&leds[3 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS)
};
BlinkColor blinkColorJar[NUM_JARS] = {
  BlinkColor(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  BlinkColor(&leds[1 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  BlinkColor(&leds[2 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  BlinkColor(&leds[3 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS)
};
ChaseOn chaseOnJar[NUM_JARS] = {
  ChaseOn(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  ChaseOn(&leds[1 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  ChaseOn(&leds[2 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  ChaseOn(&leds[3 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS)
};
