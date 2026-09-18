#pragma once
#include <FastLED.h>
#include "MatrixConfig.h"

/*
 * array_functions.h  (state machine edition)
 *
 * The original array_functions.h (adapted from the FastLED Mapping Demo
 * by Jason Coon / Evil Genius Labs LLC -- https://github.com/jasoncoon/led-mapper)
 * had 12 one-shot functions (clockwisePalette, outwardPalette,
 * northPalette, etc.) that all shared the same core idea: for every LED,
 * sample a palette at an index built from a global `offset` plus that
 * LED's precomputed angle/radius/x/y coordinate, then repaint the whole
 * strip. They relied on globals (currentPalette, offset, speed) and a
 * beat8(speed) call in loop() to animate, and never called
 * FastLED.show() themselves.
 *
 * This version folds all 12 patterns into ONE non-blocking state
 * machine class, CD77_ArrayPaletteMotion, matching the Update()/runID
 * pattern used everywhere else in this sketch:
 *   - "direction" (a PaletteMotionDirection enum value) picks which of
 *     the 12 original patterns to use, in place of 12 separate functions
 *   - the animation offset is computed with FastLED's own beat8(bpm),
 *     same as the original functions did -- beat8() derives its value
 *     directly from elapsed real time (millis()), so it's always
 *     perfectly smooth and phase-accurate no matter how often it's
 *     called or how much jitter there is between calls. (An earlier
 *     version of this file used a manually-incremented step counter
 *     instead, which looked visibly choppier than the original --
 *     beat8() is what actually gives the smooth rotation.)
 *   - updateIntervalMs still throttles how often the strip actually
 *     REDRAWS (a plain non-blocking millis() gate, same pattern as
 *     every other effect class here) -- but because beat8() is
 *     time-based rather than step-based, the motion stays smooth and
 *     un-jittery regardless of that redraw rate.
 *   - FastLED.show() is called internally, same as every other effect
 *     class in this sketch
 *   - runID (and a direction change) forces a clean restart, same
 *     pattern as CD77_RowFill etc.
 *
 * This operates on the coordinate arrays for the WHOLE 4-jar strip
 * (coordsX/coordsY/angles/radii, each sized NUM_LEDS), not per-jar --
 * these coordinates describe one continuous physical layout across all
 * 4 jars, so there's a single "AllJars" instance, no per-jar variant.
 *
 * ── USAGE IN Performance() ───────────────────────────────────────────
 *   BETWEEN(START(0,0,0), TO(0,0,3)) {
 *     paletteMotion.Update(RainbowColors_p, PM_CLOCKWISE, 20, 20, gPerformanceRunID);
 *   }
 *
 * ── Update() PARAMETERS ────────────────────────────────────────────────
 *   palette          CRGBPalette16          Palette to sample from (e.g. RainbowColors_p)
 *   direction        PaletteMotionDirection Which of the 12 patterns to draw (see enum below)
 *   updateIntervalMs unsigned long          Non-blocking delay between REDRAWS, ms
 *                                            (keep this low, 15-30ms, for a smooth display --
 *                                            it's a redraw-rate throttle, not a speed control)
 *   speedBPM         uint8_t                Rotation speed, in "beats per minute" (passed
 *                                            straight to FastLED's beat8()) -- e.g. 20 BPM
 *                                            means one full palette cycle every 3 seconds.
 *                                            Bigger = faster motion.
 *   runID            unsigned long          Changes once per lap -- forces a clean restart
 *                                            even with identical palette/direction
 */

// ---------- Custom palettes (from Chemdoc77) ----------
CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
CRGBPalette16 gb10_p      = CRGBPalette16(CRGB::Navy, CRGB::Navy, CRGB::Yellow, CRGB::Black);
CRGBPalette16 gb9_p       = CRGBPalette16(CRGB::Red, CRGB::Red, CRGB::Blue, CRGB::Black);
CRGBPalette16 gb6_p       = CRGBPalette16(CRGB::DarkRed, CRGB::DarkBlue, CRGB::DarkRed, CRGB::DarkBlue);
CRGBPalette16 gb11_p       = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::DarkRed, CRGB::Gray);
// ---------- Which of the 12 original patterns to draw ----------
enum PaletteMotionDirection {
  PM_CLOCKWISE,
  PM_COUNTER_CLOCKWISE,
  PM_OUTWARD,
  PM_INWARD,
  PM_NORTH,
  PM_NORTHEAST,
  PM_EAST,
  PM_SOUTHEAST,
  PM_SOUTH,
  PM_SOUTHWEST,
  PM_WEST,
  PM_NORTHWEST
};

class CD77_ArrayPaletteMotion {
public:
  CD77_ArrayPaletteMotion(CRGB* leds, uint16_t numLeds,
                           const byte* coordsX, const byte* coordsY,
                           const byte* angles, const byte* radii)
    : _leds(leds), _numLeds(numLeds),
      _coordsX(coordsX), _coordsY(coordsY), _angles(angles), _radii(radii) {}

  bool Update(CRGBPalette16 palette, PaletteMotionDirection direction,
              unsigned long updateIntervalMs, uint8_t speedBPM, unsigned long runID) {
    unsigned long now = millis();

    if (_firstCall || direction != _direction || runID != _runID) {
      _firstCall  = false;
      _direction  = direction;
      _runID      = runID;
      _lastUpdate = now;
    }

    if (now - _lastUpdate < updateIntervalMs) {
      return false; // not time to redraw yet
    }
    _lastUpdate = now;

    // beat8() derives its value from elapsed real time, not from how
    // many times we've called Update() -- this is what makes the
    // rotation smooth and consistent regardless of redraw timing.
    uint8_t offset = beat8(speedBPM);

    for (uint16_t i = 0; i < _numLeds; i++) {
      uint8_t index;
      switch (direction) {
        case PM_CLOCKWISE:         index = (uint8_t)(offset + _angles[i]); break;
        case PM_COUNTER_CLOCKWISE: index = (uint8_t)(offset - _angles[i]); break;
        case PM_OUTWARD:           index = (uint8_t)(offset - _radii[i]);  break;
        case PM_INWARD:            index = (uint8_t)(offset + _radii[i]);  break;
        case PM_NORTH:             index = (uint8_t)(offset - _coordsY[i]); break;
        case PM_NORTHEAST:         index = (uint8_t)(offset - (_coordsX[i] + _coordsY[i])); break;
        case PM_EAST:              index = (uint8_t)(offset - _coordsX[i]); break;
        case PM_SOUTHEAST:         index = (uint8_t)(offset - _coordsX[i] + _coordsY[i]); break;
        case PM_SOUTH:             index = (uint8_t)(offset + _coordsY[i]); break;
        case PM_SOUTHWEST:         index = (uint8_t)(offset + _coordsX[i] + _coordsY[i]); break;
        case PM_WEST:              index = (uint8_t)(offset + _coordsX[i]); break;
        case PM_NORTHWEST:         index = (uint8_t)(offset + _coordsX[i] - _coordsY[i]); break;
        default:                   index = offset; break;
      }
      _leds[i] = ColorFromPalette(palette, index);
    }
    FastLED.show();

    return true;
  }

private:
  CRGB*       _leds;
  uint16_t    _numLeds;
  const byte* _coordsX;
  const byte* _coordsY;
  const byte* _angles;
  const byte* _radii;

  PaletteMotionDirection _direction  = PM_CLOCKWISE;
  unsigned long          _runID      = 0;
  bool                   _firstCall  = true;
  unsigned long          _lastUpdate = 0;
};

// ---------- Effect instance (whole 4-jar strip) ----------
// `leds` and the coordsX/coordsY/angles/radii coordinate arrays are all
// defined in glass_jar_4jars.cpp (the coordinate arrays describe one
// continuous physical layout across all 4 jars, so there's a single
// "AllJars" instance, no per-jar variant -- see file header above).
extern CRGB leds[NUM_LEDS];
extern byte coordsX[NUM_LEDS];
extern byte coordsY[NUM_LEDS];
extern byte angles[NUM_LEDS];
extern byte radii[NUM_LEDS];

CD77_ArrayPaletteMotion paletteMotion(leds, NUM_LEDS, coordsX, coordsY, angles, radii);
