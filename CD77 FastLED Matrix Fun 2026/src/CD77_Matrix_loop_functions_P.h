// Matrix_loop_functions_P.h
// Palette-coloured box-fill effects.  Companion to Matrix_loop_functions.h.
//
// Functions:
//   updateMatrixBoxesP      -- palette full-matrix fill
//   updateMatrix_4quad_BoxesP   -- palette all-4-quads simultaneous fill
//   updateMatrix_4quad_BoxesQP  -- palette independent per-quad fill
//
// Quads are always QUAD_WIDTH x QUAD_HEIGHT = (MATRIX_WIDTH/2) x (MATRIX_HEIGHT/2).
// Addressed as (qCol, qRow): qCol in {1,2}, qRow in {1,2}, (1,1) = top-left.
//
// How palette colouring works:
//   Each LED's colour is looked up from the palette using its angular position
//   (theta) around the centre of the quad, mapped 0-255.  A paletteOffset
//   counter drifts forward by paletteSpeed each step for a flowing effect.
//
// paletteSpeed: 0=static  3=gentle drift  10=fast  20=psychedelic
//
// Reset sentinels:
//   updateMatrixBoxesP:    sp_ring  = -1;
//   updateMatrix_4quad_BoxesP: sp8_ring = -1;
//   updateMatrix_4quad_BoxesQP: resets automatically each cycle

#pragma once
#include "CD77_Matrix_loop_functions.h"

// Local row/col from (ring, side, step) -- full matrix
static void _spP_getRC(uint8_t ring, uint8_t side, uint16_t step,
                        uint16_t &row, uint16_t &col) {
  uint8_t k = ring;
  uint16_t W = MATRIX_WIDTH, H = MATRIX_HEIGHT;
  switch (side) {
    case 0:  row = H - 1 - k - step;  col = k;             break;
    case 1:  row = k;                  col = k + step;       break;
    case 2:  row = k + step;           col = W - 1 - k;      break;
    default: row = H - 1 - k;         col = W - 1 - k - step; break;
  }
}

// Local row/col from (ring, side, step) -- one quad (QUAD_WIDTH x QUAD_HEIGHT)
static void _sp8P_getRC(uint8_t ring, uint8_t side, uint16_t step,
                         uint16_t &row, uint16_t &col) {
  uint8_t k = ring;
  uint16_t QW = QUAD_WIDTH, QH = QUAD_HEIGHT;
  switch (side) {
    case 0:  row = QH - 1 - k - step;  col = k;               break;
    case 1:  row = k;                   col = k + step;         break;
    case 2:  row = k + step;            col = QW - 1 - k;       break;
    default: row = QH - 1 - k;         col = QW - 1 - k - step; break;
  }
}

// Palette index -- full matrix, centre at ((H-1)/2, (W-1)/2)
static inline uint8_t _spP_palIdx(uint16_t row, uint16_t col, uint8_t offset) {
  float cy = (MATRIX_HEIGHT - 1) * 0.5f;
  float cx = (MATRIX_WIDTH  - 1) * 0.5f;
  float theta = atan2f((float)row - cy, (float)col - cx);
  return (uint8_t)((theta + (float)PI) * (255.0f / (2.0f * (float)PI))) + offset;
}

// Palette index -- one quad, centre at ((QH-1)/2, (QW-1)/2)
static inline uint8_t _sp8P_palIdx(uint16_t localRow, uint16_t localCol, uint8_t offset) {
  float cy = (QUAD_HEIGHT - 1) * 0.5f;
  float cx = (QUAD_WIDTH  - 1) * 0.5f;
  float theta = atan2f((float)localRow - cy, (float)localCol - cx);
  return (uint8_t)((theta + (float)PI) * (255.0f / (2.0f * (float)PI))) + offset;
}

// P-specific init (fills with bgColor instead of black)
static void _spP_init(bool reverse, CRGB bgColor, uint8_t &palOffset) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i] = bgColor;
  FastLED.show();
  palOffset  = 0;
  sp_reverse = reverse;
  if (!reverse) { sp_ring = 0; sp_side = 0; sp_step = 0; }
  else {
    sp_ring = SP_NUM_RINGS - 1; sp_side = 3;
    sp_step = (int16_t)_sp_sideLen((uint8_t)(SP_NUM_RINGS - 1), 3) - 1;
  }
  sp_lastUpdate = millis();
  sp_state = SP_RUNNING;
}

static void _sp8P_init(bool reverse, CRGB bgColor, uint8_t &palOffset) {
  for (uint8_t q = 0; q < NUM_QUADS; q++) {
    uint16_t qr = _qRowOff(q), qc = _qColOff(q);
    for (uint16_t r = 0; r < QUAD_HEIGHT; r++)
      for (uint16_t c = 0; c < QUAD_WIDTH; c++)
        leds[SP_LED(qr + r, qc + c)] = bgColor;
  }
  FastLED.show();
  palOffset   = 0;
  sp8_reverse = reverse;
  if (!reverse) { sp8_ring = 0; sp8_side = 0; sp8_step = 0; }
  else {
    sp8_ring = QUAD_RINGS - 1; sp8_side = 3;
    sp8_step = (int16_t)_sp8_sideLen((uint8_t)(QUAD_RINGS - 1), 3) - 1;
  }
  sp8_lastUpdate = millis();
  sp8_state = SP8_RUNNING;
}

// Palette offset state variables
static uint8_t spp_palOffset   = 0;
static uint8_t sp8p_palOffset  = 0;
static uint8_t sp8qp_palOffset = 0;
static int16_t sp8qp_lastCtr   = -2;

// updateMatrixBoxesP -- palette full-matrix fill.
// Reset with: sp_ring = -1;
static void updateMatrixBoxesP(const CRGBPalette16 &palette, CRGB bgColor,
                                 uint32_t intervalMs, uint32_t holdMs,
                                 bool reverse = false, uint8_t paletteSpeed = 3) {
  if (sp_ring == -1) { _spP_init(reverse, bgColor, spp_palOffset); return; }
  uint32_t now = millis();
  if (sp_state == SP_HOLD) {
    if (now - sp_holdStart < holdMs) return;
    _spP_init(reverse, bgColor, spp_palOffset); return;
  }
  if (now - sp_lastUpdate < intervalMs) return;
  sp_lastUpdate = now;
  uint16_t row, col;
  _spP_getRC((uint8_t)sp_ring, (uint8_t)sp_side, (uint16_t)sp_step, row, col);
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

// updateMatrix_4quad_BoxesP -- palette all-4-quads simultaneous fill.
// Reset with: sp8_ring = -1;
static void updateMatrix_4quad_BoxesP(const CRGBPalette16 &palette, CRGB bgColor,
                                    uint32_t intervalMs, uint32_t holdMs,
                                    bool reverse = false, uint8_t paletteSpeed = 3) {
  sp8q_intervalMs = intervalMs;
  sp8q_holdMs     = holdMs;
  if (sp8_ring == -1) { _sp8P_init(reverse, bgColor, sp8p_palOffset); return; }
  uint32_t now = millis();
  if (sp8_state == SP8_HOLD) {
    if (now - sp8_holdStart < sp8q_holdMs) return;
    _sp8P_init(reverse, bgColor, sp8p_palOffset); return;
  }
  if (now - sp8_lastUpdate < sp8q_intervalMs) return;
  sp8_lastUpdate = now;
  uint16_t localRow, localCol;
  _sp8P_getRC((uint8_t)sp8_ring, (uint8_t)sp8_side, (uint16_t)sp8_step, localRow, localCol);
  uint8_t palIdx = _sp8P_palIdx(localRow, localCol, sp8p_palOffset);
  for (uint8_t q = 0; q < NUM_QUADS; q++)
    leds[_sp8_getLED(q, (uint8_t)sp8_ring, (uint8_t)sp8_side, (uint16_t)sp8_step)] =
        ColorFromPalette(palette, palIdx);
  FastLED.show();
  sp8p_palOffset += paletteSpeed;
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

// updateMatrix_4quad_BoxesQP -- palette independent per-quad fill.
// Shares sp8q counter with updateMatrix_4quad_BoxesQ.
// Resets automatically each cycle.
//
// Parameters:
//   qCol, qRow   -- quad address: qCol in {1,2}, qRow in {1,2}
//   palette      -- any CRGBPalette16 (can differ per quad)
//   intervalMs   -- ms between each LED step
//   holdMs       -- ms to hold fully-lit before restarting
//   reverse      -- false = outside->in   true = inside->out
//   paletteSpeed -- 0=static  3=gentle  10=fast  20=psychedelic
static void updateMatrix_4quad_BoxesQP(uint8_t qCol, uint8_t qRow,
                                    const CRGBPalette16 &palette,
                                    uint32_t intervalMs, uint32_t holdMs,
                                    bool reverse = false, uint8_t paletteSpeed = 3) {
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
    if (q == 0) sp8qp_palOffset = 0;
  }
  if (!sp8q_ticked) return;

  if (sp8q_counter != sp8qp_lastCtr) {
    sp8qp_palOffset += paletteSpeed;
    sp8qp_lastCtr    = sp8q_counter;
  }

  int16_t idx = reverse ? (SP8Q_TOTAL_TICKS - 1) - sp8q_counter : sp8q_counter;
  uint8_t ring, side; uint16_t step;
  _sp8q_indexToPos(idx, ring, side, step);

  uint16_t localRow, localCol;
  _sp8P_getRC(ring, side, step, localRow, localCol);
  uint8_t palIdx = _sp8P_palIdx(localRow, localCol, sp8qp_palOffset);
  leds[_sp8_getLED(q, ring, side, step)] = ColorFromPalette(palette, palIdx);
}


/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/