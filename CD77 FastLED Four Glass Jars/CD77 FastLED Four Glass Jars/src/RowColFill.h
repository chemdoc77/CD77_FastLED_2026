#pragma once
#include "MatrixConfig.h"

enum RowFillDirection {
  TOP_DOWN,
  BOTTOM_UP
};

enum ColFillDirection {
  LEFT_RIGHT,
  RIGHT_LEFT
};

// =====================================================================
// CD77_RowFill
//
// Non-blocking row build-up state machine, single-call version -- only
// Update() needs to be called from loop(). The class watches its own
// arguments internally and restarts itself whenever the color,
// direction, or runID changes, so there's no separate Start() call to
// wire up.
//
// On the 4-jar chain, each step lights the same row on ALL 4 jars at
// once, so all four jars build up in sync, mirroring each other.
//
//   TOP_DOWN  -> row 0 first, then 1, 2, ... down to row 7 (matrix top)
//   BOTTOM_UP -> row 7 first, then 6, 5, ... up to row 0 (matrix bottom)
//
// runID lets the effect restart even when called again later with the
// SAME color/direction (e.g. the next lap of a repeating cycle) --
// pass any value that changes each time this segment should begin
// (a cycle counter, a millis() timestamp, etc.).
//
// Usage:
//   CD77_RowFill rowFill(leds, jarMatrix);
//   // every loop() pass:
//   rowFill.Update(CRGB::Blue, 500, TOP_DOWN, cycleNumber);
// =====================================================================

class CD77_RowFill {
public:
  CD77_RowFill(CRGB* ledArray, const uint8_t (*jarMatrixRef)[MATRIX_ROWS][MATRIX_COLS])
    : _leds(ledArray), _jarMatrix(jarMatrixRef) {}

  // Call every loop() pass. Internally detects a new color, direction,
  // or runID and restarts the build-up automatically; otherwise just
  // advances the existing build-up on its own timer.
  bool Update(CRGB color, unsigned long rowInterval, RowFillDirection direction, unsigned long runID) {
    unsigned long now = millis();

    if (_firstCall || color != _color || direction != _direction || runID != _runID) {
      _firstCall  = false;
      _color      = color;
      _direction  = direction;
      _runID      = runID;
      _rowStep    = 0;
      _complete   = false;
      _lastUpdate = now;

      FastLED.clear();
      FastLED.show();
    }

    _interval = rowInterval; // safe to update every call, doesn't affect timing already in progress

    if (_complete) {
      return false; // holds fully lit until color/direction/runID changes
    }

    if (now - _lastUpdate < _interval) {
      return false; // not time for the next row yet
    }
    _lastUpdate = now;

    uint8_t row = (_direction == TOP_DOWN) ? _rowStep : (MATRIX_ROWS - 1 - _rowStep);

    for (uint8_t jar = 0; jar < NUM_JARS; jar++) {
      for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        _leds[_jarMatrix[jar][row][col]] = _color;
      }
    }
    FastLED.show();

    _rowStep++;
    if (_rowStep >= MATRIX_ROWS) {
      _complete = true; // all 8 rows lit on every jar, hold until color/direction/runID changes
    }
    return true;
  }

  bool isComplete() const { return _complete; }
  bool isRunning()  const { return !_complete; }

private:
  CRGB*         _leds;
  const uint8_t (*_jarMatrix)[MATRIX_ROWS][MATRIX_COLS];

  CRGB              _color      = CRGB::Black;
  unsigned long     _interval   = 0;
  RowFillDirection  _direction  = TOP_DOWN;
  unsigned long     _runID      = 0;
  uint8_t           _rowStep    = 0;
  bool              _complete   = false;
  bool              _firstCall  = true;
  unsigned long     _lastUpdate = 0;
};

// =====================================================================
// CD77_ColFill
//
// Same single-call pattern as CD77_RowFill, but builds up vertical
// columns instead of horizontal rows -- and works through the 4 jars
// SEQUENTIALLY rather than mirroring: jar 0's columns 0-7 fill
// completely first, then jar 1's columns 0-7, then jar 2's, then
// jar 3's. Already-filled jars stay lit as later jars build up.
//
//   LEFT_RIGHT -> within each jar, column 0 first, then 1, 2, ... out to column 7
//   RIGHT_LEFT -> within each jar, column 7 first, then 6, 5, ... back to column 0
//
// Usage:
//   CD77_ColFill colFill(leds, jarMatrix);
//   colFill.Update(CRGB::Red, 150, LEFT_RIGHT, cycleNumber);
// =====================================================================

class CD77_ColFill {
public:
  CD77_ColFill(CRGB* ledArray, const uint8_t (*jarMatrixRef)[MATRIX_ROWS][MATRIX_COLS])
    : _leds(ledArray), _jarMatrix(jarMatrixRef) {}

  bool Update(CRGB color, unsigned long colInterval, ColFillDirection direction, unsigned long runID) {
    unsigned long now = millis();

    if (_firstCall || color != _color || direction != _direction || runID != _runID) {
      _firstCall  = false;
      _color      = color;
      _direction  = direction;
      _runID      = runID;
      _step       = 0;
      _complete   = false;
      _lastUpdate = now;

      FastLED.clear();
      FastLED.show();
    }

    _interval = colInterval;

    if (_complete) {
      return false;
    }

    if (now - _lastUpdate < _interval) {
      return false;
    }
    _lastUpdate = now;

    // _step counts 0..(NUM_JARS*MATRIX_COLS - 1): every MATRIX_COLS
    // steps moves on to the next jar, so jar 0's columns finish before
    // jar 1's columns begin, etc.
    uint8_t jar          = _step / MATRIX_COLS;
    uint8_t colWithinJar = _step % MATRIX_COLS;
    uint8_t col = (_direction == LEFT_RIGHT) ? colWithinJar : (MATRIX_COLS - 1 - colWithinJar);

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
      _leds[_jarMatrix[jar][row][col]] = _color;
    }
    FastLED.show();

    _step++;
    if (_step >= (uint16_t)(NUM_JARS * MATRIX_COLS)) {
      _complete = true;
    }
    return true;
  }

  bool isComplete() const { return _complete; }
  bool isRunning()  const { return !_complete; }

private:
  CRGB*         _leds;
  const uint8_t (*_jarMatrix)[MATRIX_ROWS][MATRIX_COLS];

  CRGB              _color      = CRGB::Black;
  unsigned long     _interval   = 0;
  ColFillDirection  _direction  = LEFT_RIGHT;
  unsigned long     _runID      = 0;
  uint8_t           _step       = 0;
  bool              _complete   = false;
  bool              _firstCall  = true;
  unsigned long     _lastUpdate = 0;
};

// ---------- Effect instances (mirrored across all 4 jars) ----------
// `leds` is defined in glass_jar_4jars.cpp; declared extern here so this
// header is self-contained regardless of #include order.
extern CRGB leds[NUM_LEDS];

CD77_RowFill rowFill(leds, jarMatrix);
CD77_ColFill colFill(leds, jarMatrix);
