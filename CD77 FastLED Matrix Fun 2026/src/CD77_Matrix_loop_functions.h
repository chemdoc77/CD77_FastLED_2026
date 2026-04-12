// Matrix_loop_functions.h
// Concentric box-fill effects for any W x H WS2812b serpentine matrix.
// Non-blocking state machine modelled on the Adafruit multi-tasking approach.
//
// HOW TO USE
//  1. In your .ino, BEFORE any #includes, define:
//       #define MATRIX_WIDTH    16
//       #define MATRIX_HEIGHT   16
//       #define NUM_LEDS        (MATRIX_WIDTH * MATRIX_HEIGHT)
//       #define WIRING_ORIGIN   WIRING_ORIGIN_TOP_LEFT
//       #define WIRING_DIR      WIRING_DIR_COLUMNS
//  2. In setup() call  sp_init();
//  3. In loop() call the effect functions as needed.
//
// WIRING ORIGIN OPTIONS
//   WIRING_ORIGIN_TOP_LEFT     WIRING_ORIGIN_TOP_RIGHT
//   WIRING_ORIGIN_BOTTOM_LEFT  WIRING_ORIGIN_BOTTOM_RIGHT
//
// WIRING DIRECTION OPTIONS
//   WIRING_DIR_COLUMNS   serpentine stripes run along columns (default)
//   WIRING_DIR_ROWS      serpentine stripes run along rows
//
// QUAD ADDRESSING
//  There are always exactly 4 quadrants, each QUAD_WIDTH x QUAD_HEIGHT:
//    QUAD_WIDTH  = MATRIX_WIDTH  / 2
//    QUAD_HEIGHT = MATRIX_HEIGHT / 2
//
//  Addressed as (qCol, qRow), both 1-based, (1,1) = top-left:
//    (1,1) Top-L  |  (2,1) Top-R
//    (1,2) Bot-L  |  (2,2) Bot-R
//
//  Examples:
//    16x16 matrix -> 4 quads of  8x8
//    32x16 matrix -> 4 quads of 16x8
//     8x8  matrix -> 4 quads of  4x4
//    24x24 matrix -> 4 quads of 12x12

#pragma once
#include <FastLED.h>

extern CRGB leds[];

// Wiring configuration constants
#define WIRING_ORIGIN_TOP_LEFT     0
#define WIRING_ORIGIN_TOP_RIGHT    1
#define WIRING_ORIGIN_BOTTOM_LEFT  2
#define WIRING_ORIGIN_BOTTOM_RIGHT 3
#define WIRING_DIR_COLUMNS             0   // serpentine: even cols top->bottom, odd cols bottom->top
#define WIRING_DIR_ROWS                1   // serpentine: even rows left->right, odd rows right->left
#define WIRING_DIR_COLUMNS_PROGRESSIVE 2   // progressive: every column runs same direction
#define WIRING_DIR_ROWS_PROGRESSIVE    3   // progressive: every row runs same direction

#ifndef WIRING_ORIGIN
#define WIRING_ORIGIN WIRING_ORIGIN_TOP_LEFT
#endif
#ifndef WIRING_DIR
#define WIRING_DIR WIRING_DIR_COLUMNS
#endif

// True if wiring is serpentine (alternating stripe direction)
#define WIRING_SERPENTINE  (WIRING_DIR == WIRING_DIR_COLUMNS || WIRING_DIR == WIRING_DIR_ROWS)

// Quad dimensions -- always exactly 4 quads
#define QUAD_WIDTH    ((uint16_t)(MATRIX_WIDTH  / 2))
#define QUAD_HEIGHT   ((uint16_t)(MATRIX_HEIGHT / 2))
#define NUM_QUADS     4

// Convert user (qCol, qRow) 1-based to internal index 0-3
//   index 0 = (1,1) Top-L    index 1 = (2,1) Top-R
//   index 2 = (1,2) Bot-L    index 3 = (2,2) Bot-R
static inline uint8_t _qIdx(uint8_t qCol, uint8_t qRow) {
  return (uint8_t)((qRow - 1) * 2 + (qCol - 1));
}
// Top-left pixel offset of internal quad index q
static inline uint16_t _qRowOff(uint8_t q) { return (q / 2) * QUAD_HEIGHT; }
static inline uint16_t _qColOff(uint8_t q) { return (q % 2) * QUAD_WIDTH;  }

// LED index map  (flat, row-major)
// Access via SP_LED(row, col).  Built by sp_initLedMap().
static uint16_t sp_ledMap[NUM_LEDS];
#define SP_LED(row, col)  sp_ledMap[(uint16_t)(row) * MATRIX_WIDTH + (col)]

// sp_initLedMap -- build sp_ledMap for the configured wiring.
// Supports all 8 combinations of 4 origin corners x 2 run directions.
static void sp_initLedMap() {
  const bool rightOrigin  = (WIRING_ORIGIN == WIRING_ORIGIN_TOP_RIGHT  ||
                              WIRING_ORIGIN == WIRING_ORIGIN_BOTTOM_RIGHT);
  const bool bottomOrigin = (WIRING_ORIGIN == WIRING_ORIGIN_BOTTOM_LEFT ||
                              WIRING_ORIGIN == WIRING_ORIGIN_BOTTOM_RIGHT);
  const bool colDir       = (WIRING_DIR == WIRING_DIR_COLUMNS ||
                              WIRING_DIR == WIRING_DIR_COLUMNS_PROGRESSIVE);
  const bool serpentine   = WIRING_SERPENTINE;
  const uint16_t numStripes = colDir ? MATRIX_WIDTH  : MATRIX_HEIGHT;
  const uint16_t stripeLen  = colDir ? MATRIX_HEIGHT : MATRIX_WIDTH;
  uint16_t idx = 0;
  for (uint16_t s = 0; s < numStripes; s++) {
    uint16_t stripePos = rightOrigin ? (numStripes - 1 - s) : s;
    bool goingPositive;
    if (serpentine) {
      // Alternating stripe direction
      if (colDir) goingPositive = bottomOrigin ? (s % 2 == 1) : (s % 2 == 0);
      else        goingPositive = rightOrigin  ? (s % 2 == 1) : (s % 2 == 0);
    } else {
      // Progressive: every stripe runs the same direction from origin
      if (colDir) goingPositive = !bottomOrigin;
      else        goingPositive = !rightOrigin;
    }
    for (uint16_t p = 0; p < stripeLen; p++) {
      uint16_t physPos = goingPositive ? p : (stripeLen - 1 - p);
      uint16_t row = colDir ? physPos   : stripePos;
      uint16_t col = colDir ? stripePos : physPos;
      sp_ledMap[(uint16_t)row * MATRIX_WIDTH + col] = idx++;
    }
  }
}

// Full-matrix ring geometry
// Ring k = kth concentric rectangle from outside (k=0 outermost).
// Total rings = min(W,H)/2.
// Side 0: left  col k,       row (H-1-k) -> k         (bottom -> top)
// Side 1: top   row k,       col k        -> (W-1-k)   (left  -> right)
// Side 2: right col (W-1-k), row k        -> (H-1-k)   (top   -> bottom)
// Side 3: bottom row(H-1-k), col (W-1-k) -> (k+1)     (right -> left, shorter)
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
  uint8_t k = ring;
  uint16_t W = MATRIX_WIDTH, H = MATRIX_HEIGHT;
  switch (side) {
    case 0: return SP_LED(H - 1 - k - step, k);
    case 1: return SP_LED(k, k + step);
    case 2: return SP_LED(k + step, W - 1 - k);
    case 3: return SP_LED(H - 1 - k, W - 1 - k - step);
    default: return 0;
  }
}

// FULL-MATRIX STATE MACHINE
enum SP_State { SP_RUNNING, SP_HOLD };
static SP_State  sp_state      = SP_RUNNING;
static int16_t   sp_ring       = -1;
static int16_t   sp_side       = 0;
static int16_t   sp_step       = 0;
static bool      sp_reverse    = false;
static uint32_t  sp_lastUpdate = 0;
static uint32_t  sp_holdStart  = 0;

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

// updateMatrixBoxes -- full-matrix concentric ring fill.
// Reset with: sp_ring = -1;
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

// Quad ring geometry (works for any QUAD_WIDTH x QUAD_HEIGHT)
// Ring k = kth concentric rectangle from outside (k=0 outermost).
// Total rings = min(QW,QH)/2.
// Side 0: left  col k,         row (QH-1-k) -> k          (bottom -> top)
// Side 1: top   row k,         col k         -> (QW-1-k)   (left  -> right)
// Side 2: right col (QW-1-k),  row k         -> (QH-1-k)   (top   -> bottom)
// Side 3: bottom row (QH-1-k), col (QW-1-k) -> (k+1)      (right -> left, shorter)
static const uint8_t QUAD_RINGS =
    (uint8_t)((QUAD_WIDTH < QUAD_HEIGHT) ? QUAD_WIDTH / 2 : QUAD_HEIGHT / 2);

static uint16_t _sp8_sideLen(uint8_t ring, uint8_t side) {
  switch (side) {
    case 0: case 2: return (uint16_t)(QUAD_HEIGHT - 2 * ring);
    case 1:         return (uint16_t)(QUAD_WIDTH  - 2 * ring);
    case 3:         return (uint16_t)(QUAD_WIDTH  - 2 * ring - 1);
    default:        return 0;
  }
}

static uint16_t _sp8_getLED(uint8_t q, uint8_t ring, uint8_t side, uint16_t step) {
  uint16_t qr = _qRowOff(q), qc = _qColOff(q);
  uint8_t k = ring;
  uint16_t QW = QUAD_WIDTH, QH = QUAD_HEIGHT;
  switch (side) {
    case 0: return SP_LED(qr + QH - 1 - k - step, qc + k);
    case 1: return SP_LED(qr + k, qc + k + step);
    case 2: return SP_LED(qr + k + step, qc + QW - 1 - k);
    case 3: return SP_LED(qr + QH - 1 - k, qc + QW - 1 - k - step);
    default: return 0;
  }
}

// SP8Q_TOTAL_TICKS -- total LEDs in one quad ring fill, computed from dimensions.
// Formula: QUAD_RINGS * (2*QH + 2*QW - 1) - 4 * QUAD_RINGS * (QUAD_RINGS - 1)
// Verified: 8x8 quad  -> QUAD_RINGS=4 -> 4*(31) - 4*4*3 = 124-48 = 76 (correct)
//           16x8 quad -> QUAD_RINGS=4 -> 4*(47) - 48 = 188-48 = 140
//           4x4 quad  -> QUAD_RINGS=2 -> 2*(15) - 4*2*1 = 30-8 = 22
#define SP8Q_TOTAL_TICKS \
  ((int16_t)(QUAD_RINGS * (2*QUAD_HEIGHT + 2*QUAD_WIDTH - 1) - \
             4 * QUAD_RINGS * (QUAD_RINGS - 1)))

// ALL-QUADS STATE MACHINE (updateMatrix_4quad_Boxes -- all 4 quads, same colour)
enum SP8_State { SP8_RUNNING, SP8_HOLD };
static SP8_State sp8_state      = SP8_RUNNING;
static int16_t   sp8_ring       = -1;
static int16_t   sp8_side       = 0;
static int16_t   sp8_step       = 0;
static bool      sp8_reverse    = false;
static uint32_t  sp8_lastUpdate = 0;
static uint32_t  sp8_holdStart  = 0;

static uint32_t  sp8q_intervalMs = 80;
static uint32_t  sp8q_holdMs     = 200;

static void _sp8_init(bool reverse) {
  for (uint8_t q = 0; q < NUM_QUADS; q++) {
    uint16_t qr = _qRowOff(q), qc = _qColOff(q);
    for (uint16_t r = 0; r < QUAD_HEIGHT; r++)
      for (uint16_t c = 0; c < QUAD_WIDTH; c++)
        leds[SP_LED(qr + r, qc + c)] = CRGB::Black;
  }
  FastLED.show();
  sp8_reverse = reverse;
  if (!reverse) { sp8_ring = 0; sp8_side = 0; sp8_step = 0; }
  else {
    sp8_ring = QUAD_RINGS - 1;
    sp8_side = 3;
    sp8_step = (int16_t)_sp8_sideLen((uint8_t)(QUAD_RINGS - 1), 3) - 1;
  }
  sp8_lastUpdate = millis();
  sp8_state = SP8_RUNNING;
}

// updateMatrix_4quad_Boxes -- fills ALL 4 quads simultaneously, same colour.
// Works for any matrix size -- quads are always QUAD_WIDTH x QUAD_HEIGHT.
// Reset with: sp8_ring = -1;
static void updateMatrix_4quad_Boxes(CRGB color, uint32_t intervalMs,
                                  uint32_t holdMs, bool reverse = false) {
  sp8q_intervalMs = intervalMs;
  sp8q_holdMs     = holdMs;
  if (sp8_ring == -1) { _sp8_init(reverse); return; }
  uint32_t now = millis();
  if (sp8_state == SP8_HOLD) {
    if (now - sp8_holdStart < sp8q_holdMs) return;
    _sp8_init(reverse); return;
  }
  if (now - sp8_lastUpdate < sp8q_intervalMs) return;
  sp8_lastUpdate = now;
  for (uint8_t q = 0; q < NUM_QUADS; q++)
    leds[_sp8_getLED(q, (uint8_t)sp8_ring, (uint8_t)sp8_side, (uint16_t)sp8_step)] = color;
  FastLED.show();
  if (!reverse) {
    sp8_step++;
    if (sp8_step >= (int16_t)_sp8_sideLen((uint8_t)sp8_ring, (uint8_t)sp8_side)) {
      sp8_step = 0; sp8_side++;
      if (sp8_side > 3) {
        sp8_side = 0; sp8_ring++;
        if (sp8_ring >= (int16_t)QUAD_RINGS) { sp8_holdStart = now; sp8_state = SP8_HOLD; }
      }
    }
  } else {
    sp8_step--;
    if (sp8_step < 0) {
      sp8_side--;
      if (sp8_side < 0) {
        sp8_ring--;
        if (sp8_ring < 0) { sp8_holdStart = now; sp8_state = SP8_HOLD; }
        else { sp8_side = 3; sp8_step = (int16_t)_sp8_sideLen((uint8_t)sp8_ring, 3) - 1; }
      } else { sp8_step = (int16_t)_sp8_sideLen((uint8_t)sp8_ring, (uint8_t)sp8_side) - 1; }
    }
  }
}

// PER-QUAD STATE MACHINE (shared counter, zero drift)
// All 4 quads share ONE counter and ONE phase.
// Forward quads read counter 0 -> SP8Q_TOTAL_TICKS-1;
// reverse quads read it backward.
//
// HOW TO USE:
//  1. sp8q_tick() runs automatically in loop() -- nothing extra needed.
//  2. Call updateMatrix_4quad_BoxesQ() for each active quad.
//  3. Call FastLED.show() ONCE after all quad calls.
//
// Example:
//   FROM(0,0,0) {
//     updateMatrix_4quad_BoxesQ(1, 1, CRGB::Blue, 80, 200, false);  // Top-Left
//     updateMatrix_4quad_BoxesQ(2, 1, CRGB::Red,  80, 200, false);  // Top-Right
//     updateMatrix_4quad_BoxesQ(1, 2, CRGB::Red,  80, 200, true);   // Bottom-Left
//     updateMatrix_4quad_BoxesQ(2, 2, CRGB::Blue, 80, 200, true);   // Bottom-Right
//     FastLED.show();
//   }

enum SP8Q_Phase { SP8Q_RUNNING, SP8Q_HOLD };
static SP8Q_Phase sp8q_phase    = SP8Q_RUNNING;
static int16_t    sp8q_counter  = -1;
static bool       sp8q_ticked   = false;
static bool       sp8q_newCycle = false;
static uint32_t   sp8q_tickLast  = 0;
static uint32_t   sp8q_holdStart = 0;

static void sp8q_tick(uint32_t intervalMs, uint32_t holdMs) {
  sp8q_ticked = false; sp8q_newCycle = false;
  uint32_t now = millis();
  if (sp8q_counter == -1) {
    sp8q_counter = 0; sp8q_ticked = true; sp8q_newCycle = true;
    sp8q_tickLast = now; return;
  }
  if (sp8q_phase == SP8Q_HOLD) {
    if (now - sp8q_holdStart >= holdMs) {
      sp8q_phase = SP8Q_RUNNING; sp8q_counter = 0;
      sp8q_ticked = true; sp8q_newCycle = true; sp8q_tickLast = now;
    }
    return;
  }
  if (now - sp8q_tickLast >= intervalMs) {
    sp8q_tickLast = now; sp8q_counter++;
    if (sp8q_counter >= SP8Q_TOTAL_TICKS) {
      sp8q_phase = SP8Q_HOLD; sp8q_holdStart = now; sp8q_ticked = false;
    } else { sp8q_ticked = true; }
  }
}

static void sp8q_reset() {
  sp8q_phase = SP8Q_RUNNING; sp8q_counter = -1;
  sp8q_ticked = false; sp8q_newCycle = false;
  sp8q_tickLast = millis(); sp8q_holdStart = 0;
}

// _sp8q_indexToPos -- flat counter [0, SP8Q_TOTAL_TICKS-1] -> (ring, side, step)
static void _sp8q_indexToPos(int16_t idx,
                              uint8_t &ring, uint8_t &side, uint16_t &step) {
  int16_t rem = idx;
  for (ring = 0; ring < QUAD_RINGS; ring++) {
    for (side = 0; side < 4; side++) {
      uint16_t sl = _sp8_sideLen(ring, side);
      if (rem < (int16_t)sl) { step = (uint16_t)rem; return; }
      rem -= (int16_t)sl;
    }
  }
  ring = QUAD_RINGS - 1; side = 3; step = 0;
}

// updateMatrix_4quad_BoxesQ -- independent per-quad fill.
// Quads are QUAD_WIDTH x QUAD_HEIGHT (always exactly 4 quads).
//
// Parameters:
//   qCol       -- 1 = left quad    2 = right quad
//   qRow       -- 1 = top quad     2 = bottom quad
//   color      -- CRGB fill colour
//   intervalMs -- ms between each LED step
//   holdMs     -- ms to hold fully-lit before restarting
//   reverse    -- false = outside->in   true = inside->out
//
// Pass the same intervalMs/holdMs to every quad call in one block.
static void updateMatrix_4quad_BoxesQ(uint8_t qCol, uint8_t qRow,
                                   CRGB color,
                                   uint32_t intervalMs, uint32_t holdMs,
                                   bool reverse = false) {
  sp8q_intervalMs = intervalMs;
  sp8q_holdMs     = holdMs;
  if (qCol < 1 || qCol > 2) return;
  if (qRow < 1 || qRow > 2) return;
  uint8_t q = _qIdx(qCol, qRow);
  if (sp8q_counter < 0) return;

  if (sp8q_newCycle) {
    uint16_t qr = _qRowOff(q), qc = _qColOff(q);
    for (uint16_t r = 0; r < QUAD_HEIGHT; r++)
      for (uint16_t c = 0; c < QUAD_WIDTH; c++)
        leds[SP_LED(qr + r, qc + c)] = CRGB::Black;
  }
  if (!sp8q_ticked) return;

  int16_t idx = reverse ? (SP8Q_TOTAL_TICKS - 1) - sp8q_counter : sp8q_counter;
  uint8_t ring, side; uint16_t step;
  _sp8q_indexToPos(idx, ring, side, step);
  leds[_sp8_getLED(q, ring, side, step)] = color;
}

/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/