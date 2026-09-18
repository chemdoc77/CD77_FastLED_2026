#pragma once
#include "MatrixConfig.h"
#include "RowColFill.h" // for RowFillDirection / ColFillDirection

// =====================================================================
// CD77_RowGradientFill
//
// Same build-up behavior as CD77_RowFill, but instead of every row
// being the same fixed color, the color itself steps through a
// gradient (startColor -> endColor) as the rows build up. Each row is
// still a single solid color -- it's the progression from one row to
// the next that changes color, e.g. row 1 = pure Red, the middle rows
// blend toward blue, and the last row lit = pure Blue. Rows still
// accumulate as they light up, mirrored identically across all 4 jars.
//
//   TOP_DOWN  -> row 0 first, then 1, 2, ... down to row 7 (matrix top)
//   BOTTOM_UP -> row 7 first, then 6, 5, ... up to row 0 (matrix bottom)
//
// The gradient always runs startColor -> endColor in build ORDER (the
// first row lit is startColor, the last row lit is endColor),
// regardless of which physical direction (TOP_DOWN/BOTTOM_UP) that
// build order maps to.
//
// Usage:
//   CD77_RowGradientFill rowGradient(leds, jarMatrix);
//   // every loop() pass:
//   rowGradient.Update(CRGB::Red, CRGB::Blue, 500, TOP_DOWN, cycleNumber);
// =====================================================================

class CD77_RowGradientFill {
public:
  CD77_RowGradientFill(CRGB* ledArray, const uint8_t (*jarMatrixRef)[MATRIX_ROWS][MATRIX_COLS])
    : _leds(ledArray), _jarMatrix(jarMatrixRef) {}

  bool Update(CRGB startColor, CRGB endColor, unsigned long rowInterval,
              RowFillDirection direction, unsigned long runID) {
    unsigned long now = millis();

    if (_firstCall || startColor != _startColor || endColor != _endColor ||
        direction != _direction || runID != _runID) {
      _firstCall  = false;
      _startColor = startColor;
      _endColor   = endColor;
      _direction  = direction;
      _runID      = runID;
      _rowStep    = 0;
      _complete   = false;
      _lastUpdate = now;

      // Precompute one color per build step (index 0 = first row lit,
      // index 7 = last row lit), stepping from startColor to endColor.
      fill_gradient_RGB(_stepColors, MATRIX_ROWS, _startColor, _endColor);

      FastLED.clear();
      FastLED.show();
    }

    _interval = rowInterval;

    if (_complete) {
      return false;
    }

    if (now - _lastUpdate < _interval) {
      return false;
    }
    _lastUpdate = now;

    uint8_t row = (_direction == TOP_DOWN) ? _rowStep : (MATRIX_ROWS - 1 - _rowStep);
    CRGB stepColor = _stepColors[_rowStep];

    for (uint8_t jar = 0; jar < NUM_JARS; jar++) {
      for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        _leds[_jarMatrix[jar][row][col]] = stepColor;
      }
    }
    FastLED.show();

    _rowStep++;
    if (_rowStep >= MATRIX_ROWS) {
      _complete = true;
    }
    return true;
  }

  bool isComplete() const { return _complete; }
  bool isRunning()  const { return !_complete; }

private:
  CRGB*         _leds;
  const uint8_t (*_jarMatrix)[MATRIX_ROWS][MATRIX_COLS];

  CRGB              _startColor = CRGB::Black;
  CRGB              _endColor   = CRGB::Black;
  CRGB              _stepColors[MATRIX_ROWS];
  unsigned long     _interval   = 0;
  RowFillDirection  _direction  = TOP_DOWN;
  unsigned long     _runID      = 0;
  uint8_t           _rowStep    = 0;
  bool              _complete   = false;
  bool              _firstCall  = true;
  unsigned long     _lastUpdate = 0;
};

// =====================================================================
// CD77_ColGradientFill
//
// Same idea as CD77_RowGradientFill, but for vertical columns: each
// column is a single solid color, and that color steps through ONE
// continuous gradient (startColor -> endColor) across all 4 jars --
// jar 0 column 0 = startColor, jar 3 column 7 = endColor, blending
// smoothly the whole way through, not resetting/repeating per jar.
// Works through the 4 jars SEQUENTIALLY: jar 0's columns 0-7 fill
// first, then jar 1's, then jar 2's, then jar 3's. Already-filled jars
// stay lit as later jars build up.
//
//   LEFT_RIGHT -> within each jar, column 0 first, then 1, 2, ... out to column 7
//   RIGHT_LEFT -> within each jar, column 7 first, then 6, 5, ... back to column 0
//
// Usage:
//   CD77_ColGradientFill colGradient(leds, jarMatrix);
//   colGradient.Update(CRGB::Red, CRGB::Blue, 150, LEFT_RIGHT, cycleNumber);
// =====================================================================

class CD77_ColGradientFill {
public:
  CD77_ColGradientFill(CRGB* ledArray, const uint8_t (*jarMatrixRef)[MATRIX_ROWS][MATRIX_COLS])
    : _leds(ledArray), _jarMatrix(jarMatrixRef) {}

  bool Update(CRGB startColor, CRGB endColor, unsigned long colInterval,
              ColFillDirection direction, unsigned long runID) {
    unsigned long now = millis();

    if (_firstCall || startColor != _startColor || endColor != _endColor ||
        direction != _direction || runID != _runID) {
      _firstCall  = false;
      _startColor = startColor;
      _endColor   = endColor;
      _direction  = direction;
      _runID      = runID;
      _step       = 0;
      _complete   = false;
      _lastUpdate = now;

      // Precompute one color per step across the ENTIRE build (index 0
      // = jar 0's first column lit, index (NUM_JARS*MATRIX_COLS - 1) =
      // jar 3's last column lit), stepping once from startColor to
      // endColor across all 4 jars combined.
      fill_gradient_RGB(_stepColors, NUM_JARS * MATRIX_COLS, _startColor, _endColor);

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
    CRGB stepColor = _stepColors[_step];

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
      _leds[_jarMatrix[jar][row][col]] = stepColor;
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

  CRGB              _startColor = CRGB::Black;
  CRGB              _endColor   = CRGB::Black;
  CRGB              _stepColors[NUM_JARS * MATRIX_COLS];
  unsigned long     _interval   = 0;
  ColFillDirection  _direction  = LEFT_RIGHT;
  unsigned long     _runID      = 0;
  uint8_t           _step       = 0;
  bool              _complete   = false;
  bool              _firstCall  = true;
  unsigned long     _lastUpdate = 0;
};

// ---------- Effect instances (mirrored across all 4 jars) ----------
extern CRGB leds[NUM_LEDS];

CD77_RowGradientFill rowGradientFill(leds, jarMatrix);
CD77_ColGradientFill colGradientFill(leds, jarMatrix);
