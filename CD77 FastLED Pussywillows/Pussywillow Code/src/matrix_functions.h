#pragma once
#include <FastLED.h>



/* the following code is adapted from:
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
*/
// palettes from Chemdoc77




CRGBPalette16 gb10_p = CRGBPalette16(CRGB::Navy, CRGB::Navy, CRGB::Yellow, CRGB::Black);
CRGBPalette16 gb9_p = CRGBPalette16(CRGB::Red, CRGB::Yellow, CRGB::Red, CRGB::Yellow);
CRGBPalette16 gb6_p = CRGBPalette16(CRGB::DarkRed, CRGB::DarkBlue, CRGB::DarkRed, CRGB::Purple);

CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
CRGBPalette16 hc1_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkGreen, CRGB::DarkRed, CRGB::Black);
CRGBPalette16 hc2_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen,  CRGB::DarkGreen, CRGB::DarkGreen,  CRGB::DarkRed, CRGB::DarkRed, CRGB::DarkRed, CRGB::DarkRed);
CRGBPalette16 hc3_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen);
CRGBPalette16 hc4_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen);
CRGBPalette16 hc5_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkViolet, CRGB::OrangeRed, CRGB::OrangeRed, CRGB::OrangeRed , CRGB::OrangeRed, CRGB::OrangeRed, CRGB::OrangeRed, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen);
CRGBPalette16 HO1_p = CRGBPalette16(CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red , CRGB::White, CRGB::White, CRGB::White, CRGB::White , CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::White, CRGB::White, CRGB::White, CRGB::White);
CRGBPalette16 HO2_p = CRGBPalette16(CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red , CRGB::Black, CRGB::Black, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Black, CRGB::Black, CRGB::Purple, CRGB::Purple, CRGB::Purple, CRGB::Purple);
CRGBPalette16 HO3_p = CRGBPalette16(CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red , CRGB::Red, CRGB::White, CRGB::Red, CRGB::White , CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::White, CRGB::Blue, CRGB::White);

// Gradient palette "grass_precipitation_monthly_gp", originally from
// https://phillips.shef.ac.uk/pub/cpt-city/grass/precipitation_monthly
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

DEFINE_GRADIENT_PALETTE( grass_precipitation_monthly_gp ) {
    0,  42, 55, 45,
    0, 103, 18,  1,
   12, 255,255,  0,
   25,   0,255,  0,
   51,  22, 55,255,
   76,   0,  0,255,
  102,  42,  0,255,
  127, 255,  0,255,
  255, 255,  0,  0};






// Gradient palette "kmo_rainbow_12_gp", originally from
// https://phillips.shef.ac.uk/pub/cpt-city/kmo/rainbow-12
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 96 bytes of program space.

DEFINE_GRADIENT_PALETTE( kmo_rainbow_12_gp ) {
    0,  49,  1, 37,
   21,  49,  1, 37,
   21,  88,  7, 16,
   42,  88,  7, 16,
   42, 142, 33, 25,
   63, 142, 33, 25,
   63, 213, 82,  9,
   84, 213, 82,  9,
   84, 213,186,  0,
  106, 213,186,  0,
  106,  67,186, 16,
  127,  67,186, 16,
  127,   8,186, 52,
  148,   8,186, 52,
  148,   1,156,117,
  170,   1,156,117,
  170,   0,128,145,
  191,   0,128,145,
  191,   0, 82,145,
  212,   0, 82,145,
  212,   3, 33,117,
  233,   3, 33,117,
  233,  23,  7, 71,
  255,  23,  7, 71};






// Gradient palette "bhw_bhw3_bhw3_62_gp", originally from  MAYBE MAYBE MAYBE
// https://phillips.shef.ac.uk/pub/cpt-city/bhw/bhw3/bhw3_62
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw_bhw3_bhw3_62_gp ) {
    0, 255,255, 45,
   43, 208, 93,  1,
  137, 224,  1,242,
  181, 159,  1, 29,
  255,  63,  4, 68};






// Gradient palette "bhw_bhw1_bhw1_w00t_gp", originally from    GOOD ONE USE USE USE
// https://phillips.shef.ac.uk/pub/cpt-city/bhw/bhw1/bhw1_w00t
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( bhw_bhw1_bhw1_w00t_gp ) {
    0,   3, 13, 43,
  104,  78,141,240,
  188, 255,  0,  0,
  255,  28,  1,  1};





DEFINE_GRADIENT_PALETTE( bhw_bhw3_bhw3_01_gp ) {
    0, 255,146,228,
    5, 255,146,228,
   25, 152, 43, 65,
   48,  36, 17, 22,
   72,   8, 34, 75,
   81,   1, 58,170,
  104,  42, 49,245,
  124,  15, 15, 95,
  150,   1,  8, 39,
  175,  98, 32, 35,
  204, 123,  9,  2,
  219, 220, 15,  2,
  237, 255, 93,  6,
  255, 244,244,  0};



// ─────────────────────────────────────────────────────────────────────────────
//  NON-BLOCKING MATRIX FILL FUNCTIONS
//
//  Call signature (all 13 functions):
//    updateXxxPalette(palette, speed, intervalMs)
//      palette    - CRGBPalette16 to colour the LEDs
//      speed      - uint8_t, animation speed for beat8() (1-255; higher = faster)
//      intervalMs - uint16_t, ms between redraws (default 20 = ~50 FPS)
//
//  Each function computes beat8(speed) internally, fills all LEDs, and calls
//  FastLED.show(). Only one should be active at a time in a BETWEEN() block.
//
//  Usage:
//    BETWEEN(START(0,0,0), TO(0,0,10)) { updateNorthPalette(Rainbow_gp, 40, 20); }
// ─────────────────────────────────────────────────────────────────────────────

static uint32_t mf_lastUpdate = 0;  // shared frame-rate timer

// Internal draw helpers
static void _mf_physicalOrder(CRGBPalette16& p, uint8_t o)    { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,led_matrix[i]-o); }
static void _mf_clockwise(CRGBPalette16& p, uint8_t o)        { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o+angles[i]); }
static void _mf_counterClockwise(CRGBPalette16& p, uint8_t o) { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o-angles[i]); }
static void _mf_outward(CRGBPalette16& p, uint8_t o)          { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o-radii[i]); }
static void _mf_inward(CRGBPalette16& p, uint8_t o)           { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o+radii[i]); }
static void _mf_north(CRGBPalette16& p, uint8_t o)            { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o-coordsY[i]); }
static void _mf_northEast(CRGBPalette16& p, uint8_t o)        { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o-(coordsX[i]+coordsY[i])); }
static void _mf_east(CRGBPalette16& p, uint8_t o)             { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o-coordsX[i]); }
static void _mf_southEast(CRGBPalette16& p, uint8_t o)        { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o-coordsX[i]+coordsY[i]); }
static void _mf_south(CRGBPalette16& p, uint8_t o)            { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o+coordsY[i]); }
static void _mf_southWest(CRGBPalette16& p, uint8_t o)        { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o+coordsX[i]+coordsY[i]); }
static void _mf_west(CRGBPalette16& p, uint8_t o)             { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o+coordsX[i]); }
static void _mf_northWest(CRGBPalette16& p, uint8_t o)        { for (uint16_t i=0;i<NUM_LEDS;i++) leds[i]=ColorFromPalette(p,o+coordsX[i]-coordsY[i]); }

#define MF_UPDATE(fn, pal, spd, ms)                           \
  do {                                                         \
    uint32_t _now = millis();                                  \
    if (_now - mf_lastUpdate < (uint32_t)(ms)) return;        \
    mf_lastUpdate = _now;                                      \
    uint8_t _o = beat8((spd));                                 \
    fn((pal), _o);                                             \
    FastLED.show();                                            \
  } while(0)

void updatePhysicalOrderPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)    { MF_UPDATE(_mf_physicalOrder,   palette,spd,intervalMs); }
void updateClockwisePalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)        { MF_UPDATE(_mf_clockwise,        palette,spd,intervalMs); }
void updateCounterClockwisePalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20) { MF_UPDATE(_mf_counterClockwise, palette,spd,intervalMs); }
void updateOutwardPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)          { MF_UPDATE(_mf_outward,          palette,spd,intervalMs); }
void updateInwardPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)           { MF_UPDATE(_mf_inward,           palette,spd,intervalMs); }
void updateNorthPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)            { MF_UPDATE(_mf_north,            palette,spd,intervalMs); }
void updateNorthEastPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)        { MF_UPDATE(_mf_northEast,        palette,spd,intervalMs); }
void updateEastPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)             { MF_UPDATE(_mf_east,             palette,spd,intervalMs); }
void updateSouthEastPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)        { MF_UPDATE(_mf_southEast,        palette,spd,intervalMs); }
void updateSouthPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)            { MF_UPDATE(_mf_south,            palette,spd,intervalMs); }
void updateSouthWestPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)        { MF_UPDATE(_mf_southWest,        palette,spd,intervalMs); }
void updateWestPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)             { MF_UPDATE(_mf_west,             palette,spd,intervalMs); }
void updateNorthWestPalette(CRGBPalette16 palette, uint8_t spd, uint16_t intervalMs=20)        { MF_UPDATE(_mf_northWest,        palette,spd,intervalMs); }
