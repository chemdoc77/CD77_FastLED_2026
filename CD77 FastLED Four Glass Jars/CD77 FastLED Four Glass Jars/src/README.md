# Glass Jar LED Matrix — glass_jar_4jars

Four 8x8 WS2812B matrices, one per glass jar, daisy-chained together into a
single 256-LED strip on one data pin (Lolin D32, FastLED). Every effect is
a non-blocking state machine class, driven by a time-coded performance
loop with two alternate timelines to choose from — a full 28-segment demo
(`Performance()`) and a shorter, focused demo of a specific "cascade"
effect (`Performance2()`).

## File Layout

| File | Contents |
|---|---|
| `glass_jar_4jars.cpp` | Main sketch — `setup()`, `loop()`, and the includes that pull in every effect header |
| `MatrixConfig.h` | Hardware config (pin, brightness, LED type) and the `jarMatrix[][][]` LED index map |
| `RowColFill.h` | `CD77_RowFill`, `CD77_ColFill` — solid-color row/column build-up (mirrored across all 4 jars), plus the `rowFill`/`colFill` instances |
| `GradientFill.h` | `CD77_RowGradientFill`, `CD77_ColGradientFill` — gradient row/column build-up (mirrored), plus the `rowGradientFill`/`colGradientFill` instances |
| `SquareFill.h` | `CD77_SquareFill`, `CD77_SquareFillMultiColor` — concentric ring build-up (mirrored), plus the `squareFill`/`squareFillMulti` instances |
| `PerJarFill.h` | Per-jar versions of all 6 effects above, plus their `*Jar[NUM_JARS]` instance arrays — each instance controls exactly one jar independently |
| `CD77_slowchangepalette.h` | `FadeToPaletteAnimation` — smooth solid-color crossfade, mirrored across all 4 jars, plus the `FadetoPaletteAnimation` instance |
| `CD77_slowchangepalette_PerJar.h` | `FadeToPaletteAnimationPerJar` — same crossfade, but independent per jar, plus the `fadeJar[NUM_JARS]` instance array |
| `basic_functions.h` | `SolidColor`, `BlinkColor`, `ChaseOn` — simple index-based effects, plus their all-jars and per-jar instances |
| `array_functions.h` | `CD77_ArrayPaletteMotion` — 12 coordinate-driven palette motion patterns (clockwise, outward, north, etc.), plus the `paletteMotion` instance |
| `CascadeLapTracker.h` | `CascadeLapTracker` — tracks "build one jar, hold, start the next" cascade progress across laps, plus the `squareCascadeTracker`/`rowGradCascadeTracker` instances |
| `classic_effects.h` | `Twinkle`, `TheaterChase`, `CometBounce` — classic sparkle/marquee/bounce effects, plus their all-jars and per-jar instances |
| `ThreeDots.h` | `ThreeDots` — three soft-edged "balls" bouncing sinusoidally across a matrix, plus the `threeDots` instance |
| `Time_performance.h` | Time-coded show — edit `Performance()` / `Performance2()` here |

All fourteen files must live in the same `src/` folder.

## Hardware

- **MCU:** Lolin D32 (ESP32), data pin **12**
- **Matrix:** 4x 8x8 WS2812B matrices, daisy-chained in series on one data pin (256 LEDs total)
- **Brightness:** 75 (set in `MatrixConfig.h`)
- **LED index map:** `jarMatrix[jar][row][col]` in `MatrixConfig.h` converts a (jar, row, col) position into the correct physical LED index in the single 256-LED chain. Jar 0 = LEDs 0-63, jar 1 = 64-127, jar 2 = 128-191, jar 3 = 192-255, each using the same column-wise serpentine wiring, offset by 64 per jar.
- **Coordinate arrays:** `coordsX`, `coordsY`, `angles`, `radii` (declared in `glass_jar_4jars.cpp`, sized `NUM_LEDS`) describe the physical (x, y) position, angle, and radius of every LED across the whole 4-jar layout, by physical strip index — used only by `array_functions.h`'s `CD77_ArrayPaletteMotion`.

## Common Pattern

Every `CD77_*`/`FadeToPaletteAnimation*` effect class follows the same shape:

```cpp
ClassName instanceName(leds, matrixOrJarMatrix);   // constructed once, globally, right in the class's own header
instanceName.Update(<params>, runID);               // call every loop() pass, from inside a BETWEEN/AT block
```

- **`Update(...)`** does everything — no separate `Start()` call needed. It watches its own arguments internally and restarts the build-up whenever the color(s)/direction/palette change, or `runID` changes. Otherwise it just advances on its own timer.
- **`runID`** (an `unsigned long`) is what lets an effect restart correctly on the *next lap* of a repeating performance, even when called again with the exact same parameters. Pass a value that changes once per lap — this sketch uses `gPerformanceRunID` from `Time_performance.h`.
- **`isRunning()` / `isComplete()`** (on the `CD77_*Fill`/`*PerJar` classes) report whether the build-up has finished and is holding, or is still in progress.
- `basic_functions.h`'s three classes (`SolidColor`, `BlinkColor`, `ChaseOn`) and `classic_effects.h`'s three classes (`Twinkle`, `TheaterChase`, `CometBounce`) use lowercase `update()` instead, don't take a `runID`, and use their own `reset()` method (called from `RestartPerformance()`) to force a clean restart each lap.
- Once a build-up completes, it **holds** (does nothing further) until called again with different parameters or a new `runID`.

**Golden rule for this sketch:** every call into an effect class, and any other logic that touches the LEDs or depends on elapsed time, must live inside a `BETWEEN()`, `FROM()`, or `AT()` block — never as loose top-level code in `Performance()`/`Performance2()`. If you need persistent state across calls (like the cascade effects do), wrap it in its own class (see `CascadeLapTracker.h`) and call that class's methods from inside a `BETWEEN`/`AT` block, rather than scattering `static` variables and bare `if` statements directly in the timeline function.

### Where the effect instances live

Every global effect instance (`rowFill`, `fadeJar[]`, `twinkleAllJars`, etc.) is declared right inside its own class's header, immediately after the class definition(s) it belongs to — not in `glass_jar_4jars.cpp`. Each header pulls in `MatrixConfig.h` for `NUM_LEDS`/`NUM_JARS`/`jarMatrix`/etc., and declares `extern CRGB leds[NUM_LEDS];` for the shared LED array (which is still defined once, in `glass_jar_4jars.cpp`). That keeps each header self-contained and readable end-to-end: the class, then the object(s) built from it, in one place.

`glass_jar_4jars.cpp` itself only holds `leds[]`, the `coordsX`/`coordsY`/`angles`/`radii` coordinate arrays, the `#include`s that pull in every effect header (in the same order as before), `setup()`, and `loop()`.

**Caveat:** because these instances are *definitions* (not just declarations) sitting in headers guarded only by `#pragma once`, this is only safe as long as each header is included by exactly one `.cpp`/`.ino` in the whole build — true here, since everything funnels through `glass_jar_4jars.cpp`. Including one of these headers from a second `.cpp`/`.ino` in the same build would cause duplicate-symbol link errors.

---

## RowColFill.h

### `CD77_RowFill`

Lights the same row on all 4 jars at once, in a solid color, top-down or bottom-up. Rows accumulate (stay lit) as the effect progresses.

```cpp
rowFill.Update(CRGB color, unsigned long rowInterval, RowFillDirection direction, unsigned long runID);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Solid color for every row, on every jar |
| `rowInterval` | `unsigned long` | Milliseconds between each new row turning on |
| `direction` | `RowFillDirection` | `TOP_DOWN` (row 0 first, downward) or `BOTTOM_UP` (row 7 first, upward) |
| `runID` | `unsigned long` | Changes once per lap to force a restart even with identical color/direction |

### `CD77_ColFill`

Builds up vertical columns, but works through the 4 jars **sequentially** rather than mirroring: jar 0's columns 0-7 fill completely first, then jar 1's, then jar 2's, then jar 3's. Already-filled jars stay lit while later jars build up.

```cpp
colFill.Update(CRGB color, unsigned long colInterval, ColFillDirection direction, unsigned long runID);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Solid color for every column, on every jar |
| `colInterval` | `unsigned long` | Milliseconds between each new column turning on. Total steps = 4 jars x 8 columns = 32, so the full build-up takes `32 x colInterval` ms |
| `direction` | `ColFillDirection` | `LEFT_RIGHT` (within each jar: column 0 first, rightward) or `RIGHT_LEFT` (column 7 first, leftward) |
| `runID` | `unsigned long` | Changes once per lap to force a restart |

---

## GradientFill.h

### `CD77_RowGradientFill`

Same row-by-row build-up as `CD77_RowFill`, but each row is a single solid color, and that color steps through a gradient (`startColor` → `endColor`) as more rows light up — identically on all 4 jars. The first row lit = `startColor`, the last row lit = `endColor`, with rows in between blending smoothly.

```cpp
rowGradientFill.Update(CRGB startColor, CRGB endColor, unsigned long rowInterval, RowFillDirection direction, unsigned long runID);
```

| Parameter | Type | Description |
|---|---|---|
| `startColor` | `CRGB` | Color of the first row lit |
| `endColor` | `CRGB` | Color of the last row lit |
| `rowInterval` | `unsigned long` | Milliseconds between each new row turning on |
| `direction` | `RowFillDirection` | `TOP_DOWN` or `BOTTOM_UP` |
| `runID` | `unsigned long` | Changes once per lap to force a restart |

### `CD77_ColGradientFill`

Same idea, but for columns. Works through the 4 jars **sequentially** — jar 0's columns 0-7 fill first, then jar 1's, then jar 2's, then jar 3's — but the color steps through **one continuous gradient** across all 4 jars combined: jar 0's column 0 is `startColor`, jar 3's column 7 is `endColor`, blending smoothly the whole way through (not resetting/repeating per jar).

```cpp
colGradientFill.Update(CRGB startColor, CRGB endColor, unsigned long colInterval, ColFillDirection direction, unsigned long runID);
```

| Parameter | Type | Description |
|---|---|---|
| `startColor` | `CRGB` | Color of the very first column lit (jar 0, column 0) |
| `endColor` | `CRGB` | Color of the very last column lit (jar 3, column 7) |
| `colInterval` | `unsigned long` | Milliseconds between each new column turning on. Total steps = 32, so the full build-up takes `32 x colInterval` ms |
| `direction` | `ColFillDirection` | `LEFT_RIGHT` or `RIGHT_LEFT` (applies within each jar) |
| `runID` | `unsigned long` | Changes once per lap to force a restart |

---

## SquareFill.h

Both classes divide the 8x8 grid into 4 concentric square "rings," identified per-cell as `min(row, col, 7-row, 7-col)`:

- **Ring 0** = outer border (28 LEDs)
- **Ring 1** = next layer in (20 LEDs)
- **Ring 2** = next layer in (12 LEDs)
- **Ring 3** = innermost 2x2 core (4 LEDs)

### `CD77_SquareFill`

Lights the rings one at a time, all in the same solid color, either from the center outward or from the border inward — identically on all 4 jars. Rings accumulate as they light up.

```cpp
squareFill.Update(CRGB color, unsigned long ringInterval, SquareFillDirection direction, unsigned long runID);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Solid color for every ring, on every jar |
| `ringInterval` | `unsigned long` | Milliseconds between each new ring turning on |
| `direction` | `SquareFillDirection` | `CENTER_TO_OUTSIDE` (ring 3 first, outward) or `OUTSIDE_TO_CENTER` (ring 0 first, inward) |
| `runID` | `unsigned long` | Changes once per lap to force a restart |

### `CD77_SquareFillMultiColor`

Same ring build-up, but each of the 4 rings gets its **own** independent color instead of one shared color — the same per-ring colors on every jar.

```cpp
squareFillMulti.Update(const CRGB ringColors[4], unsigned long ringInterval, SquareFillDirection direction, unsigned long runID);
```

| Parameter | Type | Description |
|---|---|---|
| `ringColors` | `CRGB[4]` | `ringColors[0]` = outer ring color ... `ringColors[3]` = innermost core color |
| `ringInterval` | `unsigned long` | Milliseconds between each new ring turning on |
| `direction` | `SquareFillDirection` | `CENTER_TO_OUTSIDE` or `OUTSIDE_TO_CENTER` — each ring keeps its assigned color regardless of direction |
| `runID` | `unsigned long` | Changes once per lap to force a restart |

---

## PerJarFill.h — Independent Per-Jar Control

Every mirrored effect above has a per-jar counterpart with the exact same `Update()` signature, but instead of mirroring across all 4 jars, each instance controls exactly **one** jar — so different jars can run different colors, directions, or timings at the same time.

| Mirrored class | Per-jar class |
|---|---|
| `CD77_RowFill` | `CD77_RowFillPerJar` |
| `CD77_ColFill` | `CD77_ColFillPerJar` |
| `CD77_RowGradientFill` | `CD77_RowGradientFillPerJar` |
| `CD77_ColGradientFill` | `CD77_ColGradientFillPerJar` |
| `CD77_SquareFill` | `CD77_SquareFillPerJar` |
| `CD77_SquareFillMultiColor` | `CD77_SquareFillMultiColorPerJar` |

The header declares one array of 4 instances per effect type, each pointed at that jar's own 2D slice of `jarMatrix`:

```cpp
CD77_RowFillPerJar rowFillJar[NUM_JARS] = {
  CD77_RowFillPerJar(leds, jarMatrix[0]),
  CD77_RowFillPerJar(leds, jarMatrix[1]),
  CD77_RowFillPerJar(leds, jarMatrix[2]),
  CD77_RowFillPerJar(leds, jarMatrix[3])
};
```

Then call each jar's instance independently, every `loop()` pass:

```cpp
rowFillJar[0].Update(CRGB::Blue,   500, TOP_DOWN, gPerformanceRunID); // jar 0: blue
rowFillJar[1].Update(CRGB::Red,    500, TOP_DOWN, gPerformanceRunID); // jar 1: red
```

Nothing else changes — same parameters, same `runID` restart trick, same `isRunning()`/`isComplete()`. Any effect can run on any jar with its own parameters, and different jars can even run **different effect types** at once, since they're all independent objects.

**Important implementation detail:** unlike the mirrored classes (which call `FastLED.clear()` to wipe the whole 256-LED strip on restart), the per-jar classes only clear their *own* jar's 64 LEDs internally. This is what keeps starting/restarting one jar's effect from wiping out whatever the other three jars are currently displaying.

---

## CD77_slowchangepalette.h / CD77_slowchangepalette_PerJar.h

A different kind of effect: instead of building up rows/columns/rings across the matrix, this smoothly **crossfades a solid block of LEDs** from whatever color it's currently showing to a new target color — like a slow color-changing mood light. Both classes call `FastLED.show()` internally.

| Class / instance | Behavior |
|---|---|
| `FadeToPaletteAnimation` (`FadetoPaletteAnimation`) | **Mirrored** — one instance fades the entire 256-LED strip (all 4 jars) as a single combined block of color |
| `FadeToPaletteAnimationPerJar` (`fadeJar[]`) | **Per-jar** — index `[0]`-`[3]` fades jar 0-3 independently, each with its own color/timing, all at the same time |

### Why two separate classes?

Both classes bake brightness directly into each pixel's color (via `nscale8_video()`) instead of touching the global `FastLED.setBrightness()`. Set the overall ceiling brightness once in `setup()` with `FastLED.setBrightness(BRIGHTNESS)` and leave it alone — every fade instance's own brightness ramp then scales its own pixels relative to that fixed ceiling.

> **Fixed bug, worth knowing about:** `FadeToPaletteAnimation` originally called the *global* `FastLED.setBrightness()` directly, on the (reasonable-looking) assumption that only one instance of it ever runs at a time, so there's no fighting-over-one-global-value problem like the one `FadeToPaletteAnimationPerJar` was built to avoid. But this sketch's performance timelines run `FadetoPaletteAnimation` (mirrored) and `fadeJar[]` (per-jar) **back to back in the same lap** — e.g. `Time_performance.h`'s mirrored-fade segment followed a few seconds later by its per-jar-fade segment. `FadetoPaletteAnimation` would leave the *global* brightness scale sitting at whatever it last used when its segment ended; `fadeJar[]`'s segment then bakes its *own* brightness into the already-rendered pixel values, and that leftover global scale from the segment before gets applied a **second time** on top at render time — compounding to roughly `targetBrightness²` (out of 255) instead of `targetBrightness`. In practice this looked like: the mirrored fade's red (single-scaled) noticeably brighter than the per-jar fade's red on jar 0 moments later (double-scaled), with jars 1-3's blue/green/yellow all similarly too dim — with **no change in wiring or power injection** involved. `FadeToPaletteAnimation` now bakes brightness the same way `FadeToPaletteAnimationPerJar` always has, so neither class ever touches the global scale, and brightness stays predictable regardless of what ran before it in the timeline.

### Update() signature (identical for both classes)

```cpp
FadetoPaletteAnimation.Update(CRGBPalette16 palette, uint8_t colorBlendAmount = 24, uint16_t updateIntervalMs = 20, uint8_t brightnessFadeAmount = 8, uint16_t colorStepIntervalMs = 0, uint8_t targetBrightness = BRIGHTNESS);
```

| Parameter | Type | Description |
|---|---|---|
| `palette` | `CRGBPalette16` | Target color to fade toward, e.g. `CRGBPalette16(CRGB::Red)` |
| `colorBlendAmount` | `uint8_t` (1-255) | How far the current color moves toward the target per color step. Default 24; try 4-16 for a slow, smooth crossfade |
| `updateIntervalMs` | `uint16_t` | How often the display refreshes / brightness ramps. Default 20ms (50fps). Keep this low (20-30) for smoothness — it's not a slow-down knob |
| `brightnessFadeAmount` | `uint8_t` (1-255) | How much brightness moves toward its target per tick. Default 8; 255 snaps to target brightness instantly |
| `colorStepIntervalMs` | `uint16_t` | How often the color actually blends a step (default 0 = every tick). Raise this (150-500) to slow the color transition itself while the display still refreshes smoothly |
| `targetBrightness` | `uint8_t` (0-255) | This call's brightness ceiling. Default `BRIGHTNESS`. Pure red/green/blue/yellow don't look equally bright to the eye at the same numeric value (green reads much more luminous than red or blue; WS2812 dies also often have unequal per-channel efficiency), so different jars/colors calling `Update()` in the same `BETWEEN` block often want different values here to look visually matched — see the `fadeJar[]` example below |

**Other methods (identical for both classes):**

```cpp
.Update();                        // animate toward the already-set target, no palette argument needed
.setTargetPalette(palette);
.setColorBlendAmount(1-255);
.setUpdateInterval(ms);
.setColorStepInterval(ms);
.setBrightness(0-255);            // only for use OUTSIDE a BETWEEN block -- inside one, prefer the
                                   // targetBrightness argument on Update() itself (see above), so every
                                   // value that varies per call stays in that one call
.setBrightnessFadeAmount(1-255);
.reset(startColor = CRGB::Black); // snap instantly to startColor, brightness 0
```

Example — boosting all 4 jars' brightness targets independently, per color, all inline in the calls that are already there every frame (leaving one jar at the old default while boosting the others just makes that one look relatively darker than before, not "fixed" — every jar generally needs its own tuned target, not just the ones that looked dim):

```cpp
fadeJar[0].Update(CRGBPalette16(CRGB::Red),    3, 20, 255, 0, 150); // boosted brighter
fadeJar[1].Update(CRGBPalette16(CRGB::Blue),   3, 20, 255, 0, 150); // boosted brighter
fadeJar[2].Update(CRGBPalette16(CRGB::Green),  3, 20, 255, 0, 150); // boosted brighter
fadeJar[3].Update(CRGBPalette16(CRGB::Yellow), 3, 20, 255, 0, 150); // boosted brighter
```

`RestartPerformance()` calls `.reset()` on `FadetoPaletteAnimation` and on every `fadeJar[i]` at the top of each lap, so every fade starts the new cycle from black rather than mid-fade from wherever the last lap left off. `FastLED.setBrightness(BRIGHTNESS)` is called once, right after, as a safety-net ceiling for the whole strip — not because any `.reset()` here zeroes the global scale anymore (neither fade class touches it, per the fix above), just to guarantee the ceiling is always at its intended value regardless of what else might run.

---

## basic_functions.h — SolidColor / BlinkColor / ChaseOn

Three simple effects that don't know anything about rows/columns/rings/jars at all — each one just fills, blinks, or chases across the whole `leds[]` array (or a slice of it) by raw index.

| Class | Behavior |
|---|---|
| `SolidColor` | Fills its LED range with one solid color |
| `BlinkColor` | Blinks its LED range on/off with a color, an on-time, and an off-time |
| `ChaseOn` | Lights LEDs one at a time (by physical index within its range), then pauses and restarts from a cleared range |

Because each jar is already a contiguous 64-LED block within the shared `leds[]` array, "all jars" and "one jar" versions need no special class variants — just different constructor arguments:

```cpp
// All jars at once: operates across the full 256-LED strip.
SolidColor solidColorAllJars(leds, NUM_LEDS);

// One jar only: index 0-3 controls jar 0-3 independently.
SolidColor solidColorJar[NUM_JARS] = {
  SolidColor(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  SolidColor(&leds[1 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  SolidColor(&leds[2 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  SolidColor(&leds[3 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS)
};
// (blinkColorJar[] and chaseOnJar[] follow the identical pattern)
```

### Update() signatures

```cpp
solidColorAllJars.update(CRGB color);
blinkColorAllJars.update(CRGB color, unsigned long xOnMs, unsigned long yOffMs);
chaseOnAllJars.update(CRGB color, unsigned long xStepMs);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Color to show |
| `xOnMs` | `unsigned long` | (`BlinkColor`) how long the range stays on, ms |
| `yOffMs` | `unsigned long` | (`BlinkColor`) how long the range stays off, ms |
| `xStepMs` | `unsigned long` | (`ChaseOn`) delay before lighting the next LED, ms |

Note the method name is lowercase `update()`, not `Update()`. None of the three take a `runID` either — they use their own `reset()` method instead (called from `RestartPerformance()`).

**`ChaseOn` note:** it walks LEDs by physical strip index, not by row/column. Given all 256 LEDs (the "all jars" case), the chase sweeps through jar 0's LEDs, then straight into jar 1's, then jar 2's, then jar 3's, as one continuous 256-step chase — not four synchronized per-jar chases. Given one jar's 64-LED slice, it chases only within that jar.

---

## array_functions.h — CD77_ArrayPaletteMotion

Originally adapted from the [FastLED Mapping Demo](https://github.com/jasoncoon/led-mapper) by Jason Coon / Evil Genius Labs LLC. Folds 12 coordinate-driven palette-motion patterns into one non-blocking state machine class: for every LED, it samples a palette at an index built from a rotating time-based offset plus that LED's precomputed angle/radius/x/y coordinate.

```cpp
CD77_ArrayPaletteMotion paletteMotion(leds, NUM_LEDS, coordsX, coordsY, angles, radii);

paletteMotion.Update(CRGBPalette16 palette, PaletteMotionDirection direction, unsigned long updateIntervalMs, uint8_t speedBPM, unsigned long runID);
```

| Parameter | Type | Description |
|---|---|---|
| `palette` | `CRGBPalette16` | Palette to sample from, e.g. `gb6_p`, `RainbowColors_p`, `IceColors_p`, `gb10_p`, `gb9_p`, `gb11_p` |
| `direction` | `PaletteMotionDirection` | Which pattern: `PM_CLOCKWISE`, `PM_COUNTER_CLOCKWISE`, `PM_OUTWARD`, `PM_INWARD`, `PM_NORTH`, `PM_NORTHEAST`, `PM_EAST`, `PM_SOUTHEAST`, `PM_SOUTH`, `PM_SOUTHWEST`, `PM_WEST`, `PM_NORTHWEST` |
| `updateIntervalMs` | `unsigned long` | Non-blocking delay between *redraws*, ms — keep this low (15-30ms) for a smooth display; it's a redraw-rate throttle, not a speed control |
| `speedBPM` | `uint8_t` | Rotation speed in "beats per minute," passed straight to FastLED's `beat8()` — e.g. 60 BPM = one full palette cycle every second. This is what actually controls motion speed and keeps it smooth: `beat8()` derives its value from elapsed real time, not from how many times `Update()` has been called |
| `runID` | `unsigned long` | Changes once per lap — forces a clean restart |

This operates on the whole 256-LED strip via the `coordsX`/`coordsY`/`angles`/`radii` arrays declared in `glass_jar_4jars.cpp` — these describe one continuous physical layout spanning all 4 jars, so there's a single instance, no per-jar variant.

---

## CascadeLapTracker.h — "Build One Jar, Hold, Start The Next"

A small reusable class used by the cascade effects in `Performance2()` — jar 0 builds its animation to completion and then holds (stays fully lit); once jar 0 is done, jar 1 starts building while jar 0 remains lit; once jar 1 is done, jar 2 starts while jars 0-1 remain lit; and so on through jar 3.

### Why not just gate on isComplete()?

`isComplete()` on a `*PerJar` effect reflects "has this jar **ever** finished" — on lap 2+, jars 1/2/3 are still marked complete from the *end* of lap 1, even before their `Update()` has been called even once in the new lap. Gating purely on `isComplete()` lets that stale `true` leak through and unlock the next jar in the very first frame of the new lap, running it at the same time as jar 0's fresh rebuild. `CascadeLapTracker` fixes this by keeping its own `stage` counter that only advances forward **within the current lap**.

```cpp
CascadeLapTracker squareCascadeTracker; // declared globally in CascadeLapTracker.h
```

| Method | Description |
|---|---|
| `checkNewLap(uint32_t currentRunID)` | For cascades whose window starts at `t=0.000`. Call every frame from inside that cascade's own `BETWEEN` block. Returns `true` exactly once per lap and resets `stage()` to 0. See the note below on why `t=0.000` needs this instead of a plain `AT()`. |
| `reset()` | For cascades whose window starts at `t > 0.000`, where a normal `AT(0,0,xx.000)` block already fires correctly once per lap. Call unconditionally inside that `AT` block. |
| `stage()` | Current cascade stage: `0` = only the first jar is running, `1` = the second jar has been unlocked, `2` = the third, `3` = the fourth. |
| `advance()` | Unlocks the next jar. Call this once, guarded by `stage() == N && jarN.isComplete()`. |

### The AT(0,0,00.000) bug

`AT()` at exactly `t=0.000` can **never fire**, anywhere in this framework. `atTC()` requires `gLastTimeCodeDoneAt < tc` to trigger, but `gLastTimeCodeDoneAt` starts at (and is reset to) `0` by `RestartPerformance()` — so for `tc=0` specifically, the check is always `0 < 0`, which is never true. This does **not** affect `AT()` at any other timestamp (e.g. `AT(0,0,13.000)` works fine, since `0 < 13000` is legitimately true). `checkNewLap()` works around this by watching `gPerformanceRunID` itself change instead — it only changes once per lap (inside `RestartPerformance()`), so comparing against a remembered "last seen" value fires exactly once at the true start of every lap, including the first one.

### Usage pattern

```cpp
// Cascade whose window starts at t=0.000:
BETWEEN(START(0,0,00.000), TO(0,0,08.000)) {
  if (squareCascadeTracker.checkNewLap(gPerformanceRunID)) {
    // one-time per-lap setup, e.g. clear the waiting jars
  }
  effectJar[0].Update(...);
  if (squareCascadeTracker.stage() == 0 && effectJar[0].isComplete()) {
    squareCascadeTracker.advance();
  }
  if (squareCascadeTracker.stage() >= 1) {
    effectJar[1].Update(...);
    // ...
  }
}

// Cascade whose window starts at t > 0.000:
AT(0,0,13.000) {
  rowGradCascadeTracker.reset();
  // one-time per-lap setup, e.g. clear the waiting jars
}
BETWEEN(START(0,0,13.000), TO(0,0,24.000)) {
  effectJar[0].Update(...);
  if (rowGradCascadeTracker.stage() == 0 && effectJar[0].isComplete()) {
    rowGradCascadeTracker.advance();
  }
  // ...
}
```

---

## classic_effects.h — Twinkle / TheaterChase / CometBounce

Three more classic LED effects, following the same pattern as `basic_functions.h`: none of them know anything about jars — each one just operates on whatever contiguous LED range it's constructed with, so "all jars" vs. "one jar" is just a different constructor argument.

| Class | Behavior |
|---|---|
| `Twinkle` | Every tick, all LEDs in range fade toward black, then a new twinkle spawns at a random position with some probability. Pass `CRGB::Black` as the color for random-hue twinkles instead of one fixed color |
| `TheaterChase` | Classic marquee: every Nth LED lit in a solid color, the whole pattern shifts by one LED each step |
| `CometBounce` | A bright "head" with a fading tail, bouncing back and forth across the range (KITT-scanner style) |

```cpp
// All jars at once: operates across the full 256-LED strip.
Twinkle      twinkleAllJars(leds, NUM_LEDS);
TheaterChase theaterChaseAllJars(leds, NUM_LEDS);
CometBounce  cometBounceAllJars(leds, NUM_LEDS);

// One jar only: index 0-3 controls jar 0-3 independently.
Twinkle twinkleJar[NUM_JARS] = {
  Twinkle(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS),
  // ... jar 1-3 follow the identical pattern
};
// (theaterChaseJar[] and cometBounceJar[] follow the identical pattern)
```

### Update() signatures

```cpp
twinkleAllJars.update(CRGB color, uint8_t fadeAmount, uint8_t spawnChance, unsigned long updateIntervalMs);
theaterChaseAllJars.update(CRGB color, uint8_t spacing, unsigned long stepIntervalMs);
cometBounceAllJars.update(CRGB color, uint8_t tailFade, unsigned long stepIntervalMs);
```

| Parameter | Type | Description |
|---|---|---|
| `color` | `CRGB` | Effect color (`Twinkle` accepts `CRGB::Black` for random-hue twinkles) |
| `fadeAmount` | `uint8_t` (1-255) | (`Twinkle`) how much existing twinkles fade each tick — higher = shorter-lived sparkles |
| `spawnChance` | `uint8_t` (0-255) | (`Twinkle`) probability a new twinkle spawns each tick — higher = busier sparkle field |
| `spacing` | `uint8_t` | (`TheaterChase`) LEDs between each lit pixel |
| `tailFade` | `uint8_t` (1-255) | (`CometBounce`) how much the tail fades each tick — higher = shorter tail |
| `updateIntervalMs` / `stepIntervalMs` | `unsigned long` | Non-blocking delay between animation ticks |

Same as `basic_functions.h`: lowercase `update()`, no `runID`, and each class uses its own `reset()` method (called from `RestartPerformance()`) to force a clean restart each lap.

---

## ThreeDots.h

### `ThreeDots`

Three soft-edged circular "balls," each moving independently along a sine/cosine path so they drift and bounce around a matrix, overlapping and additively blending color where they cross. Originally written for a single small matrix (based on sutaburosu's `sdf_circle2` — https://wokwi.com/projects/329474825790685778 — modified by chemdoc77), and adapted here into the same non-blocking class style as every other effect in this sketch.

```cpp
ThreeDots threeDots(leds, NUM_JARS * MATRIX_COLS, MATRIX_ROWS); // whole 4-jar strip, treated as a 32x8 grid
threeDots.Update(); FastLED.show();                              // called every loop() pass -- FastLED.show() required, see below
```

| Method | Description |
|---|---|
| `Update()` | Advances and redraws all three balls, calls `FastLED.show()` internally. Non-blocking, throttled by its own internal `updateInterval` (default 16ms, ~60 FPS) |
| `setUpdateInterval(uint16_t ms)` | Change the redraw throttle |
| `setBall(i, xSpeed, ySpeed, radius, transition, color)` | Fully reconfigure ball `i` (0-2): horizontal/vertical motion speed, ball radius, soft-edge falloff width, and color |
| `setBallColor(i, color)` | Change just one ball's color |
| `setBallSpeed(i, xSpeed, ySpeed)` | Change just one ball's motion speed |
| `setBallSize(i, radius, transition)` | Change just one ball's radius and edge softness |
| `getBallColor(i)` | Returns ball `i`'s current color |
| `enableFPS(bool)` | Prints FPS to `Serial` periodically (requires `Serial.begin()`) — off by default |

**Differs from every other class in this sketch in four ways:**

- **No `runID` and no `reset()`.** Unlike `basic_functions.h`/`classic_effects.h` (which use `reset()` from `RestartPerformance()`) or the `CD77_*Fill` classes (which use `runID`), `ThreeDots` has no restart hook at all — its motion is a continuous function of `millis()`, so there's nothing to "restart"; it just keeps flowing smoothly across performance laps.
- **`Update()` takes no parameters.** Color/speed/size are set separately via the `setBall*()` methods (or left at their built-in defaults: a red, green, and blue ball with slightly different speeds) rather than passed into `Update()` itself.
- **`Update()` does NOT call `FastLED.show()`.** Every other effect class in this sketch calls `FastLED.show()` internally as the last step of its own `Update()`/`update()`. `ThreeDots` is the one exception — it calls `FastLED.clear()` and fills `leds[]`, but never pushes the buffer to the strip itself. **You must call `FastLED.show()` yourself, right after `threeDots.Update()`, or nothing will light up** — see the `Performance4()` usage below.
- **Own coordinate system, not `jarMatrix`.** `ThreeDots` has its own internal `XY(x, y)` mapping, built for a simple row-major serpentine matrix — it does **not** use `jarMatrix` from `MatrixConfig.h`. The `threeDots` instance is constructed as one 32-wide x 8-tall logical grid (`NUM_JARS * MATRIX_COLS x MATRIX_ROWS` = 256 LEDs, matching `NUM_LEDS`), but because this sketch's actual wiring is column-serpentine *per jar* (see `jarMatrix`), not row-serpentine across one continuous 32x8 grid, the balls will look visually scrambled/discontinuous at jar and column-group boundaries rather than tracing a smooth physical bounce. It compiles and runs fine as-is; getting it to look physically correct on this hardware would require rewriting `XY()` to index through `jarMatrix` instead of its own formula.

---

## Time_performance.h

Based on Mark Kriegsman's Time Performance sketch (https://gist.github.com/kriegsman/a916be18d32ec675fea8), extended with `BETWEEN()`/`START()`/`TO()` for time-windowed effects.

| Macro | Behavior |
|---|---|
| `AT(h, m, s)` | Fires **once**, the first `loop()` where the timeline reaches that timestamp — use for clears and one-shot transitions. **Cannot fire at `t=0.000` exactly** — see `CascadeLapTracker.h` above for the workaround. |
| `FROM(h, m, s)` | Fires **every loop()** from that timestamp onward and **never stops** — use sparingly |
| `BETWEEN(START(h1,m1,s1), TO(h2,m2,s2))` | Fires **every loop()** only within that time window — **preferred** for all continuous effects |

`RestartPerformance()` resets the timeline back to `t = 0`, increments `gPerformanceRunID` (which lets every `CD77_*Fill` effect restart cleanly each lap), calls `.reset()` on the palette-fade, basic-effect, and classic-effect classes, and restores `FastLED.setBrightness(BRIGHTNESS)` once as a fixed ceiling for the whole strip (see `CD77_slowchangepalette.h`'s "Why two separate classes?" note — no effect class in this sketch touches the global brightness scale itself anymore).

### Two timelines, two purposes

The file defines **two** performance functions. `glass_jar_4jars.cpp`'s `loop()` calls whichever one you want active:

```cpp
void loop() {
  gTimeCode = millis() - gTimeCodeBase;
  performanceUpdate.Update(Performance4); // swap in Performance/Performance2/Performance3 to run a different timeline
}
```

#### `Performance()` — the full demo, 28 segments, 116-second loop

Cycles through every effect in the sketch: all 6 mirrored effects, a per-jar row-fill demo, both palette-fade effects (mirrored then per-jar), all 3 `basic_functions.h` effects (each mirrored then per-jar), a per-jar multi-color-rings demo, and all 12 `array_functions.h` palette-motion directions. Good for seeing everything the sketch can do, one after another.

#### `Performance2()` — focused demo of the cascade pattern, 34-second loop

A shorter timeline built to showcase `CascadeLapTracker`'s "build one jar, hold, start the next" effect specifically:

```
0- 8s : squareFillMultiJar cascade  (jar 0 rings build & hold, then jar 1, then jar 2, then jar 3)
8-13s : colFill                     (mirrored, solid red columns)
13-24s: rowGradientFillJar cascade  (same cascade pattern, using per-jar row gradients)
24-29s: colGradientFill             (mirrored, Red -> Blue columns)
29-34s: squareFill                  (mirrored, Purple rings)
34s   : RestartPerformance() -- loop back to t=0
```

Edit the `TC` timestamps in either function to change how long each effect runs, add more `BETWEEN()` blocks for additional effects, or swap in different colors/intervals/directions for any jar. If you add a new cascade effect, follow the `CascadeLapTracker` pattern above rather than adding loose `static` variables or bare `if` statements to the timeline function.

### PerformanceSelector — chaining multiple shows in `loop()`

`Time_performance.h` also defines more standalone timeline functions than just `Performance()`/`Performance2()` — e.g. `Performance6()`, `Performance7()`, etc., each its own self-contained timeline with its own `AT(...)  { RestartPerformance(); }` at the end. `PerformanceSelector` (bottom of `Time_performance.h`, single global instance `performanceUpdate`) lets you chain several of them into a rotation, one show at a time, each running to the end of its own lap before handing off to the next:

```cpp
void loop() {
  gTimeCode = millis() - gTimeCodeBase;
  performanceUpdate.Update(Performance6); // runs to the end of its own lap, then...
  performanceUpdate.Update(Performance7); // ...hands off to this one, then back to Performance6, wrapping around
}
```

Call `performanceUpdate.Update(...)` once per `loop()` pass **for each** show you want in the rotation, in the same order every time. The first show named runs every frame until its own internal `RestartPerformance()` fires; only then does control hand off to the next show named that frame, and so on, wrapping back to the first once every named show has had a turn. With only one `Update()` call, that one show just loops forever, same as before `PerformanceSelector` existed.

**Gotcha this fixes:** `gTimeCode` is recomputed **inside** `PerformanceSelector::Update()`, immediately before it runs whichever show is currently active — not just once at the top of `loop()`. This matters because a hand-off can happen mid-`loop()`-pass: if Performance6 finishes and calls `RestartPerformance()` (resetting `gTimeCodeBase`), the very next `Update()` call in that same `loop()` pass (for Performance7) needs a `gTimeCode` computed against the *new* `gTimeCodeBase` — otherwise Performance7 would run its very first frame using a stale, already-fully-elapsed `gTimeCode` left over from Performance6, causing its own end-of-lap `AT(...)` to fire immediately and hand control right back before Performance7 ever visibly ran. (This is exactly what "only Performance6 runs, and it just repeats" looks like.) The `gTimeCode = millis() - gTimeCodeBase;` line still in `loop()` is harmless to leave in — it just initializes things before the very first `Update()` call of all — but it's the one inside `Update()` that actually matters once shows start handing off to each other.
