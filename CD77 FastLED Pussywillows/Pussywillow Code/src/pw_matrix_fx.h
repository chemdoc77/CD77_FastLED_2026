// pw_matrix_fx.h
// Rotating spiral and concentric box-fill effects for the pussywillow matrix.
//
// Adapted from CD77_Spiral_FX.h, CD77_Spiral_FX_P.h,
// CD77_Matrix_loop_functions.h and CD77_Matrix_loop_functions_P.h
// by Chemdoc77.  Original code licensed CC BY-NC-SA 4.0.
//
// Adapted for the pussywillow 10 x 15 WS2811 serpentine matrix which has
// a unique physical LED mapping stored in pw_ledMap[MATRIX_HEIGHT][MATRIX_WIDTH]
// (defined in pussywillow_functions.h, which must be included before this file).
//
// Instead of the generic wiring-formula map builder from the original sketch,
// pw_sp_initLedMap() populates sp_ledMap[] directly from pw_ledMap so every
// effect uses the correct physical LED indices automatically.
//
// IMPORTANT: Call pw_sp_init() in setup() AFTER FastLED.addLeds().
//
// ── FULL MATRIX FUNCTIONS ────────────────────────────────────────────────────
//
//   fullSpiral.Update(armColor, bgColor, speedMs, thickness, CW);
//   fullSpiral.Reset();
//
//   fullSpiralP.Update(palette, bgColor, speedMs, thickness, CW, palSpeed);
//   fullSpiralP.Reset();
//
//   updateMatrixBoxes(color, intervalMs, holdMs, reverse);
//     Reset sentinel: sp_ring = -1;
//
//   updateMatrixBoxesP(palette, bgColor, intervalMs, holdMs, reverse, palSpeed);
//     Reset sentinel: sp_ring = -1;
//
// ── FRONT / BACK AREA FUNCTIONS ──────────────────────────────────────────────
//  The matrix is divided into two 15x5 areas:
//    Front = columns 0–4 (left half)   Back = columns 5–9 (right half)
//
//   updateAreaSpiral(area, armColor, bgColor, speedMs, thickness, CW);
//   updateAreaSpiralP(area, palette, bgColor, speedMs, thickness, CW, palSpeed);
//
//   updateAreaBoxes(area, color, intervalMs, holdMs, reverse);
//     Reset sentinel: spa_front_ring = -1;  /  spa_back_ring = -1;
//
//   updateAreaBoxesP(area, palette, bgColor, intervalMs, holdMs, reverse, palSpeed);
//     Reset sentinel: spa_front_ring = -1;  /  spa_back_ring = -1;
//
// All functions are non-blocking and call FastLED.show() internally.
//
// ── RESET SENTINELS — add to your AT() restart line as needed ────────────────
//   sp_ring       = -1;   // full-matrix box functions
//   spa_front_ring = -1;  // front area box functions
//   spa_back_ring  = -1;  // back area box functions
//   Spiral / SpiralP objects reset automatically (phase-based).

#pragma once
#include <FastLED.h>
#include <math.h>

extern CRGB leds[];

// ─────────────────────────────────────────────────────────────────────────────
//  LED MAP
//  sp_ledMap[] is a flat row-major array: sp_ledMap[row * MATRIX_WIDTH + col]
//  gives the physical LED index for visual position (row, col).
//  SP_LED(row, col) is the access macro used by all functions in this file.
//
//  pw_sp_initLedMap() copies from pw_ledMap (the pussywillow's unique map)
//  rather than computing from a wiring formula.
// ─────────────────────────────────────────────────────────────────────────────
static uint16_t sp_ledMap[NUM_LEDS];
#define SP_LED(row, col)  sp_ledMap[(uint16_t)(row) * MATRIX_WIDTH + (col)]

static void pw_sp_initLedMap() {
  for (uint16_t r = 0; r < MATRIX_HEIGHT; r++)
    for (uint16_t c = 0; c < MATRIX_WIDTH; c++)
      sp_ledMap[r * MATRIX_WIDTH + c] = pw_ledMap[r][c];
}

// ─────────────────────────────────────────────────────────────────────────────
//  SPIRAL TIGHTNESS
//  Computed adaptively so the visual appearance is consistent — approximately
//  1.22 full arm rotations from centre to corner, matching the 16x16 reference.
//  Capped at 0.72 (the 16x16 reference value) to prevent over-tightening on
//  smaller matrices which produces a noisy, unreadable spiral.
// ─────────────────────────────────────────────────────────────────────────────
static float _spiralTightness = 0.72f;

static void _pw_computeTightness() {
  const float TARGET_RAD = 7.637f;  // 0.72 * rMax of a 16x16 matrix
  const float MAX_TIGHT  = 0.72f;
  float hfW = (MATRIX_WIDTH  - 1) * 0.5f;
  float hfH = (MATRIX_HEIGHT - 1) * 0.5f;
  float rMax = sqrtf(hfW * hfW + hfH * hfH);
  _spiralTightness = TARGET_RAD / rMax;
  if (_spiralTightness > MAX_TIGHT) _spiralTightness = MAX_TIGHT;
}

// ─────────────────────────────────────────────────────────────────────────────
//  CLASS: Spiral  —  single-colour rotating Archimedean spiral, full matrix
// ─────────────────────────────────────────────────────────────────────────────
#define SPIRAL_ARMS      2
#define SPIRAL_FRAME_MS  16

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

  // Call every loop() iteration. Calls FastLED.show() internally.
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
    FastLED.show();
  }

  void Reset() {
    _phase = 0.0f;
    _lastFrameMs = 0;
    for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  }
};

// ─────────────────────────────────────────────────────────────────────────────
//  CLASS: SpiralP  —  palette-coloured rotating Archimedean spiral, full matrix
// ─────────────────────────────────────────────────────────────────────────────
#define SP_ARMS      2
#define SP_FRAME_MS  16

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
          leds[SP_LED(row, col)] = ColorFromPalette(palette, _paletteOffset);
          continue;
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

  // Call every loop() iteration. Calls FastLED.show() internally.
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
    FastLED.show();
  }

  void Reset() {
    _phase = 0.0f;
    _paletteOffset = 0;
    _lastFrameMs = 0;
    for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  }
};

// Global instances — use these in Performance()
static Spiral  fullSpiral;
static SpiralP fullSpiralP;

// ─────────────────────────────────────────────────────────────────────────────
//  BOX-FILL STATE  —  shared by both updateMatrixBoxes and updateMatrixBoxesP
// ─────────────────────────────────────────────────────────────────────────────

// Number of concentric rings = min(MATRIX_WIDTH, MATRIX_HEIGHT) / 2
static const uint8_t SP_NUM_RINGS =
    (uint8_t)((MATRIX_WIDTH < MATRIX_HEIGHT) ? MATRIX_WIDTH / 2 : MATRIX_HEIGHT / 2);

static uint16_t _sp_sideLen(uint8_t ring, uint8_t side) {
  switch (side) {
    case 0: case 2: return (uint16_t)(MATRIX_HEIGHT - 2 * ring);
    case 1:         return (uint16_t)(MATRIX_WIDTH  - 2 * ring);
    case 3:         return (uint16_t)(MATRIX_WIDTH  - 2 * ring - 1);
    default:        return 0;
  }
}

static uint16_t _sp_getLED(uint8_t ring, uint8_t side, uint16_t step) {
  uint8_t  k = ring;
  uint16_t W = MATRIX_WIDTH, H = MATRIX_HEIGHT;
  switch (side) {
    case 0: return SP_LED(H - 1 - k - step, k);
    case 1: return SP_LED(k, k + step);
    case 2: return SP_LED(k + step, W - 1 - k);
    case 3: return SP_LED(H - 1 - k, W - 1 - k - step);
    default: return 0;
  }
}

enum SP_State { SP_RUNNING, SP_HOLD };
static SP_State  sp_state      = SP_RUNNING;
static int16_t   sp_ring       = -1;   // -1 = not yet initialised (reset sentinel)
static int16_t   sp_side       = 0;
static int16_t   sp_step       = 0;
static bool      sp_reverse    = false;
static uint32_t  sp_lastUpdate = 0;
static uint32_t  sp_holdStart  = 0;

// Palette offset for updateMatrixBoxesP — kept here so it resets with sp_ring = -1
static uint8_t   spp_palOffset = 0;

static void _sp_init(bool reverse) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  FastLED.show();
  sp_reverse = reverse;
  if (!reverse) { sp_ring = 0; sp_side = 0; sp_step = 0; }
  else {
    sp_ring = SP_NUM_RINGS - 1;
    sp_side = 3;
    sp_step = (int16_t)_sp_sideLen((uint8_t)(SP_NUM_RINGS - 1), 3) - 1;
  }
  sp_lastUpdate = millis();
  sp_state = SP_RUNNING;
}

static void _spP_init(bool reverse, CRGB bgColor) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i] = bgColor;
  FastLED.show();
  spp_palOffset = 0;
  sp_reverse = reverse;
  if (!reverse) { sp_ring = 0; sp_side = 0; sp_step = 0; }
  else {
    sp_ring = SP_NUM_RINGS - 1;
    sp_side = 3;
    sp_step = (int16_t)_sp_sideLen((uint8_t)(SP_NUM_RINGS - 1), 3) - 1;
  }
  sp_lastUpdate = millis();
  sp_state = SP_RUNNING;
}

// Palette index based on angular position around the matrix centre
static inline uint8_t _spP_palIdx(uint16_t row, uint16_t col, uint8_t offset) {
  float cy = (MATRIX_HEIGHT - 1) * 0.5f;
  float cx = (MATRIX_WIDTH  - 1) * 0.5f;
  float theta = atan2f((float)row - cy, (float)col - cx);
  return (uint8_t)((theta + (float)PI) * (255.0f / (2.0f * (float)PI))) + offset;
}

// ─────────────────────────────────────────────────────────────────────────────
//  updateMatrixBoxes  —  solid colour, full-matrix concentric ring fill
//
//  Traces concentric rectangular rings one LED at a time from outside in
//  (or inside out when reverse=true).  Holds when fully filled then repeats.
//
//  Reset: sp_ring = -1;
// ─────────────────────────────────────────────────────────────────────────────
static void updateMatrixBoxes(CRGB color, uint32_t intervalMs,
                               uint32_t holdMs, bool reverse = false) {
  if (sp_ring == -1) { _sp_init(reverse); return; }
  uint32_t now = millis();
  if (sp_state == SP_HOLD) {
    if (now - sp_holdStart < holdMs) return;
    _sp_init(reverse); return;
  }
  if (now - sp_lastUpdate < intervalMs) return;
  sp_lastUpdate = now;
  leds[_sp_getLED((uint8_t)sp_ring, (uint8_t)sp_side, (uint16_t)sp_step)] = color;
  FastLED.show();
  if (!reverse) {
    sp_step++;
    if (sp_step >= (int16_t)_sp_sideLen((uint8_t)sp_ring, (uint8_t)sp_side)) {
      sp_step = 0; sp_side++;
      if (sp_side > 3) {
        sp_side = 0; sp_ring++;
        if (sp_ring >= (int16_t)SP_NUM_RINGS) { sp_holdStart = now; sp_state = SP_HOLD; }
      }
    }
  } else {
    sp_step--;
    if (sp_step < 0) {
      sp_side--;
      if (sp_side < 0) {
        sp_ring--;
        if (sp_ring < 0) { sp_holdStart = now; sp_state = SP_HOLD; }
        else { sp_side = 3; sp_step = (int16_t)_sp_sideLen((uint8_t)sp_ring, 3) - 1; }
      } else { sp_step = (int16_t)_sp_sideLen((uint8_t)sp_ring, (uint8_t)sp_side) - 1; }
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  updateMatrixBoxesP  —  palette colour, full-matrix concentric ring fill
//
//  Same ring-fill behaviour as updateMatrixBoxes but each LED is coloured
//  from the palette based on its angular position around the matrix centre.
//  A paletteOffset counter drifts forward by paletteSpeed each step for a
//  flowing colour effect.
//
//  paletteSpeed: 0=static  3=gentle drift  10=fast  20=psychedelic
//
//  Reset: sp_ring = -1;
// ─────────────────────────────────────────────────────────────────────────────
static void updateMatrixBoxesP(const CRGBPalette16 &palette, CRGB bgColor,
                                uint32_t intervalMs, uint32_t holdMs,
                                bool reverse = false, uint8_t paletteSpeed = 3) {
  if (sp_ring == -1) { _spP_init(reverse, bgColor); return; }
  uint32_t now = millis();
  if (sp_state == SP_HOLD) {
    if (now - sp_holdStart < holdMs) return;
    _spP_init(reverse, bgColor); return;
  }
  if (now - sp_lastUpdate < intervalMs) return;
  sp_lastUpdate = now;
  uint16_t W = MATRIX_WIDTH, H = MATRIX_HEIGHT;
  uint8_t  k = (uint8_t)sp_ring;
  uint16_t row = 0, col = 0;
  switch (sp_side) {
    case 0: row = H - 1 - k - sp_step;  col = k;                 break;
    case 1: row = k;                     col = k + sp_step;        break;
    case 2: row = k + sp_step;           col = W - 1 - k;          break;
    case 3: row = H - 1 - k;            col = W - 1 - k - sp_step; break;
  }
  leds[SP_LED(row, col)] = ColorFromPalette(palette, _spP_palIdx(row, col, spp_palOffset));
  FastLED.show();
  spp_palOffset += paletteSpeed;
  if (!reverse) {
    sp_step++;
    if (sp_step >= (int16_t)_sp_sideLen((uint8_t)sp_ring, (uint8_t)sp_side)) {
      sp_step = 0; sp_side++;
      if (sp_side > 3) {
        sp_side = 0; sp_ring++;
        if (sp_ring >= (int16_t)SP_NUM_RINGS) { sp_holdStart = now; sp_state = SP_HOLD; }
      }
    }
  } else {
    sp_step--;
    if (sp_step < 0) {
      sp_side--;
      if (sp_side < 0) {
        sp_ring--;
        if (sp_ring < 0) { sp_holdStart = now; sp_state = SP_HOLD; }
        else { sp_side = 3; sp_step = (int16_t)_sp_sideLen((uint8_t)sp_ring, 3) - 1; }
      } else { sp_step = (int16_t)_sp_sideLen((uint8_t)sp_ring, (uint8_t)sp_side) - 1; }
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  AREA SPIRAL TIGHTNESS
//  Computed for AREA_WIDTH x AREA_HEIGHT (5x15), capped at 0.72.
// ─────────────────────────────────────────────────────────────────────────────
#define AREA_HEIGHT  15
#define AREA_WIDTH    5
#define AREA_RINGS    3   // ceil(AREA_WIDTH/2) = 3 — covers all 75 LEDs including centre column

static float _areaSpiralTightness = 0.72f;

static void _pw_computeAreaTightness() {
  const float TARGET_RAD = 7.637f;
  const float MAX_TIGHT  = 0.72f;
  float hfW = (AREA_WIDTH  - 1) * 0.5f;
  float hfH = (AREA_HEIGHT - 1) * 0.5f;
  float rMax = sqrtf(hfW * hfW + hfH * hfH);
  _areaSpiralTightness = TARGET_RAD / rMax;
  if (_areaSpiralTightness > MAX_TIGHT) _areaSpiralTightness = MAX_TIGHT;
}

// ─────────────────────────────────────────────────────────────────────────────
//  pw_sp_init  —  call ONCE in setup() after FastLED.addLeds()
//
//  1. Populates sp_ledMap[] from the pussywillow's pw_ledMap.
//  2. Computes adaptive spiral tightness for the 10x15 matrix size.
// ─────────────────────────────────────────────────────────────────────────────
static void pw_sp_init() {
  pw_sp_initLedMap();
  _pw_computeTightness();
  _pw_computeAreaTightness();
}

/* License
   Original CD77_Spiral_FX / CD77_Matrix_loop_functions code by Chemdoc77.
   Licensed under Creative Commons Attribution License CC BY-NC-SA 4.0.
   For non-commercial use you are welcome to use the code in any way you like.
   For commercial use please contact the author.
*/

// ═════════════════════════════════════════════════════════════════════════════
//  FRONT / BACK AREA DEFINITIONS
//
//  The pussywillow 10x15 matrix is divided into two 15x5 areas:
//    Front = the 5 left  columns (cols 0–4)
//    Back  = the 5 right columns (cols 5–9)
//
//  Each area has its own LED map (from the uploaded spreadsheets),
//  its own spiral class instances, and its own box-fill state machine.
//
//  Area dimensions: AREA_HEIGHT=15  AREA_WIDTH=5  AREA_RINGS=2
// ═════════════════════════════════════════════════════════════════════════════

// ─────────────────────────────────────────────────────────────────────────────
//  AREA LED MAPS  —  sourced directly from the uploaded spreadsheets
//  areaMap[row][col]: row 0=top, row 14=bottom; col 0=leftmost in the area
// ─────────────────────────────────────────────────────────────────────────────
static const uint8_t pw_frontMap[15][5] = {
  { 67,  52,  37,  22,   7},  // row  0  (top)
  { 68,  53,  38,  23,   8},  // row  1
  { 66,  51,  36,  21,   6},  // row  2
  { 69,  54,  39,  24,   9},  // row  3
  { 65,  50,  35,  20,   5},  // row  4
  { 70,  55,  40,  25,  10},  // row  5
  { 64,  49,  34,  19,   4},  // row  6
  { 71,  56,  41,  26,  11},  // row  7
  { 63,  48,  33,  18,   3},  // row  8
  { 72,  57,  42,  27,  12},  // row  9
  { 62,  47,  32,  17,   2},  // row 10
  { 73,  58,  43,  28,  13},  // row 11
  { 61,  46,  31,  16,   1},  // row 12
  { 74,  59,  44,  29,  14},  // row 13
  { 60,  45,  30,  15,   0},  // row 14 (bottom)
};

static const uint8_t pw_backMap[15][5] = {
  { 82,  97, 112, 127, 142},  // row  0  (top)
  { 83,  98, 113, 128, 143},  // row  1
  { 81,  96, 111, 126, 141},  // row  2
  { 84,  99, 114, 129, 144},  // row  3
  { 80,  95, 110, 125, 140},  // row  4
  { 85, 100, 115, 130, 145},  // row  5
  { 79,  94, 109, 124, 139},  // row  6
  { 86, 101, 116, 131, 146},  // row  7
  { 78,  93, 108, 123, 138},  // row  8
  { 87, 102, 117, 132, 147},  // row  9
  { 77,  92, 107, 122, 137},  // row 10
  { 88, 103, 118, 133, 148},  // row 11
  { 76,  91, 106, 121, 136},  // row 12
  { 89, 104, 119, 134, 149},  // row 13
  { 75,  90, 105, 120, 135},  // row 14 (bottom)
};

// Macro: physical LED index from (areaMap, row, col)
#define AREA_LED(areaMap, row, col)  (areaMap)[(row)][(col)]

// ─────────────────────────────────────────────────────────────────────────────
//  CLASS: SpiralArea  —  single-colour rotating spiral in one area
// ─────────────────────────────────────────────────────────────────────────────
#define SPIRAL_AREA_ARMS      2
#define SPIRAL_AREA_FRAME_MS  16

class SpiralArea {
private:
  const uint8_t (*_map)[5];   // pointer to pw_frontMap or pw_backMap
  float    _phase;
  uint32_t _lastFrameMs;

  void draw(CRGB armColor, CRGB bgColor, float armThickness) {
    const float cx = (AREA_WIDTH  - 1) * 0.5f;
    const float cy = (AREA_HEIGHT - 1) * 0.5f;
    const float rMax = sqrtf(cx * cx + cy * cy);
    const float armSpacing = 2.0f * PI / (float)SPIRAL_AREA_ARMS;
    float safeFrac = constrain(armThickness, 0.05f, 0.90f);
    const float halfArm = armSpacing * safeFrac * 0.5f;
    for (uint16_t row = 0; row < AREA_HEIGHT; row++) {
      for (uint16_t col = 0; col < AREA_WIDTH; col++) {
        float dx = (float)col - cx, dy = (float)row - cy;
        float r = sqrtf(dx * dx + dy * dy);
        if (r < 0.5f) { leds[_map[row][col]] = armColor; continue; }
        float theta = atan2f(dy, dx);
        float spiralAngle = fmodf(theta - _areaSpiralTightness * r - _phase, 2.0f * PI);
        if (spiralAngle < 0.0f) spiralAngle += 2.0f * PI;
        float sectorAngle = fmodf(spiralAngle, armSpacing);
        if (sectorAngle > armSpacing * 0.5f) sectorAngle -= armSpacing;
        float t = fabsf(sectorAngle) / halfArm;
        float intensity = (t <= 1.0f) ? 0.5f * (1.0f + cosf(PI * t)) : 0.0f;
        intensity *= 0.6f + 0.4f * (r / rMax);
        leds[_map[row][col]] = blend(bgColor, armColor, (uint8_t)(intensity * 255.0f));
      }
    }
  }

public:
  SpiralArea(const uint8_t (*areaMap)[5])
    : _map(areaMap), _phase(0.0f), _lastFrameMs(0) {}

  // Call every loop() iteration. Calls FastLED.show() internally.
  void Update(CRGB armColor, CRGB bgColor,
              uint32_t speedMs, float armThickness, bool clockwise = true) {
    uint32_t now = millis();
    if (now - _lastFrameMs < SPIRAL_AREA_FRAME_MS) return;
    uint32_t elapsed = now - _lastFrameMs;
    _lastFrameMs = now;
    if (speedMs > 0) {
      float delta = (2.0f * PI) * ((float)elapsed / (float)speedMs);
      _phase += clockwise ? delta : -delta;
      if (_phase >= 2.0f * PI) _phase -= 2.0f * PI;
      if (_phase <  0.0f)      _phase += 2.0f * PI;
    }
    draw(armColor, bgColor, armThickness);
    FastLED.show();
  }

  void Reset() {
    _phase = 0.0f; _lastFrameMs = 0;
    for (uint16_t r = 0; r < AREA_HEIGHT; r++)
      for (uint16_t c = 0; c < AREA_WIDTH; c++)
        leds[_map[r][c]] = CRGB::Black;
  }
};

// ─────────────────────────────────────────────────────────────────────────────
//  CLASS: SpiralAreaP  —  palette-coloured rotating spiral in one area
// ─────────────────────────────────────────────────────────────────────────────
#define SPIRAL_AREAP_ARMS      2
#define SPIRAL_AREAP_FRAME_MS  16

class SpiralAreaP {
private:
  const uint8_t (*_map)[5];
  float    _phase;
  uint8_t  _paletteOffset;
  uint32_t _lastFrameMs;

  void draw(const CRGBPalette16 &palette, CRGB bgColor, float armThickness) {
    const float cx = (AREA_WIDTH  - 1) * 0.5f;
    const float cy = (AREA_HEIGHT - 1) * 0.5f;
    const float rMax = sqrtf(cx * cx + cy * cy);
    const float armSpacing = 2.0f * PI / (float)SPIRAL_AREAP_ARMS;
    float safeFrac = constrain(armThickness, 0.05f, 0.90f);
    const float halfArm = armSpacing * safeFrac * 0.5f;
    for (uint16_t row = 0; row < AREA_HEIGHT; row++) {
      for (uint16_t col = 0; col < AREA_WIDTH; col++) {
        float dx = (float)col - cx, dy = (float)row - cy;
        float r = sqrtf(dx * dx + dy * dy);
        if (r < 0.5f) {
          leds[_map[row][col]] = ColorFromPalette(palette, _paletteOffset);
          continue;
        }
        float theta = atan2f(dy, dx);
        uint8_t palIdx = (uint8_t)((theta + PI) * (255.0f / (2.0f * PI))) + _paletteOffset;
        CRGB armColor = ColorFromPalette(palette, palIdx);
        float spiralAngle = fmodf(theta - _areaSpiralTightness * r - _phase, 2.0f * PI);
        if (spiralAngle < 0.0f) spiralAngle += 2.0f * PI;
        float sectorAngle = fmodf(spiralAngle, armSpacing);
        if (sectorAngle > armSpacing * 0.5f) sectorAngle -= armSpacing;
        float t = fabsf(sectorAngle) / halfArm;
        float intensity = (t <= 1.0f) ? 0.5f * (1.0f + cosf(PI * t)) : 0.0f;
        intensity *= 0.6f + 0.4f * (r / rMax);
        leds[_map[row][col]] = blend(bgColor, armColor, (uint8_t)(intensity * 255.0f));
      }
    }
  }

public:
  SpiralAreaP(const uint8_t (*areaMap)[5])
    : _map(areaMap), _phase(0.0f), _paletteOffset(0), _lastFrameMs(0) {}

  // Call every loop() iteration. Calls FastLED.show() internally.
  void Update(const CRGBPalette16 &palette, CRGB bgColor,
              uint32_t speedMs, float armThickness,
              bool clockwise = true, uint8_t paletteSpeed = 3) {
    uint32_t now = millis();
    if (now - _lastFrameMs < SPIRAL_AREAP_FRAME_MS) return;
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
    FastLED.show();
  }

  void Reset() {
    _phase = 0.0f; _paletteOffset = 0; _lastFrameMs = 0;
    for (uint16_t r = 0; r < AREA_HEIGHT; r++)
      for (uint16_t c = 0; c < AREA_WIDTH; c++)
        leds[_map[r][c]] = CRGB::Black;
  }
};

// Global area spiral instances
static SpiralArea  frontSpiral(pw_frontMap);
static SpiralArea  backSpiral(pw_backMap);
static SpiralAreaP frontSpiralP(pw_frontMap);
static SpiralAreaP backSpiralP(pw_backMap);

// ─────────────────────────────────────────────────────────────────────────────
//  AREA BOX-FILL STATE MACHINE
//
//  Each area (front / back) has its own independent state so both can run
//  simultaneously with different colours, directions and timing.
//
//  Area ring geometry (AREA_WIDTH=5, AREA_HEIGHT=15):
//    Ring 0: outermost rectangle, 5 wide x 15 tall
//    Ring 1: one step inside,     3 wide x 13 tall
//    AREA_RINGS = 2
//
//  Side traversal (same convention as full-matrix box-fill):
//    Side 0: left  col, bottom → top
//    Side 1: top   row, left   → right
//    Side 2: right col, top    → bottom
//    Side 3: bottom row, right → left (shorter — avoids corner overlap)
// ─────────────────────────────────────────────────────────────────────────────

// Side lengths for an area ring.
// Special case: when the ring reduces to a single column (AREA_WIDTH - 2*ring == 1),
// only side 0 is traversed (the full column) — sides 1, 2, 3 return 0 to avoid
// double-counting the same column or visiting out-of-bounds positions.
static uint16_t _spa_sideLen(uint8_t ring, uint8_t side) {
  uint16_t w = (uint16_t)(AREA_WIDTH  - 2 * ring);
  uint16_t h = (uint16_t)(AREA_HEIGHT - 2 * ring);
  if (w == 0 || h == 0) return 0;
  if (w == 1) return (side == 0) ? h : 0;   // single column — side 0 only
  switch (side) {
    case 0: case 2: return h;
    case 1:         return w;
    case 3:         return w - 1;
    default:        return 0;
  }
}

// Physical LED index for a given area (ring, side, step)
static uint16_t _spa_getLED(const uint8_t (*areaMap)[5],
                              uint8_t ring, uint8_t side, uint16_t step) {
  uint8_t  k = ring;
  uint16_t W = AREA_WIDTH, H = AREA_HEIGHT;
  switch (side) {
    case 0: return areaMap[H - 1 - k - step][k];
    case 1: return areaMap[k][k + step];
    case 2: return areaMap[k + step][W - 1 - k];
    case 3: return areaMap[H - 1 - k][W - 1 - k - step];
    default: return 0;
  }
}

// State struct — one instance per area
struct SPA_State {
  SP_State state;
  int16_t  ring;       // -1 = not yet initialised (reset sentinel)
  int16_t  side;
  int16_t  step;
  bool     reverse;
  uint32_t lastUpdate;
  uint32_t holdStart;
  uint8_t  palOffset;  // used by BoxesP only
};

static SPA_State _spa_front = { SP_RUNNING, -1, 0, 0, false, 0, 0, 0 };
static SPA_State _spa_back  = { SP_RUNNING, -1, 0, 0, false, 0, 0, 0 };

// Convenience aliases matching the reset-sentinel names in the comments
#define spa_front_ring  _spa_front.ring
#define spa_back_ring   _spa_back.ring

static void _spa_init(SPA_State &s, const uint8_t (*areaMap)[5],
                       bool reverse, CRGB bgColor, bool fullClear = false) {
  if (fullClear) {
    // First init — clear the entire matrix so nothing from a previous
    // effect bleeds through into the other area
    for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i] = bgColor;
  } else {
    // Hold restart — clear only this area
    for (uint16_t r = 0; r < AREA_HEIGHT; r++)
      for (uint16_t c = 0; c < AREA_WIDTH; c++)
        leds[areaMap[r][c]] = bgColor;
  }
  FastLED.show();
  s.reverse   = reverse;
  s.palOffset = 0;
  if (!reverse) { s.ring = 0; s.side = 0; s.step = 0; }
  else {
    // Start at the last LED of the innermost ring.
    // Find the last side that has any LEDs (single-column ring only uses side 0).
    s.ring = AREA_RINGS - 1;
    s.side = 3;
    while (s.side > 0 && _spa_sideLen((uint8_t)s.ring, (uint8_t)s.side) == 0)
      s.side--;
    s.step = (int16_t)_spa_sideLen((uint8_t)s.ring, (uint8_t)s.side) - 1;
  }
  s.lastUpdate = millis();
  s.state      = SP_RUNNING;
}

// Step the box-fill state machine forward one LED
static void _spa_advance(SPA_State &s) {
  if (!s.reverse) {
    s.step++;
    if (s.step >= (int16_t)_spa_sideLen((uint8_t)s.ring, (uint8_t)s.side)) {
      s.step = 0; s.side++;
      // Skip sides with zero length (e.g. sides 1,2,3 of single-column ring)
      while (s.side <= 3 && _spa_sideLen((uint8_t)s.ring, (uint8_t)s.side) == 0)
        s.side++;
      if (s.side > 3) {
        s.side = 0; s.ring++;
      }
    }
  } else {
    s.step--;
    if (s.step < 0) {
      s.side--;
      // Skip zero-length sides going backwards
      while (s.side >= 0 && _spa_sideLen((uint8_t)s.ring, (uint8_t)s.side) == 0)
        s.side--;
      if (s.side < 0) {
        s.ring--;
        if (s.ring >= 0) {
          s.side = 3;
          // Find last valid side of the previous ring
          while (s.side > 0 && _spa_sideLen((uint8_t)s.ring, (uint8_t)s.side) == 0)
            s.side--;
          s.step = (int16_t)_spa_sideLen((uint8_t)s.ring, (uint8_t)s.side) - 1;
        }
      } else {
        s.step = (int16_t)_spa_sideLen((uint8_t)s.ring, (uint8_t)s.side) - 1;
      }
    }
  }
}

// Palette index based on angular position around the area centre
static inline uint8_t _spaP_palIdx(uint16_t row, uint16_t col, uint8_t offset) {
  float cy = (AREA_HEIGHT - 1) * 0.5f;
  float cx = (AREA_WIDTH  - 1) * 0.5f;
  float theta = atan2f((float)row - cy, (float)col - cx);
  return (uint8_t)((theta + (float)PI) * (255.0f / (2.0f * (float)PI))) + offset;
}

// ─────────────────────────────────────────────────────────────────────────────
//  updateAreaBoxes  —  solid colour area box-fill (use area = FRONT or BACK)
//
//  Traces concentric rectangular rings across the area one LED at a time.
//  Holds when fully filled then clears and repeats.
//
//  Reset: spa_front_ring = -1;  /  spa_back_ring = -1;
// ─────────────────────────────────────────────────────────────────────────────
static void _updateAreaBoxes(SPA_State &s, const uint8_t (*areaMap)[5],
                               CRGB color, uint32_t intervalMs,
                               uint32_t holdMs, bool reverse) {
  if (s.ring == -1) { _spa_init(s, areaMap, reverse, CRGB::Black, true); return; }
  uint32_t now = millis();
  if (s.state == SP_HOLD) {
    if (now - s.holdStart < holdMs) return;
    _spa_init(s, areaMap, reverse, CRGB::Black, false); return;
  }
  if (now - s.lastUpdate < intervalMs) return;
  s.lastUpdate = now;
  leds[_spa_getLED(areaMap, (uint8_t)s.ring, (uint8_t)s.side, (uint16_t)s.step)] = color;
  FastLED.show();
  bool done = !s.reverse ? (s.ring >= (int16_t)AREA_RINGS)
                         : (s.ring < 0);
  if (!done) _spa_advance(s);
  done = !s.reverse ? (s.ring >= (int16_t)AREA_RINGS)
                    : (s.ring < 0);
  if (done) { s.holdStart = now; s.state = SP_HOLD; }
}

// ─────────────────────────────────────────────────────────────────────────────
//  updateAreaBoxesP  —  palette colour area box-fill (use area = FRONT or BACK)
//
//  Same ring-fill behaviour but each LED is coloured from the palette based
//  on its angular position around the area centre.
//
//  paletteSpeed: 0=static  3=gentle drift  10=fast  20=psychedelic
//
//  Reset: spa_front_ring = -1;  /  spa_back_ring = -1;
// ─────────────────────────────────────────────────────────────────────────────
static void _updateAreaBoxesP(SPA_State &s, const uint8_t (*areaMap)[5],
                                const CRGBPalette16 &palette, CRGB bgColor,
                                uint32_t intervalMs, uint32_t holdMs,
                                bool reverse, uint8_t paletteSpeed) {
  if (s.ring == -1) { _spa_init(s, areaMap, reverse, bgColor, true); return; }
  uint32_t now = millis();
  if (s.state == SP_HOLD) {
    if (now - s.holdStart < holdMs) return;
    _spa_init(s, areaMap, reverse, bgColor, false); return;
  }
  if (now - s.lastUpdate < intervalMs) return;
  s.lastUpdate = now;

  // Compute visual row/col from current (ring, side, step) for palette lookup
  uint8_t  k = (uint8_t)s.ring;
  uint16_t W = AREA_WIDTH, H = AREA_HEIGHT;
  uint16_t row = 0, col = 0;
  switch (s.side) {
    case 0: row = H - 1 - k - s.step;  col = k;                 break;
    case 1: row = k;                    col = k + s.step;         break;
    case 2: row = k + s.step;           col = W - 1 - k;          break;
    case 3: row = H - 1 - k;           col = W - 1 - k - s.step;  break;
  }
  leds[areaMap[row][col]] =
      ColorFromPalette(palette, _spaP_palIdx(row, col, s.palOffset));
  FastLED.show();
  s.palOffset += paletteSpeed;

  bool done = !s.reverse ? (s.ring >= (int16_t)AREA_RINGS)
                         : (s.ring < 0);
  if (!done) _spa_advance(s);
  done = !s.reverse ? (s.ring >= (int16_t)AREA_RINGS)
                    : (s.ring < 0);
  if (done) { s.holdStart = now; s.state = SP_HOLD; }
}

// ═════════════════════════════════════════════════════════════════════════════
//  COMBINED AREA FUNCTIONS
//
//  Each of the four area effect pairs is also available as a single function
//  with a PW_Area selector variable:
//
//    updateAreaSpiral(area, armColor, bgColor, speedMs, thickness, CW);
//    updateAreaSpiralP(area, palette, bgColor, speedMs, thickness, CW, palSpeed);
//    updateAreaBoxes(area, color, intervalMs, holdMs, reverse);
//    updateAreaBoxesP(area, palette, bgColor, intervalMs, holdMs, reverse, palSpeed);
//
//  area: FRONT  or  BACK
//
//  Reset sentinels (unchanged):
//    spa_front_ring = -1;   spa_back_ring = -1;
// ═════════════════════════════════════════════════════════════════════════════

enum PW_Area { FRONT, BACK };

static void updateAreaSpiral(PW_Area area, CRGB armColor, CRGB bgColor,
                              uint32_t speedMs, float armThickness,
                              bool clockwise = true) {
  if (area == FRONT) frontSpiral.Update(armColor, bgColor, speedMs, armThickness, clockwise);
  else               backSpiral.Update(armColor, bgColor, speedMs, armThickness, clockwise);
}

static void updateAreaSpiralP(PW_Area area, const CRGBPalette16 &palette, CRGB bgColor,
                               uint32_t speedMs, float armThickness,
                               bool clockwise = true, uint8_t paletteSpeed = 3) {
  if (area == FRONT) frontSpiralP.Update(palette, bgColor, speedMs, armThickness, clockwise, paletteSpeed);
  else               backSpiralP.Update(palette, bgColor, speedMs, armThickness, clockwise, paletteSpeed);
}

static void updateAreaBoxes(PW_Area area, CRGB color, uint32_t intervalMs,
                             uint32_t holdMs, bool reverse = false) {
  if (area == FRONT) _updateAreaBoxes(_spa_front, pw_frontMap, color, intervalMs, holdMs, reverse);
  else               _updateAreaBoxes(_spa_back,  pw_backMap,  color, intervalMs, holdMs, reverse);
}

static void updateAreaBoxesP(PW_Area area, const CRGBPalette16 &palette, CRGB bgColor,
                              uint32_t intervalMs, uint32_t holdMs,
                              bool reverse = false, uint8_t paletteSpeed = 3) {
  if (area == FRONT) _updateAreaBoxesP(_spa_front, pw_frontMap, palette, bgColor, intervalMs, holdMs, reverse, paletteSpeed);
  else               _updateAreaBoxesP(_spa_back,  pw_backMap,  palette, bgColor, intervalMs, holdMs, reverse, paletteSpeed);
}
