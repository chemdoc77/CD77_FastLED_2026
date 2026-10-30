#pragma once
/*
  CD77_palettecrossfade.h - moving palette pattern that smoothly crossfades palettes

  Based on Mark Kriegsman's PaletteCrossfade:
  https://gist.github.com/kriegsman/1494ecd36e1ba2ae0e22 (PaletteCrossfade.ino)
  where currentPalette is nudged toward targetPalette with
  nblendPaletteTowardPalette() while a colour pattern keeps moving.

  Non-blocking state machine version. The original globals (currentPalette,
  targetPalette, the 5 second palette picker, EVERY_N_MILLISECONDS) are now
  arguments of update() or constants below, so nothing outside this file has
  to track them. Nothing here ever calls delay().

  HOW IT WORKS
    * The class keeps its own currentPalette.
    * Every PALXFADE_BLEND_MS the currentPalette moves a little closer to the
      target palette you pass in (nblendPaletteTowardPalette). 'fade' sets how
      big that step is: bigger fade = faster crossfade.
    * Every PALXFADE_FRAME_MS the LEDs are painted from currentPalette using the
      direction you choose (the same PAL_ directions as CD77_palette.update(),
      from the LED maps in MapData.h), then FastLED.show() is called.
    * The pattern moves at 'speed' beats per minute, exactly like the speed of
      CD77_palette.update(). The position is remembered between calls, so changing
      speed or direction from one BETWEEN window to the next never makes the
      pattern jump.
    * Change the target palette from one call to the next and the colours crossfade
      smoothly while the pattern keeps moving. Keep passing the same palette and
      the crossfade finishes and the pattern just keeps moving.
    * First call of a show (or after the effect was not called for
      PALXFADE_RESTART_GAP_MS, e.g. after a blackout window) starts fresh: the
      current palette is set to the target, so the first picture is clean. Use
      startFrom(palette) before the first update() to crossfade IN from a
      palette of your choice instead.

  Usage (call every loop pass, inside a BETWEEN block):
    CD77_palettecrossfade.update(PAL_NORTH, hc5_p);                  // direction, target palette
    CD77_palettecrossfade.update(PAL_INWARD, IceColors_p, 3, 30);    // + fade, speed
    CD77_palettecrossfade.update(PAL_CLOCKWISE, hc2_p, 6, 18, 2);    // + repeat

  update() arguments:
    dir     : PAL_NORTH, PAL_NORTH_EAST, PAL_EAST, PAL_SOUTH_EAST, PAL_SOUTH,
              PAL_SOUTH_WEST, PAL_WEST, PAL_NORTH_WEST, PAL_CLOCKWISE,
              PAL_COUNTER_CLOCKWISE, PAL_INWARD, PAL_OUTWARD
    target  : the palette to crossfade toward (any CRGBPalette16, HeatColors_p,
              PartyColors_p, IceColors_p, hc1_p ... hc5_p, or your own)
    fade    : 1-12, how fast the colours blend into the new palette (default 6).
              1 = very slow (tens of seconds for the biggest colour change),
              6 = a few seconds, 12 = fastest. Most blends finish sooner than the
              worst case, because only the colours that differ have to move.
    speed   : pattern speed in beats per minute, same meaning as the speed of
              CD77_palette.update() (default 20). 0 = pattern stands still (the
              colours still crossfade).
    repeat  : how many times the palette repeats across the display, 1-8 (default 1)
*/
#include "Config.h"
#include "ropestuff.h"     // PaletteDir enum + MapData.h coordinates

// An effect not called for this long (ms) counts as stopped (its window ended);
// the next update() then starts fresh. Same idea as BLINK_RESTART_GAP_MS.
#define PALXFADE_RESTART_GAP_MS  100

// Fixed timing (these used to be update() arguments). Change them here if needed.
#define PALXFADE_BLEND_MS        10    // ms between crossfade steps
#define PALXFADE_FRAME_MS        20    // ms between animation frames (20 = 50 FPS)

class CD77_palettecrossfade_SM {
public:
  void reset() { _started = false; _phase = 0; }

  // Optional: call before the first update() of a window to crossfade in FROM
  // this palette instead of starting directly on the target palette.
  void startFrom(const CRGBPalette16& from) {
    uint32_t now = millis();
    _current    = from;
    _phase      = 0;
    _lastBlend  = now;
    _lastFrame  = now;
    _lastCall   = now;
    _started    = true;
  }

  void update(PaletteDir dir, const CRGBPalette16& target,
              uint8_t fade = 6, uint8_t speed = 20, uint8_t repeat = 1) {
    uint32_t now = millis();

    // Not called for a while: the window ended, so begin again from scratch.
    if (_started && (now - _lastCall) >= PALXFADE_RESTART_GAP_MS) _started = false;
    _lastCall = now;

    if (!_started) {
      _started   = true;
      _current   = target;                    // clean start: nothing to crossfade yet
      _phase     = 0;
      _lastBlend = now;
      _lastFrame = now - PALXFADE_FRAME_MS;   // draw the first frame immediately
    }

    // Blend tick: move currentPalette a little closer to the target palette
    if ((now - _lastBlend) >= PALXFADE_BLEND_MS) {
      _lastBlend = now;
      uint8_t f = (fade < 1) ? 1 : (fade > 12 ? 12 : fade);
      _target = target;   // FastLED's nblendPaletteTowardPalette() wants a non-const target
      nblendPaletteTowardPalette(_current, _target, f * 4);   // f*4 = 4..48 changes per tick
    }

    // Frame tick: move the pattern, paint, show
    uint32_t elapsed = now - _lastFrame;
    if (elapsed < PALXFADE_FRAME_MS) return;
    _lastFrame = now;

    // speed is beats per minute: one beat = one trip through the whole palette.
    // _phase counts 65536 units per beat, so it keeps its place from window to window.
    _phase += (uint32_t)(((uint64_t)speed * elapsed * 65536ULL) / 60000ULL);
    uint8_t offset = (uint8_t)(_phase >> 8);

    if (repeat < 1) repeat = 1;
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
      uint8_t idx = (uint8_t)(index(dir, offset, i) * repeat);
      leds[i] = ColorFromPalette(_current, idx, 255, LINEARBLEND);
    }
    FastLED.show();
  }

  const CRGBPalette16& currentPalette() const { return _current; }

private:
  CRGBPalette16 _current;
  CRGBPalette16 _target;     // non-const copy of the target palette for FastLED
  bool          _started   = false;
  uint32_t      _phase     = 0;     // pattern position, 65536 units per beat
  uint32_t      _lastBlend = 0;
  uint32_t      _lastFrame = 0;
  uint32_t      _lastCall  = 0;

  // Same direction maths as CD77_palette_SM in ropestuff.h
  static uint8_t index(PaletteDir d, uint8_t offset, uint16_t i) {
    switch (d) {
      case PAL_CLOCKWISE:         return offset + angles[i];
      case PAL_COUNTER_CLOCKWISE: return offset - angles[i];
      case PAL_OUTWARD:           return offset - radii[i];
      case PAL_INWARD:            return offset + radii[i];
      case PAL_NORTH:             return offset - coordsY[i];
      case PAL_NORTH_EAST:        return offset - (coordsX[i] + coordsY[i]);
      case PAL_EAST:              return offset - coordsX[i];
      case PAL_SOUTH_EAST:        return offset - coordsX[i] + coordsY[i];
      case PAL_SOUTH:             return offset + coordsY[i];
      case PAL_SOUTH_WEST:        return offset + coordsX[i] + coordsY[i];
      case PAL_WEST:              return offset + coordsX[i];
      case PAL_NORTH_WEST:        return offset + coordsX[i] - coordsY[i];
    }
    return offset;
  }
};

CD77_palettecrossfade_SM CD77_palettecrossfade;
