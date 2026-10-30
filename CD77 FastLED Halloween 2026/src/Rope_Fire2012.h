#pragma once
/*
  Rope_Fire2012.h - Fire2012 for the rope matrix (non-blocking state machine)

  Based on Mark Kriegsman's Fire2012WithPalette:
  https://github.com/FastLED/FastLED/blob/master/examples/Fire2012WithPalette/Fire2012WithPalette.ino
  and on the Fibonacci-module version (hidden heat grid read through a map).

  The rope is 10 columns (the strands run up one column and down the next), so a
  plain Fire2012 along LED numbers 0-255 would run flames along the wire and
  ignore the layout. Instead each column is its own Fire2012 "chimney":
    1. Cool every cell a little.
    2. Heat drifts up the column and diffuses.
    3. Randomly ignite a spark near the BOTTOM LED of that column.
  Then every LED reads the heat of its cell, lightly blended (1:2:1) with the cells
  of the columns on its left and right at the same sheet row, and looks its color up
  in the palette. The layout comes from RopeMap.h (the spreadsheet), so the up/down
  wiring of the strands is handled for you.

  COOLING : how much the air cools as it rises. Less = taller flames. Default 55, range 20-100
  SPARKING: chance (out of 255) per column per frame that a new spark is lit.
            Higher = more roaring. Default 120, range 50-200

  Usage (call every loop pass, inside a BETWEEN block):
    Fire2012Rope.update(HeatColors_p, 75, 80);          // palette, cooling, sparking
    Fire2012Rope.update(IceColors_p, 75, 80, 30);       // optional 4th arg: ms between frames (default 20 = ~50 FPS)
*/
#include "Config.h"
#include "RopeMap.h"

class Fire2012Rope_SM {
public:
  void reset() {
    for (uint8_t c = 0; c < ROPE_COLS; c++)
      for (uint8_t h = 0; h < ROPE_ROWS; h++) _heat[c][h] = 0;
    _started = false;
  }

  void update(const CRGBPalette16& pal, uint8_t cooling, uint8_t sparking,
              uint16_t intervalMs = 20) {
    uint32_t now = millis();
    if (_started && (now - _lastFrame) < intervalMs) return;
    _started   = true;
    _lastFrame = now;

    random16_add_entropy(random(0, 65535));

    for (uint8_t c = 0; c < ROPE_COLS; c++) {
      const uint8_t len = ropeColLen[c];
      uint8_t* heat = _heat[c];

      // Step 1. Cool down every cell a little
      for (uint8_t i = 0; i < len; i++)
        heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / len) + 2));

      // Step 2. Heat from each cell drifts 'up' and diffuses a little
      for (int8_t k = len - 1; k >= 2; k--)
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;

      // Step 3. Randomly ignite new 'sparks' of heat near the bottom of the column
      if (random8() < sparking) {
        uint8_t y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
      }
    }

    // Step 4. Map from heat cells to LED colors
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
      const uint8_t c   = ropeCol[i];
      const uint8_t own = _heat[c][ropeHeight[i]];
      const uint8_t l   = neighbor(c, -1, ropeRow[i], own);
      const uint8_t r   = neighbor(c, +1, ropeRow[i], own);
      const uint8_t heatVal = ((uint16_t)l + 2 * own + r) >> 2;
      leds[i] = ColorFromPalette(pal, scale8(heatVal, 240));   // 0-240 suits palettes best
    }
    FastLED.show();
  }

private:
  uint8_t  _heat[ROPE_COLS][ROPE_ROWS] = {{0}};
  bool     _started   = false;
  uint32_t _lastFrame = 0;

  // heat of the cell in the next column over at the same sheet row;
  // if that column has no LED there, fall back to our own heat
  uint8_t neighbor(uint8_t c, int8_t dir, uint8_t sheetRow, uint8_t fallback) const {
    int8_t nc = (int8_t)c + dir;
    if (nc < 0 || nc >= ROPE_COLS) return fallback;
    int8_t nh = (int8_t)ropeColBottomRow[nc] - (int8_t)sheetRow;
    if (nh < 0 || nh >= (int8_t)ropeColLen[nc]) return fallback;
    return _heat[nc][nh];
  }
};

Fire2012Rope_SM Fire2012Rope;
