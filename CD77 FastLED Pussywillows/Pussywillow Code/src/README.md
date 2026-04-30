#FastLED Pussywillows by Chemdoc77 — Sketch README

## Overview

This sketch drives a **15 × 10 WS2811 serpentine LED matrix (150 LEDs)** on a Lolin D32.
Effect logic lives in the included header files; the main `.ino` file handles hardware setup and the performance timeline.

---

## File Summary

| File | Purpose |
|---|---|
| `CD77_pussywillows_both.ino` | Main sketch — hardware config, setup(), loop(), global variables |
| `pussywillow_functions.h` | All pussywillow update functions and their state machines |
| `Time_performance.h` | Performance timeline using `AT()` / `FROM()` macros |
| `CD77_slowchangepalette.h` | `FadeToPaletteAnimation` class — non-blocking smooth palette crossfade with `FastLED.show()` built in |
| `matrix_functions.h` | Palette-based matrix fill functions (directional, radial, etc.) |
| `led_matrix.h` | Physical LED index map for the serpentine wiring |
| `pw_jladder.h` | Exponentially accelerating row-fill effects (`updateJladder` / `updateJladderP`) |

---

## Hardware Configuration
*(defined at the top of `CD77_pussywillows_both.ino`)*

| `#define` | Default | Description |
|---|---|---|
| `MATRIX_WIDTH` | `10` | Number of columns |
| `MATRIX_HEIGHT` | `15` | Number of rows |
| `NUM_LEDS` | `150` | Total LEDs (WIDTH × HEIGHT) |
| `LED_PIN` | `12` | Data pin to first LED |
| `LED_TYPE` | `WS2811` | LED chipset |
| `COLOR_ORDER` | `RGB` | Change to `GRB` if colours appear wrong |
| `BRIGHTNESS` | `150` | Global brightness, 0–255 |

---

## Pussywillow Update Functions

All functions are non-blocking state machines. Call them every `loop()` iteration — they self-initialise on their first call and handle their own timing internally.

---

### `updatePussywillowRow()`

Lights the matrix one **row at a time**, travelling from bottom to top or top to bottom. When the last row is reached it holds for `holdMs`, clears the matrix, and restarts.

```cpp
updatePussywillowRow(color, intervalMs, holdMs, reverse);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Colour to fill each row |
| `intervalMs` | `uint32_t` | Milliseconds between each row step |
| `holdMs` | `uint32_t` | Milliseconds to hold once the matrix is full before clearing |
| `reverse` | `bool` | `false` = bottom → top &nbsp; `true` = top → bottom |

**Example:**
```cpp
updatePussywillowRow(CRGB::Blue, 200, 2000, false);
```

---

### `updatePussywillowRowP()`

Identical to `updatePussywillowRow()` but each row is coloured from a palette. The palette offset advances by `paletteSpeed` each row step so every row gets a different colour as the fill progresses.

```cpp
updatePussywillowRowP(palette, intervalMs, holdMs, reverse, paletteSpeed);
```

| Parameter | Type | Description |
|---|---|---|
| `palette` | `CRGBPalette16` | Palette to colour each row |
| `intervalMs` | `uint32_t` | Milliseconds between each row step |
| `holdMs` | `uint32_t` | Milliseconds to hold once the matrix is full before clearing |
| `reverse` | `bool` | `false` = bottom → top &nbsp; `true` = top → bottom |
| `paletteSpeed` | `uint8_t` | How much the palette index advances each row step (1–20, default 10) |

**Example:**
```cpp
updatePussywillowRowP(RainbowColors_p, 200, 2000, false, 10);
```

---

### `updateJladder()`

Fills the matrix one **row at a time**, travelling from bottom to top (or reversed), with **speed that increases with each successive fill cycle**. All rows within a single fill travel at the same uniform speed. After the matrix is full and the hold expires, the interval for the next fill is reduced by `accelPct%`, so each loop arrives faster than the last — like a drummer gradually picking up tempo.

```cpp
updateJladder(color, startMs, accelPct, holdMs, reverse);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Solid colour to fill each row |
| `startMs` | `uint32_t` | Row interval in milliseconds for the **first** fill cycle (e.g. `400`) |
| `accelPct` | `uint8_t` | % reduction applied to the interval at the start of each new fill cycle (1–90). **Larger = faster acceleration across loops.** `10` = very gentle &nbsp; `20` = gentle &nbsp; `30` = moderate &nbsp; `45` = aggressive &nbsp; `70` = near-instant |
| `holdMs` | `uint32_t` | Milliseconds to hold once the matrix is full before the next (faster) fill begins |
| `reverse` | `bool` | `false` = bottom → top &nbsp; `true` = top → bottom |

**Reset sentinel:** `pw_jladderRow = -1;`

**Example:**
```cpp
BETWEEN(START(0,0,0), TO(0,0,8))  { updateJladder(CRGB::Red, 400, 30, 1500, false); }
```

---

### `updateJladderP()`

Identical acceleration behaviour to `updateJladder()` — speed is constant within each fill, then reduced by `accelPct%` before the next fill begins. Each row is coloured from a **palette**; the palette offset advances by `paletteSpeed` each row step so every row gets a different colour.

```cpp
updateJladderP(palette, startMs, accelPct, holdMs, reverse, paletteSpeed);
```

| Parameter | Type | Description |
|---|---|---|
| `palette` | `CRGBPalette16` | Palette to colour each row |
| `startMs` | `uint32_t` | Row interval in milliseconds for the **first** fill cycle (e.g. `400`) |
| `accelPct` | `uint8_t` | % reduction applied to the interval at the start of each new fill cycle (1–90). **Larger = faster acceleration across loops.** `10` = very gentle &nbsp; `20` = gentle &nbsp; `30` = moderate &nbsp; `45` = aggressive &nbsp; `70` = near-instant |
| `holdMs` | `uint32_t` | Milliseconds to hold once the matrix is full before the next (faster) fill begins |
| `reverse` | `bool` | `false` = bottom → top &nbsp; `true` = top → bottom |
| `paletteSpeed` | `uint8_t` | How much the palette index advances each row step (1–20, default 10) |

**Reset sentinel:** `pw_jladderPRow = -1;`

**Example:**
```cpp
BETWEEN(START(0,0,8), TO(0,0,16)) { updateJladderP(RainbowColors_p, 400, 30, 1500, false, 10); }
```

> **Note:** Both Jladder functions require `#include "pw_jladder.h"` in the `.ino` (after `pussywillow_functions.h`), and their reset sentinels (`pw_jladderRow = -1;` and `pw_jladderPRow = -1;`) must be added to `resetPussywillowEffects()` in `pussywillow_functions.h`.

---

### `updatePussywillowCol()`

Lights the matrix one **column at a time**, travelling from left to right or right to left. When the last column is reached it holds for `holdMs`, clears the matrix, and restarts.

```cpp
updatePussywillowCol(color, intervalMs, holdMs, reverse);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Colour to fill each column |
| `intervalMs` | `uint32_t` | Milliseconds between each column step |
| `holdMs` | `uint32_t` | Milliseconds to hold once the matrix is full before clearing |
| `reverse` | `bool` | `false` = left → right &nbsp; `true` = right → left |

**Example:**
```cpp
updatePussywillowCol(CRGB::Red, 200, 2000, false);
```

---

### `updatePussywillowColP()`

Identical to `updatePussywillowCol()` but each column is coloured from a palette. The palette offset advances by `paletteSpeed` each column step so every column gets a different colour.

```cpp
updatePussywillowColP(palette, intervalMs, holdMs, reverse, paletteSpeed);
```

| Parameter | Type | Description |
|---|---|---|
| `palette` | `CRGBPalette16` | Palette to colour each column |
| `intervalMs` | `uint32_t` | Milliseconds between each column step |
| `holdMs` | `uint32_t` | Milliseconds to hold once the matrix is full before clearing |
| `reverse` | `bool` | `false` = left → right &nbsp; `true` = right → left |
| `paletteSpeed` | `uint8_t` | How much the palette index advances each column step (1–20, default 25) |

**Example:**
```cpp
updatePussywillowColP(RainbowColors_p, 200, 2000, false, 25);
```

---

### `updatePussywillowDotFill()`

Lights the matrix **one LED at a time**, row by row. Within each row LEDs light left to right (or right to left when reversed). When the entire matrix is filled it holds for `holdMs`, clears, and restarts.

```cpp
updatePussywillowDotFill(color, intervalMs, holdMs, reverse);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Colour for each LED as it lights |
| `intervalMs` | `uint32_t` | Milliseconds between each individual LED step |
| `holdMs` | `uint32_t` | Milliseconds to hold once the matrix is full before clearing |
| `reverse` | `bool` | `false` = clockwise: rows bottom → top, LEDs left → right &nbsp; `true` = counter-clockwise: rows top → bottom, LEDs right → left |

**Example:**
```cpp
updatePussywillowDotFill(CRGB::Green, 50, 2000, false);
```

---

### `updatePussywillowDotFillP()`

Identical behaviour to `updatePussywillowDotFill()` but each LED is coloured from a palette. The palette offset advances by `paletteSpeed` on every single LED step, creating a flowing colour wash across the matrix as it fills in.

```cpp
updatePussywillowDotFillP(palette, intervalMs, holdMs, reverse, paletteSpeed);
```

| Parameter | Type | Description |
|---|---|---|
| `palette` | `CRGBPalette16` | Palette to colour each LED |
| `intervalMs` | `uint32_t` | Milliseconds between each individual LED step |
| `holdMs` | `uint32_t` | Milliseconds to hold once the matrix is full before clearing |
| `reverse` | `bool` | `false` = clockwise: rows bottom → top, LEDs left → right &nbsp; `true` = counter-clockwise |
| `paletteSpeed` | `uint8_t` | How much the palette index advances each LED step (1–10, default 2) — lower = smoother wash |

**Example:**
```cpp
updatePussywillowDotFillP(RainbowColors_p, 50, 2000, false, 2);
```

Lights **one row at a time** with two selectable motion modes. Only the currently active row is lit; all other rows are black. Pauses at the far end for `holdMs` before continuing.

```cpp
updatePussywillowRowBounce(color, intervalMs, holdMs, reverse, pingPong);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Solid colour for the lit row |
| `intervalMs` | `uint32_t` | Milliseconds between each row step |
| `holdMs` | `uint32_t` | Milliseconds to pause at the end before reversing or looping |
| `reverse` | `bool` | `false` = start at bottom, travel up first &nbsp; `true` = start at top, travel down first |
| `pingPong` | `bool` | `true` = bounce back and forth &nbsp; `false` = one-way loop, snaps back to start each cycle |

**Examples:**
```cpp
updatePussywillowRowBounce(CRGB::Red, 80, 500, false, true);   // bounce up and down
updatePussywillowRowBounce(CRGB::Red, 80, 500, false, false);  // loop upward, snap back to bottom
```

---

### `updatePussywillowRowBouncePalette()`

Identical motion behaviour to `updatePussywillowRowBounce()`, but the single lit row is painted with a **palette gradient** — the full palette is mapped across all 10 columns of the row (col 0 = palette index 0, col 9 = palette index 255).

```cpp
updatePussywillowRowBouncePalette(palette, intervalMs, holdMs, reverse, pingPong);
```

| Parameter | Type | Description |
|---|---|---|
| `palette` | `CRGBPalette16` | Palette to colour the lit row |
| `intervalMs` | `uint32_t` | Milliseconds between each row step |
| `holdMs` | `uint32_t` | Milliseconds to pause at the end before reversing or looping |
| `reverse` | `bool` | `false` = start at bottom, travel up first &nbsp; `true` = start at top, travel down first |
| `pingPong` | `bool` | `true` = bounce back and forth &nbsp; `false` = one-way loop, snaps back to start each cycle |

**Examples:**
```cpp
updatePussywillowRowBouncePalette(RainbowColors_p, 80, 500, false, true);   // bounce up and down
updatePussywillowRowBouncePalette(RainbowColors_p, 80, 500, false, false);  // loop upward, snap back to bottom
```

---

### `updatePussywillowSinelon()`

A coloured dot sweeps back and forth across the matrix rows, driven by `beatsin16()`. Each frame the entire matrix fades toward black, leaving a decaying trail behind the dot. The sweep fills any gap between the dot's previous and current position so the motion always appears smooth and continuous with no missing rows.

This effect runs entirely on its own internal clock — no `intervalMs` or `holdMs` are needed.

```cpp
updatePussywillowSinelon(color, bpm, fadeAmount, saturation, reverse);
```

| Parameter | Type | Default | Description |
|---|---|---|---|
| `color` | `CRGB` | — | Base colour for the dot and tail — hue is extracted from this colour |
| `bpm` | `uint8_t` | `13` | Sweep speed in beats per minute — higher = faster sweep |
| `fadeAmount` | `uint8_t` | `20` | How quickly the tail fades each frame (1–255) — lower = longer tail, higher = shorter tail |
| `saturation` | `uint8_t` | `220` | Colour saturation of the dot (0 = white, 255 = fully saturated) |
| `reverse` | `bool` | `false` | `false` = sweep starts from bottom &nbsp; `true` = sweep starts from top (phase inverted) |

**Examples:**
```cpp
updatePussywillowSinelon(CRGB::Red, 13, 20, 220, false);   // default red sweep
updatePussywillowSinelon(CRGB::Blue, 20, 10, 200, false);  // faster blue sweep, longer tail
updatePussywillowSinelon(CRGB::Green, 8, 40, 255, true);   // slow green sweep from top, short tail
```

---

### `updatePussywillowBreathe()`

Each of the 10 branches (columns) fades in and out at its own pace, driven by a sine wave with a unique phase offset per branch. The brightness peak ripples one branch at a time across the full spray — some branches brightening while others dim — creating a slow organic undulation like the branches are breathing. All LEDs within a branch share the same brightness at any moment.

```cpp
updatePussywillowBreathe(color, bpm, minBright, maxBright);
```

| Parameter | Type | Default | Description |
|---|---|---|---|
| `color` | `CRGB` | — | Colour of all branches |
| `bpm` | `uint8_t` | `10` | Overall breathing rate in beats per minute — higher = faster breathing |
| `minBright` | `uint8_t` | `10` | Minimum brightness at the bottom of each breath (0–255) |
| `maxBright` | `uint8_t` | `255` | Maximum brightness at the top of each breath (0–255) |

**Examples:**
```cpp
updatePussywillowBreathe(CRGB::White, 8, 5, 220);    // slow gentle white breathing
updatePussywillowBreathe(CRGB::Cyan, 15, 20, 255);   // faster cyan pulse
updatePussywillowBreathe(CRGB::Red, 6, 0, 200);      // very slow deep red breathing, fully off at bottom
```

---

### `updatePussywillowBreathePalette()`

Identical breathing behaviour to `updatePussywillowBreathe()` — each branch fades in and out with its own phase offset so the brightness ripples one branch at a time. Instead of a single colour, each branch takes its colour from a `CRGBPalette16` mapped evenly across all 10 columns (col 0 = palette index 0, col 9 = palette index 255), spreading the full palette gradient across the spray while it breathes.

```cpp
updatePussywillowBreathePalette(palette, bpm, minBright, maxBright);
```

| Parameter | Type | Default | Description |
|---|---|---|---|
| `palette` | `CRGBPalette16` | — | Palette to colour the branches |
| `bpm` | `uint8_t` | `10` | Overall breathing rate in beats per minute |
| `minBright` | `uint8_t` | `10` | Minimum brightness at the bottom of each breath (0–255) |
| `maxBright` | `uint8_t` | `255` | Maximum brightness at the top of each breath (0–255) |

**Examples:**
```cpp
updatePussywillowBreathePalette(RainbowColors_p, 8, 5, 220);   // slow rainbow breathing
updatePussywillowBreathePalette(OceanColors_p, 12, 0, 255);    // ocean blues, fully off at bottom
updatePussywillowBreathePalette(HO1_p, 6, 10, 200);            // red/white/blue, very slow
```

---

### `updatePussywillowMeteor()`

Multiple bright meteors streak upward from the base to the tip of randomly chosen branches simultaneously. Each meteor has a bright head and a linearly fading tail behind it. Each meteor picks a fully random hue at spawn so the streaks vary in colour naturally over time. When a meteor finishes it waits a random gap then re-spawns on a new random branch, so all 10 branches get coverage over time.

10 concurrent meteors run independently, each self-initialising and managing its own state.

```cpp
updatePussywillowMeteor(tailLength, minSpawnGap, maxSpawnGap, frameInterval);
```

| Parameter | Type | Default | Description |
|---|---|---|---|
| `tailLength` | `uint8_t` | `5` | Number of fading tail LEDs behind the head (1–12) |
| `minSpawnGap` | `uint16_t` | `400` | Minimum ms a meteor waits before re-spawning |
| `maxSpawnGap` | `uint16_t` | `1200` | Maximum ms a meteor waits before re-spawning |
| `frameInterval` | `uint8_t` | `40` | Milliseconds between rendered frames — lower = faster meteors |

**Examples:**
```cpp
updatePussywillowMeteor(5, 400, 1200, 40);   // default settings
updatePussywillowMeteor(8, 200, 800, 30);    // fast meteors, long tails
updatePussywillowMeteor(3, 600, 2000, 60);   // slow sparse meteors, short tails
```

---

### `updatePussywillowDualRow()`

Two rows travel simultaneously in opposite directions — one from the bottom upward, one from the top downward. When both reach their far ends they hold for `holdMs` then both reverse. Where the rows cross they blend with `|=` so neither colour is lost.

```cpp
updatePussywillowDualRow(colorUp, colorDown, intervalMs, holdMs);
```

| Parameter | Type | Description |
|---|---|---|
| `colorUp` | `CRGB` | Colour of the row travelling bottom → top |
| `colorDown` | `CRGB` | Colour of the row travelling top → bottom |
| `intervalMs` | `uint32_t` | Milliseconds between each row step |
| `holdMs` | `uint32_t` | Milliseconds to pause at each end before reversing |

**Example:**
```cpp
updatePussywillowDualRow(CRGB::Red, CRGB::Blue, 80, 500);
```

---

### `updatePussywillowDualRowP()`

Identical motion to `updatePussywillowDualRow()` but each row's colour cycles through a palette. An internal offset advances by `paletteSpeed` each step. The two rows are offset by 128 in the palette so they always show contrasting colours.

```cpp
updatePussywillowDualRowP(palette1, palette2, intervalMs, holdMs, paletteSpeed);
```

| Parameter | Type | Description |
|---|---|---|
| `palette1` | `CRGBPalette16` | Palette for the row travelling bottom → top |
| `palette2` | `CRGBPalette16` | Palette for the row travelling top → bottom |
| `intervalMs` | `uint32_t` | Milliseconds between each row step |
| `holdMs` | `uint32_t` | Milliseconds to pause at each end before reversing |
| `paletteSpeed` | `uint8_t` | How much the palette index advances each step (1–20, default 5) |

**Example:**
```cpp
updatePussywillowDualRowP(RainbowColors_p, RainbowColors_p, 80, 500, 5);
```

---

### `updatePussywillowDualCol()`

Two columns travel simultaneously in opposite directions — one from the left rightward, one from the right leftward. When both reach their far ends they hold for `holdMs` then both reverse. Where the columns cross they blend with `|=`.

```cpp
updatePussywillowDualCol(colorLeft, colorRight, intervalMs, holdMs);
```

| Parameter | Type | Description |
|---|---|---|
| `colorLeft` | `CRGB` | Colour of the column travelling left → right |
| `colorRight` | `CRGB` | Colour of the column travelling right → left |
| `intervalMs` | `uint32_t` | Milliseconds between each column step |
| `holdMs` | `uint32_t` | Milliseconds to pause at each end before reversing |

**Example:**
```cpp
updatePussywillowDualCol(CRGB::Red, CRGB::Blue, 150, 500);
```

---

### `updatePussywillowDualColP()`

Identical motion to `updatePussywillowDualCol()` but each column's colour cycles through a palette. An internal offset advances by `paletteSpeed` each step. The two columns are offset by 128 in the palette so they always show contrasting colours.

```cpp
updatePussywillowDualColP(palette1, palette2, intervalMs, holdMs, paletteSpeed);
```

| Parameter | Type | Description |
|---|---|---|
| `palette1` | `CRGBPalette16` | Palette for the column travelling left → right |
| `palette2` | `CRGBPalette16` | Palette for the column travelling right → left |
| `intervalMs` | `uint32_t` | Milliseconds between each column step |
| `holdMs` | `uint32_t` | Milliseconds to pause at each end before reversing |
| `paletteSpeed` | `uint8_t` | How much the palette index advances each step (1–20, default 5) |

**Example:**
```cpp
updatePussywillowDualColP(RainbowColors_p, RainbowColors_p, 150, 500, 5);
```

---

## Spiral & Box-Fill Effects

Defined in `pw_matrix_fx.h`. These effects are adapted from Chemdoc77's CD77 FastLED Matrix Fun 2026 sketch and use the pussywillow's unique `pw_ledMap` for correct physical LED addressing. Call `pw_sp_init()` once in `setup()` after `FastLED.addLeds()`. All functions call `FastLED.show()` internally.

---

### `fullSpiral.Update()`

A two-arm rotating Archimedean spiral fills the full 10×15 matrix. The spiral rotates continuously — no hold state, fully phase-driven.

```cpp
fullSpiral.Update(armColor, bgColor, speedMs, armThickness, clockwise);
```

| Parameter | Type | Description |
|---|---|---|
| `armColor` | `CRGB` | Colour of the spiral arms |
| `bgColor` | `CRGB` | Background colour between arms |
| `speedMs` | `uint32_t` | Milliseconds per full revolution — lower = faster |
| `armThickness` | `float` | Arm width: `0.05` = hair-thin … `0.90` = very wide |
| `clockwise` | `bool` | `true` = clockwise &nbsp; `false` = counter-clockwise |

**Example:**
```cpp
BETWEEN(START(0,0,0.000), TO(0,0,20.000)) { fullSpiral.Update(CRGB::Red, CRGB::Black, 1500, 0.60f, true); }
```

---

### `fullSpiralP.Update()`

Identical spiral behaviour to `fullSpiral` but the arm colour is drawn from a `CRGBPalette16`, mapped by angular position. A `paletteOffset` drifts forward each frame for a flowing colour effect.

```cpp
fullSpiralP.Update(palette, bgColor, speedMs, armThickness, clockwise, paletteSpeed);
```

| Parameter | Type | Description |
|---|---|---|
| `palette` | `CRGBPalette16` | Palette to colour the spiral arms |
| `bgColor` | `CRGB` | Background colour between arms |
| `speedMs` | `uint32_t` | Milliseconds per full revolution |
| `armThickness` | `float` | Arm width: `0.05` = hair-thin … `0.90` = very wide |
| `clockwise` | `bool` | `true` = clockwise &nbsp; `false` = counter-clockwise |
| `paletteSpeed` | `uint8_t` | Palette drift rate: `0`=static &nbsp; `3`=gentle &nbsp; `10`=fast &nbsp; `20`=psychedelic |

**Example:**
```cpp
BETWEEN(START(0,0,0.000), TO(0,0,20.000)) { fullSpiralP.Update(RainbowColors_p, CRGB::Black, 1500, 0.60f, true, 3); }
```

---

### `updateMatrixBoxes()`

Traces concentric rectangular rings across the full matrix one LED at a time, from outside in (or inside out). When all rings are filled it holds for `holdMs` then clears and repeats.

```cpp
updateMatrixBoxes(color, intervalMs, holdMs, reverse);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Fill colour |
| `intervalMs` | `uint32_t` | Milliseconds between each LED step |
| `holdMs` | `uint32_t` | Milliseconds to hold when fully filled before restarting |
| `reverse` | `bool` | `false` = outside → in &nbsp; `true` = inside → out |

Reset with: `sp_ring = -1;`

**Example:**
```cpp
BETWEEN(START(0,0,0.000), TO(0,0,20.000)) { updateMatrixBoxes(CRGB::Blue, 40, 500, false); }
```

---

### `updateMatrixBoxesP()`

Identical ring-fill behaviour to `updateMatrixBoxes` but each LED is coloured from a `CRGBPalette16` based on its angular position around the matrix centre. A `paletteOffset` drifts forward by `paletteSpeed` each step.

```cpp
updateMatrixBoxesP(palette, bgColor, intervalMs, holdMs, reverse, paletteSpeed);
```

| Parameter | Type | Description |
|---|---|---|
| `palette` | `CRGBPalette16` | Palette to colour each LED as it is filled |
| `bgColor` | `CRGB` | Background colour shown before each LED is filled |
| `intervalMs` | `uint32_t` | Milliseconds between each LED step |
| `holdMs` | `uint32_t` | Milliseconds to hold when fully filled before restarting |
| `reverse` | `bool` | `false` = outside → in &nbsp; `true` = inside → out |
| `paletteSpeed` | `uint8_t` | Palette drift rate: `0`=static &nbsp; `3`=gentle &nbsp; `10`=fast &nbsp; `20`=psychedelic |

Reset with: `sp_ring = -1;`

**Example:**
```cpp
BETWEEN(START(0,0,0.000), TO(0,0,20.000)) { updateMatrixBoxesP(RainbowColors_p, CRGB::Black, 40, 500, false, 3); }
```

---

### Front / Back Area Spiral & Box-Fill Effects

The matrix is divided into two **15×5 areas** based on the physical branch layout:
- **Front** = columns 0–4 (left half of the matrix)
- **Back** = columns 5–9 (right half of the matrix)

Each area has its own independent spiral and box-fill instances so both areas can run different effects, colours, directions and speeds simultaneously. LED mappings are sourced directly from the uploaded spreadsheets.

Call `FastLED.show()` is called internally by all these functions.

---

#### `updateAreaSpiral()`

Single-colour rotating spiral within the front or back area.

```cpp
updateAreaSpiral(area, armColor, bgColor, speedMs, armThickness, clockwise);
```

| Parameter | Type | Description |
|---|---|---|
| `area` | `PW_Area` | `FRONT` or `BACK` |
| `armColor` | `CRGB` | Colour of the spiral arms |
| `bgColor` | `CRGB` | Background colour between arms |
| `speedMs` | `uint32_t` | Milliseconds per full revolution |
| `armThickness` | `float` | `0.05` = hair-thin … `0.90` = very wide |
| `clockwise` | `bool` | `true` = CW &nbsp; `false` = CCW |

---

#### `updateAreaSpiralP()`

Palette-coloured rotating spiral within the front or back area.

```cpp
updateAreaSpiralP(area, palette, bgColor, speedMs, armThickness, clockwise, paletteSpeed);
```

| Parameter | Type | Description |
|---|---|---|
| `area` | `PW_Area` | `FRONT` or `BACK` |
| `palette` | `CRGBPalette16` | Palette for the spiral arms |
| `bgColor` | `CRGB` | Background colour |
| `speedMs` | `uint32_t` | Milliseconds per full revolution |
| `armThickness` | `float` | `0.05` = hair-thin … `0.90` = very wide |
| `clockwise` | `bool` | `true` = CW &nbsp; `false` = CCW |
| `paletteSpeed` | `uint8_t` | `0`=static &nbsp; `3`=gentle &nbsp; `10`=fast &nbsp; `20`=psychedelic |

**Example — front and back spinning opposite directions:**
```cpp
BETWEEN(START(0,0,0.000), TO(0,0,20.000)) {
  updateAreaSpiralP(FRONT, RainbowColors_p, CRGB::Black, 1500, 0.60f, true,  3);
  updateAreaSpiralP(BACK,  OceanColors_p,   CRGB::Black, 1500, 0.60f, false, 3);
}
```

---

#### `updateAreaBoxes()`

Concentric rectangular ring fill within the front or back area. Each area has its own independent state so both can run simultaneously.

```cpp
updateAreaBoxes(area, color, intervalMs, holdMs, reverse);
```

| Parameter | Type | Description |
|---|---|---|
| `area` | `PW_Area` | `FRONT` or `BACK` |
| `color` | `CRGB` | Fill colour |
| `intervalMs` | `uint32_t` | Milliseconds between each LED step |
| `holdMs` | `uint32_t` | Milliseconds to hold when full before restarting |
| `reverse` | `bool` | `false` = outside → in &nbsp; `true` = inside → out |

Reset with: `spa_front_ring = -1;` / `spa_back_ring = -1;`

---

#### `updateAreaBoxesP()`

Palette ring fill within the front or back area. Each LED is coloured from the palette based on its angular position around the area centre.

```cpp
updateAreaBoxesP(area, palette, bgColor, intervalMs, holdMs, reverse, paletteSpeed);
```

| Parameter | Type | Description |
|---|---|---|
| `area` | `PW_Area` | `FRONT` or `BACK` |
| `palette` | `CRGBPalette16` | Palette to colour each LED |
| `bgColor` | `CRGB` | Background colour before each LED is filled |
| `intervalMs` | `uint32_t` | Milliseconds between each LED step |
| `holdMs` | `uint32_t` | Milliseconds to hold when full before restarting |
| `reverse` | `bool` | `false` = outside → in &nbsp; `true` = inside → out |
| `paletteSpeed` | `uint8_t` | `0`=static &nbsp; `3`=gentle &nbsp; `10`=fast &nbsp; `20`=psychedelic |

Reset with: `spa_front_ring = -1;` / `spa_back_ring = -1;`

**Example — front and back filling in opposite directions simultaneously:**
```cpp
BETWEEN(START(0,0,0.000), TO(0,0,20.000)) {
  updateAreaBoxesP(FRONT, RainbowColors_p, CRGB::Black, 40, 500, false, 3);
  updateAreaBoxesP(BACK,  LavaColors_p,    CRGB::Black, 40, 500, true,  3);
}
```

---

## Palette Animation — `FadetoPaletteAnimation`

An instance of `FadeToPaletteAnimation` (defined in `CD77_slowchangepalette.h`) is created globally and handles smooth, non-blocking crossfades between colour palettes. `FastLED.show()` is called internally — no need to call it in `Performance()`.

```cpp
FadeToPaletteAnimation FadetoPaletteAnimation(leds, NUM_LEDS, BRIGHTNESS);
```

The preferred call in `Performance()` is a single line that sets the target palette, animates the crossfade, and shows — all in one:

```cpp
BETWEEN(START(0,0,0), TO(0,0,10)) { FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red)); }
BETWEEN(START(0,0,10), TO(0,0,20)) { FadetoPaletteAnimation.Update(OceanColors_p); }
```

### Key Methods

| Method | Description |
|---|---|
| `FadetoPaletteAnimation.Update(palette)` | Set target palette, animate crossfade, call `FastLED.show()` — **preferred single-line call** |
| `FadetoPaletteAnimation.Update()` | Animate toward the already-set target palette and call `FastLED.show()` |
| `FadetoPaletteAnimation.setTargetPalette(palette)` | Set a new target palette to crossfade toward without animating |
| `FadetoPaletteAnimation.setMaxChanges(1–48)` | Blend speed — higher = faster transition (default: `48`) |
| `FadetoPaletteAnimation.setUpdateInterval(ms)` | Animation refresh rate in ms (default: `100` = 10 FPS) |
| `FadetoPaletteAnimation.setMotionSpeed(speed)` | How fast the colour index animates (default: `2`) |
| `FadetoPaletteAnimation.setBrightness(0–255)` | Change brightness |
| `FadetoPaletteAnimation.reset()` | Snap immediately to black — called by `RestartPerformance()` |

---

## Matrix Fill Functions

Defined in `matrix_functions.h`. These are **non-blocking state machine functions** — call them every `loop()` iteration inside a `BETWEEN()` block and they self-throttle to the requested frame rate. Each function computes its own `beat8(speed)` offset internally, fills the entire matrix from the supplied palette, and calls `FastLED.show()` itself. No globals (`speed`, `currentPalette`) need to be set.

All 13 functions share the same call signature:

```cpp
updateXxxPalette(palette, speed, intervalMs);
```

| Parameter | Type | Description |
|---|---|---|
| `palette` | `CRGBPalette16` | Palette to colour the LEDs |
| `speed` | `uint8_t` | Animation speed fed into `beat8()` (1–255; higher = faster colour movement) |
| `intervalMs` | `uint16_t` | Milliseconds between redraws — default `20` (~50 FPS) |

| Function | Movement |
|---|---|
| `updatePhysicalOrderPalette()` | Colours follow the physical wire order |
| `updateNorthPalette()` | Colours flow downward |
| `updateSouthPalette()` | Colours flow upward |
| `updateEastPalette()` | Colours flow leftward |
| `updateWestPalette()` | Colours flow rightward |
| `updateNorthEastPalette()` | Diagonal — down-left |
| `updateNorthWestPalette()` | Diagonal — down-right |
| `updateSouthEastPalette()` | Diagonal — up-left |
| `updateSouthWestPalette()` | Diagonal — up-right |
| `updateInwardPalette()` | Colours flow inward toward the centre |
| `updateOutwardPalette()` | Colours flow outward from the centre |
| `updateClockwisePalette()` | Colours rotate clockwise |
| `updateCounterClockwisePalette()` | Colours rotate counter-clockwise |

> **Note:** Only one matrix fill function should be active at a time inside a `BETWEEN()` block — they share a single frame-rate timer (`mf_lastUpdate`).

**Example:**
```cpp
BETWEEN(START(0,0,0), TO(0,0,10)) { updateNorthPalette(Rainbow_gp, 40, 20); }
BETWEEN(START(0,0,10), TO(0,0,20)) { updateClockwisePalette(OceanColors_p, 60, 20); }
```

### Built-in Custom Palettes
*(defined in `matrix_functions.h`)*

| Name | Colours |
|---|---|
| `gb10_p` | Navy, Yellow |
| `gb9_p` | Red, Yellow |
| `gb6_p` | DarkRed, DarkBlue |
| `IceColors_p` | Black → Blue → Aqua → White |
| `hc1_p` | DarkViolet, DarkGreen, DarkRed |
| `hc2_p` – `hc5_p` | Violet/Green/Red variants |
| `HO1_p` | Red, White, Blue, White |

---

## Performance Timeline

Defined in `Time_performance.h`. The `Performance()` function is called every `loop()` and uses three macros to schedule events:

| Macro | Behaviour |
|---|---|
| `AT(h, m, s)` | Fires **once** at that timestamp — use for clears and one-shot transitions |
| `FROM(h, m, s)` | Fires **every loop()** from that timestamp onward and **never stops** — use sparingly |
| `BETWEEN(START(h1,m1,s1), TO(h2,m2,s2))` | Fires **every loop()** only within that time window — **preferred** for all continuous effects |

`BETWEEN()` is preferred over `FROM()` because it stops the effect automatically when its window ends, preventing interference with the next effect. `RestartPerformance()` resets all timers and state machines back to zero, allowing the sequence to loop cleanly.

**Example:**
```cpp
BETWEEN(START(0,0,0.000), TO(0,0,6.000))  { updatePussywillowRow(CRGB::Blue, 200, 440, false); }

AT(0,0,6.000)                              { FastLED.clear(); FastLED.show(); }

BETWEEN(START(0,0,6.300), TO(0,0,24.000)) { updatePussywillowDualRowP(RainbowColors_p, RainbowColors_p, 80, 500, 5); }

AT(0,1,08.300)                             { RestartPerformance(); }
```

---

## Notes

- All pussywillow effects share `pw_ledMap[][]` which maps visual row/column positions to physical LED indices, correctly accounting for the serpentine wiring.
- Only one pussywillow effect should be active at a time in `Performance()` — they each manage the full matrix independently.
- The `FadetoPaletteAnimation` object and a pussywillow effect **cannot** run simultaneously as they both write to the full `leds[]` array.
- `resetPussywillowEffects()` is called inside `RestartPerformance()` and resets all effect state machines to their uninitialised state (`-1`) so they re-initialise cleanly on their next call.



/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/