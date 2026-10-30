#pragma once
/*
   FastLED Mapping Demo: https://github.com/jasoncoon/led-mapper
   Copyright (C) 2022 Jason Coon, Evil Genius Labs LLC

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Palette-sweep effects (clockwise, north, inward, ...) as one non-blocking
   state machine. The old globals offset / speed / currentPalette are now
   arguments of update(), so nothing outside this file has to track them.

   Usage (call every loop pass):
     CD77_palette.update(PAL_NORTH, IceColors_p, 40);       // direction, palette, speed (beats/min)
     CD77_palette.update(PAL_INWARD, hc5_p, 30, 16);        // optional 4th arg: ms between frames
*/
#include "Config.h"
#include "MapData.h"

// palettes from Chemdoc77  OrangeRed
CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);

CRGBPalette16 hc1_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkGreen, CRGB::DarkRed, CRGB::Black);
CRGBPalette16 hc2_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkRed, CRGB::DarkRed, CRGB::DarkRed, CRGB::DarkRed);
CRGBPalette16 hc3_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen);
CRGBPalette16 hc4_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen);
CRGBPalette16 hc5_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::OrangeRed, CRGB::OrangeRed, CRGB::OrangeRed, CRGB::OrangeRed, CRGB::OrangeRed, CRGB::OrangeRed, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen);
CRGBPalette16 hc6_p = CRGBPalette16(CRGB::Black, CRGB::DarkGreen, CRGB::OrangeRed, CRGB::DarkViolet);
enum PaletteDir : uint8_t {
  PAL_CLOCKWISE, PAL_COUNTER_CLOCKWISE, PAL_OUTWARD, PAL_INWARD,
  PAL_NORTH, PAL_NORTH_EAST, PAL_EAST, PAL_SOUTH_EAST,
  PAL_SOUTH, PAL_SOUTH_WEST, PAL_WEST, PAL_NORTH_WEST
};

class CD77_palette_SM {
public:
  void reset() { _started = false; }

  void update(PaletteDir dir, const CRGBPalette16& pal, uint8_t speed, uint16_t frameMs = 16) {
    uint32_t now = millis();
    if (_started && (now - _lastFrame) < frameMs) return;
    _started   = true;
    _lastFrame = now;

    uint8_t offset = beat8(speed);
    for (uint16_t i = 0; i < NUM_LEDS; i++)
      leds[i] = ColorFromPalette(pal, index(dir, offset, i));
    FastLED.show();
  }

private:
  bool     _started   = false;
  uint32_t _lastFrame = 0;

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

// Blackout: replaces the old  FastLED.clear(); FastLED.show();  lines.
// Re-asserts black every frameMs so it is safe to call for a whole BETWEEN window.
class CD77_blackout_SM {
public:
  void reset() { _started = false; }
  void update(uint16_t frameMs = 50) {
    uint32_t now = millis();
    if (_started && (now - _lastFrame) < frameMs) return;
    _started   = true;
    _lastFrame = now;
    FastLED.clear();
    FastLED.show();
  }
private:
  bool     _started   = false;
  uint32_t _lastFrame = 0;
};

CD77_palette_SM   CD77_palette;
CD77_blackout_SM  CD77_blackout;
