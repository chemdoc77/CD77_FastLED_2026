#ifndef ropestuff
#define ropestuff



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

CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
CRGBPalette16 hc1_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkGreen, CRGB::DarkRed, CRGB::Black);
CRGBPalette16 hc2_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen,  CRGB::DarkGreen, CRGB::DarkGreen,  CRGB::DarkRed, CRGB::DarkRed, CRGB::DarkRed, CRGB::DarkRed);
CRGBPalette16 hc3_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen);
CRGBPalette16 hc4_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen);
CRGBPalette16 hc5_p = CRGBPalette16(CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet, CRGB::DarkViolet , CRGB::DarkViolet, CRGB::OrangeRed, CRGB::OrangeRed, CRGB::OrangeRed , CRGB::OrangeRed, CRGB::OrangeRed, CRGB::OrangeRed, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen, CRGB::DarkGreen);
CRGBPalette16 HO1_p = CRGBPalette16(CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red , CRGB::White, CRGB::White, CRGB::White, CRGB::White , CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::White, CRGB::White, CRGB::White, CRGB::White);
void clockwisePalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset + angles[i]);
  }
}

void counterClockwisePalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset - angles[i]);
  }
}

void outwardPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset - radii[i]);
  }
}

void inwardPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset + radii[i]);
  }
}

void northPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset - coordsY[i]);
  }
}

void northEastPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset - (coordsX[i] + coordsY[i]));
  }
}

void eastPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset - coordsX[i]);
  }
}

void southEastPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset - coordsX[i] + coordsY[i]);
  }
}

void southPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset + coordsY[i]);
  }
}

void southWestPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset + coordsX[i] + coordsY[i]);
  }
}

void westPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset + coordsX[i]);
  }
}

void northWestPalette()
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = ColorFromPalette(currentPalette, offset + coordsX[i] - coordsY[i]);
  }
}


#endif