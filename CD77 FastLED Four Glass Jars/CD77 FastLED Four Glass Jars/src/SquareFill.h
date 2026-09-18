#pragma once
#include "MatrixConfig.h"

enum SquareFillDirection {
  CENTER_TO_OUTSIDE,
  OUTSIDE_TO_CENTER
};

// =====================================================================
// CD77_SquareFill
//
// Lights up each jar's matrix in concentric square "rings," either
// building from the center out to the border, or from the border in
// to the center. For an 8x8 grid there are 4 rings:
//
//   ring 0 = outer border      (28 LEDs)
//   ring 1 = next layer in     (20 LEDs)
//   ring 2 = next layer in     (12 LEDs)
//   ring 3 = innermost 2x2 core (4 LEDs)
//
// A cell's ring is computed as min(row, col, 7-row, 7-col) -- 0 at the
// border, increasing toward the center. Each ring accumulates (stays
// lit) as the effect progresses, mirrored identically across all 4 jars.
//
//   CENTER_TO_OUTSIDE -> ring 3 first, then 2, 1, and finally ring 0
//   OUTSIDE_TO_CENTER -> ring 0 first, then 1, 2, and finally ring 3
//
// Usage:
//   CD77_SquareFill squareFill(leds, jarMatrix);
//   // every loop() pass:
//   squareFill.Update(CRGB::Purple, 500, CENTER_TO_OUTSIDE, cycleNumber);
// =====================================================================

class CD77_SquareFill {
public:
  static const uint8_t NUM_RINGS = MATRIX_ROWS / 2; // 4 rings for an 8x8 grid

  CD77_SquareFill(CRGB* ledArray, const uint8_t (*jarMatrixRef)[MATRIX_ROWS][MATRIX_COLS])
    : _leds(ledArray), _jarMatrix(jarMatrixRef) {}

  bool Update(CRGB color, unsigned long ringInterval, SquareFillDirection direction, unsigned long runID) {
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

    _interval = ringInterval;

    if (_complete) {
      return false;
    }

    if (now - _lastUpdate < _interval) {
      return false;
    }
    _lastUpdate = now;

    // step 0 is always "first ring lit" -- map it to ring 3 (center) or
    // ring 0 (border) depending on direction.
    uint8_t targetRing = (_direction == CENTER_TO_OUTSIDE) ? (NUM_RINGS - 1 - _step) : _step;

    for (uint8_t jar = 0; jar < NUM_JARS; jar++) {
      for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
          if (ringOf(row, col) == targetRing) {
            _leds[_jarMatrix[jar][row][col]] = _color;
          }
        }
      }
    }
    FastLED.show();

    _step++;
    if (_step >= NUM_RINGS) {
      _complete = true;
    }
    return true;
  }

  bool isComplete() const { return _complete; }
  bool isRunning()  const { return !_complete; }

private:
  static uint8_t ringOf(uint8_t row, uint8_t col) {
    uint8_t a = row;
    uint8_t b = col;
    uint8_t c = (MATRIX_ROWS - 1) - row;
    uint8_t d = (MATRIX_COLS - 1) - col;
    uint8_t m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    if (d < m) m = d;
    return m;
  }

  CRGB*         _leds;
  const uint8_t (*_jarMatrix)[MATRIX_ROWS][MATRIX_COLS];

  CRGB                 _color      = CRGB::Black;
  unsigned long        _interval   = 0;
  SquareFillDirection  _direction  = CENTER_TO_OUTSIDE;
  unsigned long        _runID      = 0;
  uint8_t              _step       = 0;
  bool                 _complete   = false;
  bool                 _firstCall  = true;
  unsigned long        _lastUpdate = 0;
};

// =====================================================================
// CD77_SquareFillMultiColor
//
// Same ring-based build-up as CD77_SquareFill, but each of the 4 rings
// gets its own independent color instead of one shared color:
//
//   ringColors[0] = outer border ring color
//   ringColors[1] = next ring in
//   ringColors[2] = next ring in
//   ringColors[3] = innermost 2x2 core ring color
//
//   CENTER_TO_OUTSIDE -> ring 3 lit first (its own color), then 2, 1,
//                         finally ring 0 -- each ring keeps its assigned
//                         color as the others build up around/inside it
//   OUTSIDE_TO_CENTER -> ring 0 first, then 1, 2, finally ring 3
//
// Mirrored identically across all 4 jars.
//
// Usage:
//   CD77_SquareFillMultiColor squareFillMulti(leds, jarMatrix);
//   CRGB ringColors[4] = { CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green };
//   // every loop() pass:
//   squareFillMulti.Update(ringColors, 400, CENTER_TO_OUTSIDE, cycleNumber);
// =====================================================================

class CD77_SquareFillMultiColor {
public:
  static const uint8_t NUM_RINGS = MATRIX_ROWS / 2; // 4 rings for an 8x8 grid

  CD77_SquareFillMultiColor(CRGB* ledArray, const uint8_t (*jarMatrixRef)[MATRIX_ROWS][MATRIX_COLS])
    : _leds(ledArray), _jarMatrix(jarMatrixRef) {}

  // ringColors must point to an array of NUM_RINGS (4) colors, indexed
  // outer (0) to center (NUM_RINGS-1).
  bool Update(const CRGB ringColors[NUM_RINGS], unsigned long ringInterval,
              SquareFillDirection direction, unsigned long runID) {
    unsigned long now = millis();

    bool colorsChanged = _firstCall;
    if (!colorsChanged) {
      for (uint8_t i = 0; i < NUM_RINGS; i++) {
        if (ringColors[i] != _ringColors[i]) {
          colorsChanged = true;
          break;
        }
      }
    }

    if (colorsChanged || direction != _direction || runID != _runID) {
      _firstCall = false;
      for (uint8_t i = 0; i < NUM_RINGS; i++) {
        _ringColors[i] = ringColors[i];
      }
      _direction  = direction;
      _runID      = runID;
      _step       = 0;
      _complete   = false;
      _lastUpdate = now;

      FastLED.clear();
      FastLED.show();
    }

    _interval = ringInterval;

    if (_complete) {
      return false;
    }

    if (now - _lastUpdate < _interval) {
      return false;
    }
    _lastUpdate = now;

    // step 0 is always "first ring lit" -- map it to ring 3 (center) or
    // ring 0 (border) depending on direction, same as CD77_SquareFill.
    uint8_t targetRing = (_direction == CENTER_TO_OUTSIDE) ? (NUM_RINGS - 1 - _step) : _step;
    CRGB stepColor = _ringColors[targetRing];

    for (uint8_t jar = 0; jar < NUM_JARS; jar++) {
      for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
          if (ringOf(row, col) == targetRing) {
            _leds[_jarMatrix[jar][row][col]] = stepColor;
          }
        }
      }
    }
    FastLED.show();

    _step++;
    if (_step >= NUM_RINGS) {
      _complete = true;
    }
    return true;
  }

  bool isComplete() const { return _complete; }
  bool isRunning()  const { return !_complete; }

private:
  static uint8_t ringOf(uint8_t row, uint8_t col) {
    uint8_t a = row;
    uint8_t b = col;
    uint8_t c = (MATRIX_ROWS - 1) - row;
    uint8_t d = (MATRIX_COLS - 1) - col;
    uint8_t m = a;
    if (b < m) m = b;
    if (c < m) m = c;
    if (d < m) m = d;
    return m;
  }

  CRGB*         _leds;
  const uint8_t (*_jarMatrix)[MATRIX_ROWS][MATRIX_COLS];

  CRGB                 _ringColors[NUM_RINGS];
  unsigned long        _interval   = 0;
  SquareFillDirection  _direction  = CENTER_TO_OUTSIDE;
  unsigned long        _runID      = 0;
  uint8_t              _step       = 0;
  bool                 _complete   = false;
  bool                 _firstCall  = true;
  unsigned long        _lastUpdate = 0;
};

// ---------- Effect instances (mirrored across all 4 jars) ----------
extern CRGB leds[NUM_LEDS];

CD77_SquareFill squareFill(leds, jarMatrix);
CD77_SquareFillMultiColor squareFillMulti(leds, jarMatrix);
