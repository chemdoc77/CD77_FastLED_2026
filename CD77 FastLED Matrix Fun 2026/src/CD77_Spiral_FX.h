// CD77_Spiral_FX.h
// Rotating Archimedean spiral effects for any W x H WS2812b matrix.
//
// Two classes:
//   Spiral   -- full-matrix rotating spiral
//   SpiralQ  -- rotating spiral in one quad (QUAD_WIDTH x QUAD_HEIGHT)
//
// There are always exactly 4 quads:
//   QUAD_WIDTH  = MATRIX_WIDTH  / 2
//   QUAD_HEIGHT = MATRIX_HEIGHT / 2
//
// For quad spirals, use the free function updateMatrix_4quad_Spiral().
// Quad addressing: (qCol, qRow), qCol in {1,2}, qRow in {1,2}, (1,1) = top-left.
//
// TIGHTNESS is computed adaptively by sp_init() so the spiral appearance
// is consistent across any matrix size.  The reference is the 16x16 matrix
// (tightness=0.72, ~1.22 full rotations centre-to-corner).
//
// Example:
//   FROM(0,0,0) {
//     updateMatrix_4quad_Spiral(1, 1, CRGB::Red,  CRGB::Black, 1500, 0.60f, true);
//     updateMatrix_4quad_Spiral(2, 1, CRGB::Blue, CRGB::Black, 1500, 0.60f, false);
//     updateMatrix_4quad_Spiral(1, 2, CRGB::Blue, CRGB::Black, 1500, 0.60f, false);
//     updateMatrix_4quad_Spiral(2, 2, CRGB::Red,  CRGB::Black, 1500, 0.60f, true);
//     FastLED.show();
//   }

#pragma once
#include "CD77_Matrix_loop_functions.h"
#include <math.h>

#define SPIRAL_ARMS        2
#define SPIRAL_FRAME_MS    16
#define SQ_ARMS            2
#define SQ_FRAME_MS        16

// Tightness globals -- set by sp_init() for the configured matrix size.
// Defaults are for a 16x16 matrix; sp_init() overwrites them.
static float _spiralTightness = 0.72f;  // full-matrix spiral
static float _sqTightness     = 1.54f;  // per-quad spiral (default = 8x8 value)

// CLASS: Spiral -- rotating spiral across the full matrix
class Spiral {
private:
  float    _phase;
  uint32_t _lastFrameMs;

  void draw(CRGB armColor, CRGB bgColor, float armThickness) {
    const float cx = (MATRIX_WIDTH  - 1) * 0.5f;
    const float cy = (MATRIX_HEIGHT - 1) * 0.5f;
    const float rMax = sqrtf(cx * cx + cy * cy);
    const float armSpacing = 2.0f * PI / (float)SPIRAL_ARMS;
    float safeFrac = constrain(armThickness, 0.05f, 0.90f);
    const float halfArm = armSpacing * safeFrac * 0.5f;
    for (uint16_t row = 0; row < MATRIX_HEIGHT; row++) {
      for (uint16_t col = 0; col < MATRIX_WIDTH; col++) {
        float dx = (float)col - cx, dy = (float)row - cy;
        float r = sqrtf(dx * dx + dy * dy);
        if (r < 0.5f) { leds[SP_LED(row, col)] = armColor; continue; }
        float theta = atan2f(dy, dx);
        float spiralAngle = fmodf(theta - _spiralTightness * r - _phase, 2.0f * PI);
        if (spiralAngle < 0.0f) spiralAngle += 2.0f * PI;
        float sectorAngle = fmodf(spiralAngle, armSpacing);
        if (sectorAngle > armSpacing * 0.5f) sectorAngle -= armSpacing;
        float t = fabsf(sectorAngle) / halfArm;
        float intensity = (t <= 1.0f) ? 0.5f * (1.0f + cosf(PI * t)) : 0.0f;
        intensity *= 0.6f + 0.4f * (r / rMax);
        leds[SP_LED(row, col)] = blend(bgColor, armColor, (uint8_t)(intensity * 255.0f));
      }
    }
  }

public:
  Spiral() : _phase(0.0f), _lastFrameMs(0) {}

  void Update(CRGB armColor, CRGB bgColor,
              uint32_t speedMs, float armThickness, bool clockwise = true) {
    uint32_t now = millis();
    if (now - _lastFrameMs < SPIRAL_FRAME_MS) return;
    uint32_t elapsed = now - _lastFrameMs;
    _lastFrameMs = now;
    if (speedMs > 0) {
      float delta = (2.0f * PI) * ((float)elapsed / (float)speedMs);
      _phase += clockwise ? delta : -delta;
      if (_phase >= 2.0f * PI) _phase -= 2.0f * PI;
      if (_phase <  0.0f)      _phase += 2.0f * PI;
    }
    draw(armColor, bgColor, armThickness);
  }

  void Reset() {
    _phase = 0.0f; _lastFrameMs = 0;
    for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  }
};

// CLASS: SpiralQ -- rotating spiral in one QUAD_WIDTH x QUAD_HEIGHT quad.
class SpiralQ {
private:
  uint16_t _qRowOff;
  uint16_t _qColOff;
  float    _phase;
  uint32_t _lastFrameMs;

  void draw(CRGB armColor, CRGB bgColor, float armThickness) {
    const float cx = (QUAD_WIDTH  - 1) * 0.5f;
    const float cy = (QUAD_HEIGHT - 1) * 0.5f;
    const float rMax = sqrtf(cx * cx + cy * cy);
    const float armSpacing = 2.0f * PI / (float)SQ_ARMS;
    float safeFrac = constrain(armThickness, 0.05f, 0.90f);
    const float halfArm = armSpacing * safeFrac * 0.5f;
    for (uint16_t row = 0; row < QUAD_HEIGHT; row++) {
      for (uint16_t col = 0; col < QUAD_WIDTH; col++) {
        float dx = (float)col - cx, dy = (float)row - cy;
        float r = sqrtf(dx * dx + dy * dy);
        if (r < 0.5f) {
          leds[SP_LED(_qRowOff + row, _qColOff + col)] = armColor; continue;
        }
        float theta = atan2f(dy, dx);
        float spiralAngle = fmodf(theta - _sqTightness * r - _phase, 2.0f * PI);
        if (spiralAngle < 0.0f) spiralAngle += 2.0f * PI;
        float sectorAngle = fmodf(spiralAngle, armSpacing);
        if (sectorAngle > armSpacing * 0.5f) sectorAngle -= armSpacing;
        float t = fabsf(sectorAngle) / halfArm;
        float intensity = (t <= 1.0f) ? 0.5f * (1.0f + cosf(PI * t)) : 0.0f;
        intensity *= 0.6f + 0.4f * (r / rMax);
        leds[SP_LED(_qRowOff + row, _qColOff + col)] =
            blend(bgColor, armColor, (uint8_t)(intensity * 255.0f));
      }
    }
  }

public:
  SpiralQ() : _qRowOff(0), _qColOff(0), _phase(0.0f), _lastFrameMs(0) {}

  void init(uint16_t rowOff, uint16_t colOff) { _qRowOff = rowOff; _qColOff = colOff; }

  void Update(CRGB armColor, CRGB bgColor,
              uint32_t speedMs, float armThickness, bool clockwise = true) {
    uint32_t now = millis();
    if (now - _lastFrameMs < SQ_FRAME_MS) return;
    uint32_t elapsed = now - _lastFrameMs;
    _lastFrameMs = now;
    if (speedMs > 0) {
      float delta = (2.0f * PI) * ((float)elapsed / (float)speedMs);
      _phase += clockwise ? delta : -delta;
      if (_phase >= 2.0f * PI) _phase -= 2.0f * PI;
      if (_phase <  0.0f)      _phase += 2.0f * PI;
    }
    draw(armColor, bgColor, armThickness);
  }
};

// Internal array -- always exactly 4 quads
static SpiralQ _sq[4];

// updateMatrix_4quad_Spiral -- rotating spiral in one QUAD_WIDTH x QUAD_HEIGHT quad.
//
// Parameters:
//   qCol, qRow   -- quad address: qCol in {1,2}, qRow in {1,2}
//   armColor     -- colour of the spiral arms
//   bgColor      -- background colour between arms
//   speedMs      -- ms per full revolution
//   armThickness -- 0.05 (thin) .. 0.90 (wide)
//   clockwise    -- true = clockwise (default)
//
// Non-blocking. Does NOT call FastLED.show().
static void updateMatrix_4quad_Spiral(uint8_t qCol, uint8_t qRow,
                                   CRGB armColor, CRGB bgColor,
                                   uint32_t speedMs, float armThickness,
                                   bool clockwise = true) {
  if (qCol < 1 || qCol > 2) return;
  if (qRow < 1 || qRow > 2) return;
  _sq[_qIdx(qCol, qRow)].Update(armColor, bgColor, speedMs, armThickness, clockwise);
}

static Spiral fullSpiral;

/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/