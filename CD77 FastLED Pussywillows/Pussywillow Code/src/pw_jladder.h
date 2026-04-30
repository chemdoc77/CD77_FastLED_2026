// pw_jladder.h
// Exponentially-accelerating row-fill effects for a WS2812b LED matrix.
// Each is a fully independent, non-blocking state machine, following the
// same conventions as pussywillow_functions.h.
//
// ── EFFECTS ──────────────────────────────────────────────────────────────────
//  updateJladder  – solid colour, speed increases exponentially each loop cycle
//  updateJladderP – palette colour, speed increases exponentially each loop cycle
//
// ── ACCELERATION BEHAVIOUR ────────────────────────────────────────────────────
//  Within a single fill (bottom→top), every row steps at the SAME interval —
//  that interval is whatever the current cycle's speed has reached.
//  After each complete fill + hold, the starting interval for the NEXT fill
//  is reduced by accelPct%, so successive loops arrive faster and faster.
//  The interval floors at JLADDER_MIN_INTERVAL_MS so it never becomes invisible.
//
//  Example with startMs=400, accelPct=30:
//    Loop 1: all rows step at 400 ms
//    Loop 2: all rows step at 280 ms
//    Loop 3: all rows step at 196 ms
//    Loop 4: all rows step at 137 ms  … and so on until the floor
//
// ── HOW TO USE ────────────────────────────────────────────────────────────────
//  1. #include "pw_jladder.h" in the .ino AFTER pussywillow_functions.h.
//  2. Add these two lines to resetPussywillowEffects() in pussywillow_functions.h:
//       pw_jladderRow  = -1;   // Jladder  (solid)
//       pw_jladderPRow = -1;   // JladderP (palette)
//
//  In Time_performance.h / Performance():
//    BETWEEN(START(0,0,0), TO(0,0,8))  { updateJladder(CRGB::Red, 400, 30, 1500, false); }
//    BETWEEN(START(0,0,8), TO(0,0,16)) { updateJladderP(RainbowColors_p, 400, 30, 1500, false, 10); }
//
//  In Time_performance.h comments block:
//    //updateJladder(CRGB::Red, startMs, accelPct, holdMs, reverse);
//    //   colour, startMs per row, acceleration %(1-90), holdMs, direction
//    //   false = bottom→top   true = top→bottom   [calls FastLED.show() internally]
//    //   Speed is constant within each fill; each successive fill is accelPct% faster.
//    //updateJladderP(RainbowColors_p, startMs, accelPct, holdMs, reverse, paletteSpeed);
//    //   palette, startMs, accelPct %(1-90), holdMs, direction, paletteSpeed(1-20)
//    //   Reset sentinel: pw_jladderRow = -1;  pw_jladderPRow = -1;
//
// ── PARAMETERS ───────────────────────────────────────────────────────────────
//  startMs      – interval (ms) used for the very FIRST fill cycle (e.g. 400)
//  accelPct     – % to reduce the interval at the START of each new fill cycle (1–90)
//                 larger number = faster acceleration across loops:
//                 10 = very gentle   20 = gentle   30 = moderate   45 = aggressive   70 = near-instant
//  holdMs       – ms to pause at the top before starting the next (faster) fill
//  reverse      – false = bottom → top   true = top → bottom
//  paletteSpeed – (JladderP only) palette index advance per row step (1–20)
// ─────────────────────────────────────────────────────────────────────────────

#pragma once
#include <FastLED.h>

// Minimum interval floor so the effect never becomes imperceptibly fast
#define JLADDER_MIN_INTERVAL_MS  15

// ═════════════════════════════════════════════════════════════════════════════
//  JLADDER  –  solid colour, per-loop exponentially accelerating row fill
//
//  All rows in a single fill travel at the same speed (the current cycle interval).
//  After each full fill + hold, the interval for the next fill is reduced by
//  accelPct%, so each successive loop is uniformly faster than the last.
//
//  Call signature:
//    updateJladder(color, startMs, accelPct, holdMs, reverse)
//      color     – CRGB solid colour for every row
//      startMs   – row interval for the first fill cycle in milliseconds (e.g. 400)
//      accelPct  – % reduction applied to the interval at the start of each new cycle (1–90)
//                  larger = faster acceleration across successive loops
//      holdMs    – ms to hold when the matrix is full before starting the next cycle
//      reverse   – false = bottom→top   true = top→bottom
// ═════════════════════════════════════════════════════════════════════════════

enum PW_JladderState { PW_JLADDER_RUNNING, PW_JLADDER_HOLD };

static PW_JladderState pw_jladderState        = PW_JLADDER_RUNNING;
static int8_t          pw_jladderRow          = -1;     // -1 = not yet initialised
static uint32_t        pw_jladderLastUpdate   = 0;
static uint32_t        pw_jladderHoldStart    = 0;
static bool            pw_jladderReverse      = false;
static float           pw_jladderCycleInterval = 400.0f; // interval for the current cycle
static uint32_t        pw_jladderStartMs        = 400;    // saved on every call; used by resetJladder()

// Call from RestartPerformance() — no argument needed; startMs is saved automatically.
static void resetJladder() {
  pw_jladderRow           = -1;
  pw_jladderCycleInterval = (float)pw_jladderStartMs;
}

static void _pw_initJladder(uint32_t startMs, bool reverse) {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_jladderReverse       = reverse;
  pw_jladderRow           = reverse ? 0 : pw_rows - 1;
  pw_jladderCycleInterval = (float)startMs;   // first cycle uses startMs
  pw_jladderLastUpdate    = millis();
  pw_jladderState         = PW_JLADDER_RUNNING;
}

// Call every loop() iteration to run the Jladder effect
static void updateJladder(CRGB color, uint32_t startMs, uint8_t accelPct,
                           uint32_t holdMs, bool reverse = false) {
  pw_jladderStartMs = startMs;                       // save so resetJladder() can use it
  if (pw_jladderRow == -1) _pw_initJladder(startMs, reverse);

  uint32_t now = millis();

  if (pw_jladderState == PW_JLADDER_RUNNING) {
    // All rows in this cycle use the same (floored) interval
    uint32_t intervalMs = (uint32_t)max(pw_jladderCycleInterval, (float)JLADDER_MIN_INTERVAL_MS);
    if (now - pw_jladderLastUpdate < intervalMs) return;
    pw_jladderLastUpdate = now;

    // Light the current row
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[pw_jladderRow][c]] = color;
    FastLED.show();

    bool done = pw_jladderReverse ? (pw_jladderRow == pw_rows - 1)
                                  : (pw_jladderRow == 0);
    if (done) {
      pw_jladderHoldStart = now;
      pw_jladderState     = PW_JLADDER_HOLD;
    } else {
      pw_jladderRow += pw_jladderReverse ? 1 : -1;
    }

  } else {  // PW_JLADDER_HOLD
    if (now - pw_jladderHoldStart < holdMs) return;

    // Reduce the cycle interval by accelPct% for the next fill
    float multiplier = (100.0f - (float)accelPct) / 100.0f;
    pw_jladderCycleInterval = max(pw_jladderCycleInterval * multiplier,
                                  (float)JLADDER_MIN_INTERVAL_MS);

    // Clear and restart with the new (faster) interval
    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    FastLED.show();
    pw_jladderRow        = pw_jladderReverse ? 0 : pw_rows - 1;
    pw_jladderLastUpdate = millis();
    pw_jladderState      = PW_JLADDER_RUNNING;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  JLADDERP  –  palette colour, per-loop exponentially accelerating row fill
//
//  Identical acceleration behaviour to updateJladder() — speed is constant
//  within each fill, then reduced by accelPct% before the next fill begins.
//  Each row is coloured from a CRGBPalette16; the palette offset advances by
//  paletteSpeed each row step so every row gets a different colour.
//
//  Call signature:
//    updateJladderP(palette, startMs, accelPct, holdMs, reverse, paletteSpeed)
//      palette      – CRGBPalette16 to colour each row
//      startMs      – row interval for the first fill cycle in milliseconds (e.g. 400)
//      accelPct     – % reduction applied to the interval at the start of each new cycle (1–90)
//                     larger = faster acceleration across successive loops
//      holdMs       – ms to hold when the matrix is full before starting the next cycle
//      reverse      – false = bottom→top   true = top→bottom
//      paletteSpeed – how much the palette index advances each row step (1–20)
// ═════════════════════════════════════════════════════════════════════════════

enum PW_JladderPState { PW_JLADDERP_RUNNING, PW_JLADDERP_HOLD };

static PW_JladderPState pw_jladderPState         = PW_JLADDERP_RUNNING;
static int8_t           pw_jladderPRow           = -1;     // -1 = not yet initialised
static uint32_t         pw_jladderPLastUpdate    = 0;
static uint32_t         pw_jladderPHoldStart     = 0;
static bool             pw_jladderPReverse       = false;
static float            pw_jladderPCycleInterval = 400.0f; // interval for the current cycle
static uint8_t          pw_jladderPOffset        = 0;      // cycling palette offset
static uint32_t         pw_jladderPStartMs       = 400;    // saved on every call; used by resetJladderP()

// Call from RestartPerformance() — no argument needed; startMs is saved automatically.
static void resetJladderP() {
  pw_jladderPRow           = -1;
  pw_jladderPCycleInterval = (float)pw_jladderPStartMs;
  pw_jladderPOffset        = 0;
}

static void _pw_initJladderP(uint32_t startMs, bool reverse) {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_jladderPReverse        = reverse;
  pw_jladderPRow            = reverse ? 0 : pw_rows - 1;
  pw_jladderPCycleInterval  = (float)startMs;   // first cycle uses startMs
  pw_jladderPLastUpdate     = millis();
  pw_jladderPState          = PW_JLADDERP_RUNNING;
}

// Call every loop() iteration to run the JladderP effect
static void updateJladderP(CRGBPalette16 palette, uint32_t startMs, uint8_t accelPct,
                            uint32_t holdMs, bool reverse = false,
                            uint8_t paletteSpeed = 10) {
  pw_jladderPStartMs = startMs;                      // save so resetJladderP() can use it
  if (pw_jladderPRow == -1) _pw_initJladderP(startMs, reverse);

  uint32_t now = millis();

  if (pw_jladderPState == PW_JLADDERP_RUNNING) {
    // All rows in this cycle use the same (floored) interval
    uint32_t intervalMs = (uint32_t)max(pw_jladderPCycleInterval, (float)JLADDER_MIN_INTERVAL_MS);
    if (now - pw_jladderPLastUpdate < intervalMs) return;
    pw_jladderPLastUpdate = now;

    // Pick colour from palette and advance offset for the next row
    CRGB color = ColorFromPalette(palette, pw_jladderPOffset, 255, LINEARBLEND);
    pw_jladderPOffset += paletteSpeed;

    // Light the current row
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[pw_jladderPRow][c]] = color;
    FastLED.show();

    bool done = pw_jladderPReverse ? (pw_jladderPRow == pw_rows - 1)
                                   : (pw_jladderPRow == 0);
    if (done) {
      pw_jladderPHoldStart = now;
      pw_jladderPState     = PW_JLADDERP_HOLD;
    } else {
      pw_jladderPRow += pw_jladderPReverse ? 1 : -1;
    }

  } else {  // PW_JLADDERP_HOLD
    if (now - pw_jladderPHoldStart < holdMs) return;

    // Reduce the cycle interval by accelPct% for the next fill
    float multiplier = (100.0f - (float)accelPct) / 100.0f;
    pw_jladderPCycleInterval = max(pw_jladderPCycleInterval * multiplier,
                                   (float)JLADDER_MIN_INTERVAL_MS);

    // Clear and restart with the new (faster) interval
    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    FastLED.show();
    pw_jladderPRow        = pw_jladderPReverse ? 0 : pw_rows - 1;
    pw_jladderPLastUpdate = millis();
    pw_jladderPState      = PW_JLADDERP_RUNNING;
  }
}




/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/