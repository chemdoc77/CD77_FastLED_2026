// CD77_Spiral_FX_P.h
// Palette-coloured rotating spiral effects.  Companion to CD77_Spiral_FX.h.
//
// Two palette classes:
//   SpiralP  -- palette spiral across the full matrix
//   SpiralQP -- palette spiral in one QUAD_WIDTH x QUAD_HEIGHT quad
//
// There are always exactly 4 quads (QUAD_WIDTH x QUAD_HEIGHT each).
// For quad palette spirals use updateMatrix_4quad_SpiralP().
// Addressing: (qCol, qRow), qCol in {1,2}, qRow in {1,2}, (1,1) = top-left.
//
// sp_init() MUST be called in setup().  It builds the LED map, sets adaptive
// tightness for the configured matrix/quad size, and initialises all objects.
//
// Adaptive tightness:
//   _spiralTightness and _sqTightness are computed so the spiral looks the same
//   on any matrix size -- approximately 1.22 full rotations from centre to corner,
//   matching the reference 16x16 appearance.
//     16x16 -> full=0.72   quad(8x8)=1.54
//      8x8  -> full=1.54   quad(4x4)=3.60
//     10x10 -> full=1.20   quad(5x5)=2.70
//     32x16 -> full=0.44   quad(16x8)=0.92

#pragma once
#include "CD77_Spiral_FX.h"

#define SP_ARMS        2
#define SP_FRAME_MS    16
#define SQP_ARMS       2
#define SQP_FRAME_MS   16

// CLASS: SpiralP -- palette spiral, full matrix
class SpiralP {
private:
  float    _phase;
  uint8_t  _paletteOffset;
  uint32_t _lastFrameMs;

  void draw(const CRGBPalette16 &palette, CRGB bgColor, float armThickness) {
    const float cx = (MATRIX_WIDTH  - 1) * 0.5f;
    const float cy = (MATRIX_HEIGHT - 1) * 0.5f;
    const float rMax = sqrtf(cx * cx + cy * cy);
    const float armSpacing = 2.0f * PI / (float)SP_ARMS;
    float safeFrac = constrain(armThickness, 0.05f, 0.90f);
    const float halfArm = armSpacing * safeFrac * 0.5f;
    for (uint16_t row = 0; row < MATRIX_HEIGHT; row++) {
      for (uint16_t col = 0; col < MATRIX_WIDTH; col++) {
        float dx = (float)col - cx, dy = (float)row - cy;
        float r = sqrtf(dx * dx + dy * dy);
        if (r < 0.5f) {
          leds[SP_LED(row, col)] = ColorFromPalette(palette, _paletteOffset); continue;
        }
        float theta = atan2f(dy, dx);
        uint8_t palIdx = (uint8_t)((theta + PI) * (255.0f / (2.0f * PI))) + _paletteOffset;
        CRGB armColor = ColorFromPalette(palette, palIdx);
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
  SpiralP() : _phase(0.0f), _paletteOffset(0), _lastFrameMs(0) {}

  void Update(const CRGBPalette16 &palette, CRGB bgColor,
              uint32_t speedMs, float armThickness,
              bool clockwise = true, uint8_t paletteSpeed = 3) {
    uint32_t now = millis();
    if (now - _lastFrameMs < SP_FRAME_MS) return;
    uint32_t elapsed = now - _lastFrameMs;
    _lastFrameMs = now;
    if (speedMs > 0) {
      float delta = (2.0f * PI) * ((float)elapsed / (float)speedMs);
      _phase += clockwise ? delta : -delta;
      if (_phase >= 2.0f * PI) _phase -= 2.0f * PI;
      if (_phase <  0.0f)      _phase += 2.0f * PI;
    }
    _paletteOffset += paletteSpeed;
    draw(palette, bgColor, armThickness);
  }

  void Reset() {
    _phase = 0.0f; _paletteOffset = 0; _lastFrameMs = 0;
    for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  }
};

// CLASS: SpiralQP -- palette spiral in one QUAD_WIDTH x QUAD_HEIGHT quad.
class SpiralQP {
private:
  uint16_t _qRowOff;
  uint16_t _qColOff;
  float    _phase;
  uint8_t  _paletteOffset;
  uint32_t _lastFrameMs;

  void draw(const CRGBPalette16 &palette, CRGB bgColor, float armThickness) {
    const float cx = (QUAD_WIDTH  - 1) * 0.5f;
    const float cy = (QUAD_HEIGHT - 1) * 0.5f;
    const float rMax = sqrtf(cx * cx + cy * cy);
    const float armSpacing = 2.0f * PI / (float)SQP_ARMS;
    float safeFrac = constrain(armThickness, 0.05f, 0.90f);
    const float halfArm = armSpacing * safeFrac * 0.5f;
    for (uint16_t row = 0; row < QUAD_HEIGHT; row++) {
      for (uint16_t col = 0; col < QUAD_WIDTH; col++) {
        float dx = (float)col - cx, dy = (float)row - cy;
        float r = sqrtf(dx * dx + dy * dy);
        if (r < 0.5f) {
          leds[SP_LED(_qRowOff + row, _qColOff + col)] =
              ColorFromPalette(palette, _paletteOffset); continue;
        }
        float theta = atan2f(dy, dx);
        uint8_t palIdx = (uint8_t)((theta + PI) * (255.0f / (2.0f * PI))) + _paletteOffset;
        CRGB armColor = ColorFromPalette(palette, palIdx);
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
  SpiralQP() : _qRowOff(0), _qColOff(0), _phase(0.0f), _paletteOffset(0), _lastFrameMs(0) {}

  void init(uint16_t rowOff, uint16_t colOff) { _qRowOff = rowOff; _qColOff = colOff; }

  void Update(const CRGBPalette16 &palette, CRGB bgColor,
              uint32_t speedMs, float armThickness,
              bool clockwise = true, uint8_t paletteSpeed = 3) {
    uint32_t now = millis();
    if (now - _lastFrameMs < SQP_FRAME_MS) return;
    uint32_t elapsed = now - _lastFrameMs;
    _lastFrameMs = now;
    if (speedMs > 0) {
      float delta = (2.0f * PI) * ((float)elapsed / (float)speedMs);
      _phase += clockwise ? delta : -delta;
      if (_phase >= 2.0f * PI) _phase -= 2.0f * PI;
      if (_phase <  0.0f)      _phase += 2.0f * PI;
    }
    _paletteOffset += paletteSpeed;
    draw(palette, bgColor, armThickness);
  }
};

// Internal array -- always exactly 4 quads
static SpiralQP _sqp[4];

// updateMatrix_4quad_SpiralP -- palette rotating spiral in one QUAD_WIDTH x QUAD_HEIGHT quad.
//
// Parameters:
//   qCol, qRow   -- quad address: qCol in {1,2}, qRow in {1,2}
//   palette      -- any CRGBPalette16 (can differ per quad)
//   bgColor      -- background colour between arms
//   speedMs      -- ms per full revolution
//   armThickness -- 0.05 (thin) .. 0.90 (wide)
//   clockwise    -- true = clockwise (default)
//   paletteSpeed -- 0=static  3=gentle  10=fast  20=psychedelic
//
// Non-blocking. Does NOT call FastLED.show().
static void updateMatrix_4quad_SpiralP(uint8_t qCol, uint8_t qRow,
                                    const CRGBPalette16 &palette, CRGB bgColor,
                                    uint32_t speedMs, float armThickness,
                                    bool clockwise = true, uint8_t paletteSpeed = 3) {
  if (qCol < 1 || qCol > 2) return;
  if (qRow < 1 || qRow > 2) return;
  _sqp[_qIdx(qCol, qRow)].Update(palette, bgColor, speedMs, armThickness, clockwise, paletteSpeed);
}

static SpiralP fullSpiralP;

// sp_init -- master initialisation.  Call ONCE in setup().
//
// 1. Builds the LED map for the configured wiring.
// 2. Computes spiral tightness adaptively so the visual appearance
//    (number of arm rotations, centre-to-corner) is consistent on
//    any matrix size.  Reference: 16x16 at tightness=0.72 gives
//    ~1.22 rotations for the full matrix and the 8x8 quads.
// 3. Initialises all 4 quad objects.
static void sp_init() {
  sp_initLedMap();

  // Adaptive tightness: maintain ~1.22 arm windings centre-to-corner for
  // matrices LARGER than 16x16, but never exceed 0.72 for smaller ones.
  //
  // Why cap at 0.72?
  //   0.72 is the reference constant tuned for 16x16 (evaluates to exactly
  //   0.720 for a 16x16 full matrix).  Scaling upward for smaller matrices
  //   causes over-tightening: on an 8x8 matrix the arms become so densely
  //   wound that adjacent pixels land randomly on arm vs. gap, producing
  //   a scrambled / noisy appearance.  Capping at 0.72 keeps the arms
  //   clearly visible at any size, with fewer windings on small matrices.
  //
  //   Examples (16x16 reference → tightness=0.72, ~1.22 windings):
  //     32x32 full  → 0.51  (scales down, fewer windings on bigger canvas)
  //     16x16 full  → 0.72  (reference, unchanged)
  //      8x8 full   → 0.72  (capped — would be 1.54 without cap)
  //      4x4 quad   → 0.72  (capped — would be 3.60 without cap)
  const float TARGET_RAD = 7.637f;   // 0.72 * rMax_of_16x16
  const float MAX_TIGHT  = 0.72f;

  float hfW = (MATRIX_WIDTH  - 1) * 0.5f;
  float hfH = (MATRIX_HEIGHT - 1) * 0.5f;
  float rMaxFull = sqrtf(hfW * hfW + hfH * hfH);
  _spiralTightness = TARGET_RAD / rMaxFull;
  if (_spiralTightness > MAX_TIGHT) _spiralTightness = MAX_TIGHT;

  float hfQW = (QUAD_WIDTH  - 1) * 0.5f;
  float hfQH = (QUAD_HEIGHT - 1) * 0.5f;
  float rMaxQuad = sqrtf(hfQW * hfQW + hfQH * hfQH);
  _sqTightness = TARGET_RAD / rMaxQuad;
  if (_sqTightness > MAX_TIGHT) _sqTightness = MAX_TIGHT;

  // Initialise quad objects
  _sq[0].init(0,           0);            _sqp[0].init(0,           0);
  _sq[1].init(0,           QUAD_WIDTH);   _sqp[1].init(0,           QUAD_WIDTH);
  _sq[2].init(QUAD_HEIGHT, 0);            _sqp[2].init(QUAD_HEIGHT, 0);
  _sq[3].init(QUAD_HEIGHT, QUAD_WIDTH);   _sqp[3].init(QUAD_HEIGHT, QUAD_WIDTH);
}


/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/