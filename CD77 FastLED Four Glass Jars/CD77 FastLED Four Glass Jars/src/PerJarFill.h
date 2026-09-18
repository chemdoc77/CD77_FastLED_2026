#pragma once
#include "MatrixConfig.h"
#include "RowColFill.h"   // for RowFillDirection / ColFillDirection (reused, not redefined)
#include "SquareFill.h"   // for SquareFillDirection (reused, not redefined)

// =====================================================================
// Per-jar effect classes
//
// These are the same effects as CD77_RowFill, CD77_ColFill, etc. in
// RowColFill.h / GradientFill.h / SquareFill.h, but instead of
// mirroring across all 4 jars at once, each instance controls exactly
// ONE jar -- so different jars can run different colors, directions,
// or timings independently and simultaneously. Construct one instance
// per jar you want to control (see the rowFillJar[]-style arrays in
// glass_jar_4jars.cpp), each pointed at that jar's own 2D slice of
// jarMatrix (jarMatrix[0], jarMatrix[1], etc.).
//
// Everything else -- the Update()-only single-call pattern, the runID
// restart trick, isRunning()/isComplete() -- works exactly the same as
// the mirrored versions.
//
// Usage:
//   CD77_RowFillPerJar rowFillJar[NUM_JARS] = {
//     CD77_RowFillPerJar(leds, jarMatrix[0]),
//     CD77_RowFillPerJar(leds, jarMatrix[1]),
//     CD77_RowFillPerJar(leds, jarMatrix[2]),
//     CD77_RowFillPerJar(leds, jarMatrix[3])
//   };
//   // every loop() pass:
//   rowFillJar[0].Update(CRGB::Blue, 500, TOP_DOWN, runID);  // jar 0, blue
//   rowFillJar[1].Update(CRGB::Red,  500, TOP_DOWN, runID);  // jar 1, red
// =====================================================================

// ---------- CD77_RowFillPerJar ----------
class CD77_RowFillPerJar {
public:
  CD77_RowFillPerJar(CRGB* ledArray, const uint8_t (*matrixRef)[MATRIX_COLS])
    : _leds(ledArray), _matrix(matrixRef) {}

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
      _clearJar();
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

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      _leds[_matrix[row][col]] = _color;
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
  void _clearJar() {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
      for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        _leds[_matrix[r][c]] = CRGB::Black;
      }
    }
    FastLED.show();
  }

  CRGB*         _leds;
  const uint8_t (*_matrix)[MATRIX_COLS];

  CRGB              _color      = CRGB::Black;
  unsigned long     _interval   = 0;
  RowFillDirection  _direction  = TOP_DOWN;
  unsigned long     _runID      = 0;
  uint8_t           _rowStep    = 0;
  bool              _complete   = false;
  bool              _firstCall  = true;
  unsigned long     _lastUpdate = 0;
};

// ---------- CD77_ColFillPerJar ----------
class CD77_ColFillPerJar {
public:
  CD77_ColFillPerJar(CRGB* ledArray, const uint8_t (*matrixRef)[MATRIX_COLS])
    : _leds(ledArray), _matrix(matrixRef) {}

  bool Update(CRGB color, unsigned long colInterval, ColFillDirection direction, unsigned long runID) {
    unsigned long now = millis();

    if (_firstCall || color != _color || direction != _direction || runID != _runID) {
      _firstCall  = false;
      _color      = color;
      _direction  = direction;
      _runID      = runID;
      _colStep    = 0;
      _complete   = false;
      _lastUpdate = now;
      _clearJar();
    }

    _interval = colInterval;

    if (_complete) {
      return false;
    }

    if (now - _lastUpdate < _interval) {
      return false;
    }
    _lastUpdate = now;

    uint8_t col = (_direction == LEFT_RIGHT) ? _colStep : (MATRIX_COLS - 1 - _colStep);

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
      _leds[_matrix[row][col]] = _color;
    }
    FastLED.show();

    _colStep++;
    if (_colStep >= MATRIX_COLS) {
      _complete = true;
    }
    return true;
  }

  bool isComplete() const { return _complete; }
  bool isRunning()  const { return !_complete; }

private:
  void _clearJar() {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
      for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        _leds[_matrix[r][c]] = CRGB::Black;
      }
    }
    FastLED.show();
  }

  CRGB*         _leds;
  const uint8_t (*_matrix)[MATRIX_COLS];

  CRGB              _color      = CRGB::Black;
  unsigned long     _interval   = 0;
  ColFillDirection  _direction  = LEFT_RIGHT;
  unsigned long     _runID      = 0;
  uint8_t           _colStep    = 0;
  bool              _complete   = false;
  bool              _firstCall  = true;
  unsigned long     _lastUpdate = 0;
};

// ---------- CD77_RowGradientFillPerJar ----------
class CD77_RowGradientFillPerJar {
public:
  CD77_RowGradientFillPerJar(CRGB* ledArray, const uint8_t (*matrixRef)[MATRIX_COLS])
    : _leds(ledArray), _matrix(matrixRef) {}

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

      fill_gradient_RGB(_stepColors, MATRIX_ROWS, _startColor, _endColor);
      _clearJar();
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

    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      _leds[_matrix[row][col]] = stepColor;
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
  void _clearJar() {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
      for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        _leds[_matrix[r][c]] = CRGB::Black;
      }
    }
    FastLED.show();
  }

  CRGB*         _leds;
  const uint8_t (*_matrix)[MATRIX_COLS];

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

// ---------- CD77_ColGradientFillPerJar ----------
class CD77_ColGradientFillPerJar {
public:
  CD77_ColGradientFillPerJar(CRGB* ledArray, const uint8_t (*matrixRef)[MATRIX_COLS])
    : _leds(ledArray), _matrix(matrixRef) {}

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
      _colStep    = 0;
      _complete   = false;
      _lastUpdate = now;

      fill_gradient_RGB(_stepColors, MATRIX_COLS, _startColor, _endColor);
      _clearJar();
    }

    _interval = colInterval;

    if (_complete) {
      return false;
    }

    if (now - _lastUpdate < _interval) {
      return false;
    }
    _lastUpdate = now;

    uint8_t col = (_direction == LEFT_RIGHT) ? _colStep : (MATRIX_COLS - 1 - _colStep);
    CRGB stepColor = _stepColors[_colStep];

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
      _leds[_matrix[row][col]] = stepColor;
    }
    FastLED.show();

    _colStep++;
    if (_colStep >= MATRIX_COLS) {
      _complete = true;
    }
    return true;
  }

  bool isComplete() const { return _complete; }
  bool isRunning()  const { return !_complete; }

private:
  void _clearJar() {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
      for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        _leds[_matrix[r][c]] = CRGB::Black;
      }
    }
    FastLED.show();
  }

  CRGB*         _leds;
  const uint8_t (*_matrix)[MATRIX_COLS];

  CRGB              _startColor = CRGB::Black;
  CRGB              _endColor   = CRGB::Black;
  CRGB              _stepColors[MATRIX_COLS];
  unsigned long     _interval   = 0;
  ColFillDirection  _direction  = LEFT_RIGHT;
  unsigned long     _runID      = 0;
  uint8_t           _colStep    = 0;
  bool              _complete   = false;
  bool              _firstCall  = true;
  unsigned long     _lastUpdate = 0;
};

// ---------- CD77_SquareFillPerJar ----------
class CD77_SquareFillPerJar {
public:
  static const uint8_t NUM_RINGS = MATRIX_ROWS / 2;

  CD77_SquareFillPerJar(CRGB* ledArray, const uint8_t (*matrixRef)[MATRIX_COLS])
    : _leds(ledArray), _matrix(matrixRef) {}

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
      _clearJar();
    }

    _interval = ringInterval;

    if (_complete) {
      return false;
    }

    if (now - _lastUpdate < _interval) {
      return false;
    }
    _lastUpdate = now;

    uint8_t targetRing = (_direction == CENTER_TO_OUTSIDE) ? (NUM_RINGS - 1 - _step) : _step;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
      for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        if (ringOf(row, col) == targetRing) {
          _leds[_matrix[row][col]] = _color;
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

  void _clearJar() {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
      for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        _leds[_matrix[r][c]] = CRGB::Black;
      }
    }
    FastLED.show();
  }

  CRGB*         _leds;
  const uint8_t (*_matrix)[MATRIX_COLS];

  CRGB                 _color      = CRGB::Black;
  unsigned long        _interval   = 0;
  SquareFillDirection  _direction  = CENTER_TO_OUTSIDE;
  unsigned long        _runID      = 0;
  uint8_t              _step       = 0;
  bool                 _complete   = false;
  bool                 _firstCall  = true;
  unsigned long        _lastUpdate = 0;
};

// ---------- CD77_SquareFillMultiColorPerJar ----------
class CD77_SquareFillMultiColorPerJar {
public:
  static const uint8_t NUM_RINGS = MATRIX_ROWS / 2;

  CD77_SquareFillMultiColorPerJar(CRGB* ledArray, const uint8_t (*matrixRef)[MATRIX_COLS])
    : _leds(ledArray), _matrix(matrixRef) {}

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
      _clearJar();
    }

    _interval = ringInterval;

    if (_complete) {
      return false;
    }

    if (now - _lastUpdate < _interval) {
      return false;
    }
    _lastUpdate = now;

    uint8_t targetRing = (_direction == CENTER_TO_OUTSIDE) ? (NUM_RINGS - 1 - _step) : _step;
    CRGB stepColor = _ringColors[targetRing];

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
      for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        if (ringOf(row, col) == targetRing) {
          _leds[_matrix[row][col]] = stepColor;
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

  void _clearJar() {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
      for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        _leds[_matrix[r][c]] = CRGB::Black;
      }
    }
    FastLED.show();
  }

  CRGB*         _leds;
  const uint8_t (*_matrix)[MATRIX_COLS];

  CRGB                 _ringColors[NUM_RINGS];
  unsigned long        _interval   = 0;
  SquareFillDirection  _direction  = CENTER_TO_OUTSIDE;
  unsigned long        _runID      = 0;
  uint8_t              _step       = 0;
  bool                 _complete   = false;
  bool                 _firstCall  = true;
  unsigned long        _lastUpdate = 0;
};

// ---------- Effect instances (independent per jar) ----------
// `leds` is defined in glass_jar_4jars.cpp; declared extern here so this
// header is self-contained regardless of #include order. Index 0-3
// controls jar 0-3 independently -- e.g.
//   rowFillJar[0].Update(CRGB::Blue, 500, TOP_DOWN, runID);  // jar 0, blue
//   rowFillJar[1].Update(CRGB::Red,  500, TOP_DOWN, runID);  // jar 1, red
extern CRGB leds[NUM_LEDS];

CD77_RowFillPerJar rowFillJar[NUM_JARS] = {
  CD77_RowFillPerJar(leds, jarMatrix[0]),
  CD77_RowFillPerJar(leds, jarMatrix[1]),
  CD77_RowFillPerJar(leds, jarMatrix[2]),
  CD77_RowFillPerJar(leds, jarMatrix[3])
};
CD77_ColFillPerJar colFillJar[NUM_JARS] = {
  CD77_ColFillPerJar(leds, jarMatrix[0]),
  CD77_ColFillPerJar(leds, jarMatrix[1]),
  CD77_ColFillPerJar(leds, jarMatrix[2]),
  CD77_ColFillPerJar(leds, jarMatrix[3])
};
CD77_RowGradientFillPerJar rowGradientFillJar[NUM_JARS] = {
  CD77_RowGradientFillPerJar(leds, jarMatrix[0]),
  CD77_RowGradientFillPerJar(leds, jarMatrix[1]),
  CD77_RowGradientFillPerJar(leds, jarMatrix[2]),
  CD77_RowGradientFillPerJar(leds, jarMatrix[3])
};
CD77_ColGradientFillPerJar colGradientFillJar[NUM_JARS] = {
  CD77_ColGradientFillPerJar(leds, jarMatrix[0]),
  CD77_ColGradientFillPerJar(leds, jarMatrix[1]),
  CD77_ColGradientFillPerJar(leds, jarMatrix[2]),
  CD77_ColGradientFillPerJar(leds, jarMatrix[3])
};
CD77_SquareFillPerJar squareFillJar[NUM_JARS] = {
  CD77_SquareFillPerJar(leds, jarMatrix[0]),
  CD77_SquareFillPerJar(leds, jarMatrix[1]),
  CD77_SquareFillPerJar(leds, jarMatrix[2]),
  CD77_SquareFillPerJar(leds, jarMatrix[3])
};
CD77_SquareFillMultiColorPerJar squareFillMultiJar[NUM_JARS] = {
  CD77_SquareFillMultiColorPerJar(leds, jarMatrix[0]),
  CD77_SquareFillMultiColorPerJar(leds, jarMatrix[1]),
  CD77_SquareFillMultiColorPerJar(leds, jarMatrix[2]),
  CD77_SquareFillMultiColorPerJar(leds, jarMatrix[3])
};
