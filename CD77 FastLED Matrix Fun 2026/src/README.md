# CD77 FastLED Matrix Fun 2026 by Chemdoc77

                   Effect Reference

## Any W x H WS2812b Matrix  |  FastLED / Lolin D32

---

## Matrix Configuration

Set these in `CD77_Spiral_FX_with_time.ino` before the `#include` lines:

```cpp
#define MATRIX_WIDTH    16    // total pixel columns  (must be even, >= 8)
#define MATRIX_HEIGHT   16    // total pixel rows     (must be even, >= 8)
#define NUM_LEDS        (MATRIX_WIDTH * MATRIX_HEIGHT)
#define WIRING_ORIGIN   WIRING_ORIGIN_TOP_LEFT
#define WIRING_DIR      WIRING_DIR_COLUMNS
```

Call `sp_init();` in `setup()` — builds the LED map and initialises all
quad objects for your matrix size and wiring.  Nothing else needs to change.

### Wiring Origin Options

| Constant                      | LED 0 location |
|-------------------------------|----------------|
| `WIRING_ORIGIN_TOP_LEFT`      | Top-left corner *(default)* |
| `WIRING_ORIGIN_TOP_RIGHT`     | Top-right corner |
| `WIRING_ORIGIN_BOTTOM_LEFT`   | Bottom-left corner |
| `WIRING_ORIGIN_BOTTOM_RIGHT`  | Bottom-right corner |

### Wiring Direction Options

| Constant                          | Description |
|-----------------------------------|-------------|
| `WIRING_DIR_COLUMNS`              | **Serpentine** — even cols top→bottom, odd cols bottom→top *(default)* |
| `WIRING_DIR_ROWS`                 | **Serpentine** — even rows left→right, odd rows right→left |
| `WIRING_DIR_COLUMNS_PROGRESSIVE`  | **Progressive** — every column runs the same direction |
| `WIRING_DIR_ROWS_PROGRESSIVE`     | **Progressive** — every row runs the same direction |

Serpentine is the most common wiring for WS2812b panels.
Progressive (typewriter-style) wiring is less common but fully supported.

---

## Quad Layout

There are always exactly **4 quads**, each `QUAD_WIDTH x QUAD_HEIGHT`:

```
QUAD_WIDTH  = MATRIX_WIDTH  / 2
QUAD_HEIGHT = MATRIX_HEIGHT / 2
```

| Matrix size | Quad size |
|-------------|-----------|
| 16 x 16     | 8 x 8     |
| 32 x 16     | 16 x 8    |
|  8 x  8     | 4 x 4     |
| 24 x 24     | 12 x 12   |

```
  (1,1) Top-Left   |  (2,1) Top-Right
  -----------------+-----------------
  (1,2) Bot-Left   |  (2,2) Bot-Right
```

Quad addressing uses `(qCol, qRow)`, both 1-based.

---

## Time Macros

```cpp
FROM(H, M, S)  { ... }   // runs every loop() from this time onward
AT(H, M, S)    { ... }   // fires exactly once at this time
```

---

## Available Palettes

`RainbowColors_p`  `RainbowStripeColors_p`  `OceanColors_p`  `LavaColors_p`
`ForestColors_p`  `CloudColors_p`  `PartyColors_p`  `HeatColors_p`

---

## SECTION 1 — Rotating Archimedean Spirals
### Source: `CD77_Spiral_FX.h`

### 1a. `fullSpiral.Update()` — Single colour, full matrix

```cpp
fullSpiral.Update(armColor, bgColor, speedMs, armThickness, clockwise);
FastLED.show();
```

### 1b. `updateMatrix_4quad_Spiral()` — Single colour, per quad

```cpp
updateMatrix_4quad_Spiral(qCol, qRow, armColor, bgColor, speedMs, armThickness, clockwise);
FastLED.show();   // once after all quad calls
```

| Parameter      | Description |
|----------------|-------------|
| `qCol`         | 1 = left quad    2 = right quad |
| `qRow`         | 1 = top quad     2 = bottom quad |
| `armColor`     | Colour of the spiral arms |
| `bgColor`      | Background colour between arms |
| `speedMs`      | ms per full revolution (smaller = faster) |
| `armThickness` | 0.05 = hair-thin  …  0.90 = very wide |
| `clockwise`    | true = CW   false = CCW |

**Example:**
```cpp
FROM(0,0,0) {
  updateMatrix_4quad_Spiral(1, 1, CRGB::Red,    CRGB::Black, 2000, 0.60f, true);
  updateMatrix_4quad_Spiral(2, 1, CRGB::Blue,   CRGB::Black, 1500, 0.60f, false);
  updateMatrix_4quad_Spiral(1, 2, CRGB::Yellow, CRGB::Black, 1000, 0.60f, false);
  updateMatrix_4quad_Spiral(2, 2, CRGB::Green,  CRGB::Black,  750, 0.60f, true);
  FastLED.show();
}
```

---

## SECTION 2 — Palette Rotating Spirals
### Source: `CD77_Spiral_FX_P.h`

### 2a. `fullSpiralP.Update()` — Palette, full matrix

```cpp
fullSpiralP.Update(palette, bgColor, speedMs, armThickness, clockwise, paletteSpeed);
FastLED.show();
```

### 2b. `updateMatrix_4quad_SpiralP()` — Palette, per quad

```cpp
updateMatrix_4quad_SpiralP(qCol, qRow, palette, bgColor, speedMs, armThickness, clockwise, paletteSpeed);
FastLED.show();   // once after all quad calls
```

| Parameter      | Description |
|----------------|-------------|
| `qCol`, `qRow` | Quad address (see above) |
| `palette`      | Any CRGBPalette16 — each quad can use a different palette |
| `bgColor`      | Background colour |
| `speedMs`      | ms per full revolution |
| `armThickness` | 0.05 = hair-thin  …  0.90 = very wide |
| `clockwise`    | true = CW   false = CCW |
| `paletteSpeed` | 0=static  3=gentle drift  10=fast  20=psychedelic |

**Example:**
```cpp
FROM(0,0,0) {
  updateMatrix_4quad_SpiralP(1, 1, LavaColors_p,   CRGB::Black, 1500, 0.40f, true,  3);
  updateMatrix_4quad_SpiralP(2, 1, OceanColors_p,  CRGB::Black, 1500, 0.40f, false, 3);
  updateMatrix_4quad_SpiralP(1, 2, ForestColors_p, CRGB::Black, 1500, 0.40f, false, 3);
  updateMatrix_4quad_SpiralP(2, 2, PartyColors_p,  CRGB::Black, 1500, 0.40f, true,  3);
  FastLED.show();
}
```

---

## SECTION 3 — Box-Fill Spirals (solid colour)
### Source: `Matrix_loop_functions.h`

Concentric rectangular ring fills.  LEDs light one at a time tracing
inward (or outward) rings until the quad/matrix is full, then hold and repeat.

### 3a. `updateMatrixBoxes()` — Solid colour, full matrix

```cpp
updateMatrixBoxes(color, intervalMs, holdMs, reverse);
FastLED.show();
```

**Reset:** `sp_ring = -1;`

### 3b. `updateMatrix_4quad_Boxes()` — Solid colour, all 4 quads simultaneously

```cpp
updateMatrix_4quad_Boxes(color, intervalMs, holdMs, reverse);
FastLED.show();
```

All 4 quads fill in perfect sync.  Scales to any matrix size.

**Reset:** `sp8_ring = -1;`

### 3c. `updateMatrix_4quad_BoxesQ()` — Solid colour, independent per-quad

```cpp
updateMatrix_4quad_BoxesQ(qCol, qRow, color, intervalMs, holdMs, reverse);
FastLED.show();   // once after all quad calls
```

All quads share one counter — zero timing drift.
Pass the same `intervalMs` and `holdMs` to every quad call in a block.

| Parameter    | Description |
|--------------|-------------|
| `qCol`, `qRow` | Quad address |
| `color`      | Fill colour for this quad |
| `intervalMs` | ms between each LED step |
| `holdMs`     | ms to hold fully-lit before restarting |
| `reverse`    | false = outside->in   true = inside->out |

**Example:**
```cpp
FROM(0,0,0) {
  updateMatrix_4quad_BoxesQ(1, 1, CRGB::Blue, 80, 200, false);  // Top-L fills in
  updateMatrix_4quad_BoxesQ(2, 1, CRGB::Red,  80, 200, false);  // Top-R fills in
  updateMatrix_4quad_BoxesQ(1, 2, CRGB::Red,  80, 200, true);   // Bot-L fills out
  updateMatrix_4quad_BoxesQ(2, 2, CRGB::Blue, 80, 200, true);   // Bot-R fills out
  FastLED.show();
}
```

---

## SECTION 4 — Box-Fill Spirals (palette colour)
### Source: `Matrix_loop_functions_P.h`

### 4a. `updateMatrixBoxesP()` — Palette, full matrix

```cpp
updateMatrixBoxesP(palette, bgColor, intervalMs, holdMs, reverse, paletteSpeed);
FastLED.show();
```

**Reset:** `sp_ring = -1;`

### 4b. `updateMatrix_4quad_BoxesP()` — Palette, all 4 quads simultaneously

```cpp
updateMatrix_4quad_BoxesP(palette, bgColor, intervalMs, holdMs, reverse, paletteSpeed);
FastLED.show();
```

**Reset:** `sp8_ring = -1;`

### 4c. `updateMatrix_4quad_BoxesQP()` — Palette, independent per-quad

```cpp
updateMatrix_4quad_BoxesQP(qCol, qRow, palette, intervalMs, holdMs, reverse, paletteSpeed);
FastLED.show();   // once after all quad calls
```

Resets automatically each cycle.  Pass the same `intervalMs`, `holdMs`, and
`paletteSpeed` to every quad call in a block.

**Example:**
```cpp
FROM(0,0,0) {
  updateMatrix_4quad_BoxesQP(1, 1, RainbowColors_p, 80, 300, false, 3);
  updateMatrix_4quad_BoxesQP(2, 1, LavaColors_p,    80, 300, false, 3);
  updateMatrix_4quad_BoxesQP(1, 2, LavaColors_p,    80, 300, true,  3);
  updateMatrix_4quad_BoxesQP(2, 2, RainbowColors_p, 80, 300, true,  3);
  FastLED.show();
}
```

---

## SECTION 5 — Restart Rules

Add to your `AT()` restart line as needed:

| Effect in use               | Add to AT() restart line |
|-----------------------------|--------------------------|
| `updateMatrixBoxes`         | `sp_ring  = -1;`         |
| `updateMatrixBoxesP`        | `sp_ring  = -1;`         |
| `updateMatrix_4quad_Boxes`      | `sp8_ring = -1;`         |
| `updateMatrix_4quad_BoxesP`     | `sp8_ring = -1;`         |
| `updateMatrix_4quad_BoxesQ`     | `sp8q_reset();`          |
| `updateMatrix_4quad_BoxesQP`    | *(resets automatically)* |
| `fullSpiral` / `fullSpiralP`| *(resets automatically)* |
| `updateMatrix_4quad_Spiral`     | *(resets automatically)* |
| `updateMatrix_4quad_SpiralP`    | *(resets automatically)* |

**Example restart line:**
```cpp
AT(0,1,0) { sp_ring = -1; sp8_ring = -1; sp8q_reset(); RestartPerformance(); }
```

---

## SECTION 6 — File Summary

| File                           | Contents |
|--------------------------------|----------|
| `CD77_Spiral_FX_with_time.ino` | Matrix config, wiring config, hardware setup |
| `Time_performance.h`           | `Performance()` — edit this to build your show |
| `CD77_Spiral_FX.h`             | `Spiral`, `SpiralQ`, `updateMatrix_4quad_Spiral`, `fullSpiral` |
| `CD77_Spiral_FX_P.h`           | `SpiralP`, `SpiralQP`, `updateMatrix_4quad_SpiralP`, `fullSpiralP`, `sp_init()` |
| `Matrix_loop_functions.h`      | LED map generation, solid box-fill functions |
| `Matrix_loop_functions_P.h`    | Palette box-fill functions |

All six files must be in the same Arduino/PlatformIO sketch folder.
