# Halloween2026

Non-blocking FastLED Halloween light show for a **Lolin D32 (ESP32)**.
256 x WS2812B (`NEOPIXEL`) on **GPIO 12**, FastLED brightness **75**.

Converted from `Halloweend2026.ino`. No effect ever calls `delay()`; every effect is a
small state machine with an `update()` call, and the show timeline uses `BETWEEN`.

## Files (`src/`)

| File | What it holds |
|---|---|
| `Halloweend2026.cpp` | `setup()` / `loop()` (the old `Halloweend2026.ino`, same name). `setup()` does the one-time `delay(1000)` power-up hold; `loop()` sets `gTimeCode` and calls the show you pick (`Performance12();` at the moment) |
| `Config.h` | Pin, LED count, brightness, power limits |
| `MapData.h` | `coordsX`, `coordsY`, `angles`, `radii` position maps (unchanged data) |
| `Flicker.h` | `CD77_flicker` random on/off flicker |
| `Halloween_chase.h` | `CD77_Halloween_Colors` random spooky color, `CD77_Chase_Halloween_fixed`, `_random` and `_fixed_color` chases |
| `Fire2012withPalette.h` | `Fire2012WithPalette` fire effect with any palette |
| `RopeMap.h` | Rope matrix layout generated from `rope_matrix_fixed.xlsx` (column, sheet row, height of every LED) |
| `Rope_Fire2012.h` | `Fire2012Rope` fire laid out on the rope matrix |
| `ropestuff.h` | `CD77_palette` 12-direction palette sweeps, `CD77_blackout`, the `hc1_p`..`hc5_p` and `IceColors_p` palettes |
| `CD77_slowchangepalette.h` | `FadeToPaletteAnimation` class (instance `FadetoPaletteAnimation`, declared in the `.cpp`): smooth time-based whole-strip colour crossfade |
| `CD77_palettecrossfade.h` | `CD77_palettecrossfade` (after Mark Kriegsman's PaletteCrossfade): a moving palette pattern, using the same 12 `PAL_` directions as `CD77_palette`, whose palette smoothly crossfades toward the target palette you pass in. Needs `ropestuff.h` |
| `LED_Effects.h` | `SolidFill`, `Blink`, `ColorWipeFill`, `ColorWipeDot` range effects with numbered instances, each with `ForceInactive(n)` and `ForceInactiveAll()`; all `Blink` instances share one clock, and `ColorWipeFill` / `ColorWipeDot` share another, so each group stays in lockstep |
| `Time_performance.h` | Timing macros, `ResetAllEffects()` / `RestartPerformance()` and all the shows (`Performance()`, `Performance2()`, `Performancex2()`, `Performancex3()`, `Performance4c()`, `Performance5()`, `Performance6()`, `Performance7()`, `Performance8()`) |

## Build and upload

PlatformIO (recommended): open the folder, then `pio run -t upload`.
`platformio.ini` targets `lolin_d32` and pulls in FastLED. For the Arduino IDE, put the
`src/` files in a sketch folder with an `.ino` stub, because `.cpp` sketches need PlatformIO or a stub.

## Cliff notes

### Timing macros (hours, minutes, seconds; seconds can have decimals)

| Macro | Meaning |
|---|---|
| `BETWEEN(h1,m1,s1, h2,m2,s2) { }` | Runs every pass while `start <= time < end`, then stops by itself |
| `BETWEEN_ENTERED()` | True only on the first pass inside a window (one-shot setup) |
| `AT(h,m,s) { }` | Runs once when the clock passes that time (used for restart) |
| `FROM(h,m,s) { }` | Old behaviour, runs until a later `FROM` takes over (kept for compatibility) |

FROM to BETWEEN conversion rule: each `FROM(start)` became `BETWEEN(start, nextStart)`;
the last one ends at the `AT(...)` restart time.

### Effect update functions (call every pass inside a `BETWEEN` block)

| Call | What it does |
|---|---|
| `CD77_flicker.update(color, minMs, maxMs)` | Random on/off flicker, wait between flips is random in min..max ms |
| `CD77_Halloween_Colors.update(waitMs)` | Picks a new random orange/red/yellow every `waitMs` |
| `CD77_Chase_Halloween_fixed.update(waitMs, dots)` | Chase in the spooky color; a burst of `dots` pixels every `waitMs` |
| `CD77_Chase_Halloween_random.update(minMs, maxMs, dots)` | Same, burst period re-rolled each time |
| `CD77_Chase_Halloween_fixed_color.update(color, waitMs, dots)` | Chase in a chosen color |
| `Fire2012WithPalette.update(palette, cooling, sparking, fps=60)` | Fire2012. Cooling 20-100 (less = taller), sparking 50-200 (more = roaring) |
| `Fire2012Rope.update(palette, cooling, sparking, intervalMs=20)` | Fire2012 on the rope matrix: each of the 10 columns is its own chimney, sparks start at each column's bottom LED. 4th argument is ms between frames (like the Fibonacci `updateFibonacciFire`) |
| `CD77_palette.update(dir, palette, speed, frameMs=16)` | Sweep; `dir` is `PAL_NORTH`, `PAL_SOUTH`, `PAL_INWARD`, `PAL_OUTWARD`, `PAL_CLOCKWISE`, ... |
| `CD77_blackout.update()` | All LEDs off |
| `CD77_palettecrossfade.update(dir, palette, fade=6, speed=20, repeat=1)` | Moving palette pattern whose colours crossfade toward `palette` (`nblendPaletteTowardPalette`). `dir` as for `CD77_palette`. `fade` 1-12 sets how fast the colours blend into a new palette (bigger = faster), `speed` is the pattern speed in beats per minute like `CD77_palette` (0 = still), `repeat` 1-8 repeats the palette across the display. Only `dir` and `palette` are required. See below |
| `FadetoPaletteAnimation.Update(palette, fadeMs=1500, frameMs=20)` | Smooth crossfade of the WHOLE strip (one solid colour) from its current colour to the palette's slot-0 colour, so pass a flat palette such as `CRGBPalette16(CRGB::Red)`. `fadeMs` = how long a full fade takes (time-based, always finishes in that time), `frameMs` = ms between frames. Both optional. A new palette mid-fade re-targets without a pop |
| `SolidFill::Update(leds, n, {color, from, to, waitMs, fwd})` | Fills the range and keeps it; repaints the same colour every `waitMs` as a keep-alive (0 = once). `to` is exclusive |
| `Blink::Update(leds, n, {color, from, to, waitMs, fwd})` | ON/OFF every `waitMs`, ON first (`fwd` ignored). All instances share one start time, so same-`waitMs` instances stay in lockstep (see below) |
| `ColorWipeFill::Update(leds, n, {color, from, to, waitMs, fwd})` | Fills one LED per `waitMs`, leaves them lit, then clears and restarts. Shares a clock with `ColorWipeDot` (see below) |
| `ColorWipeDot::Update(leds, n, {color, from, to, waitMs, fwd})` | One dot travels the range, one LED per `waitMs`. Shares a clock with `ColorWipeFill` (see below) |

`n` is the 1-based instance number. One instance runs only one wipe/dot/fill at a time, so give
simultaneous effects of the same type different instance numbers.

**Synchronized `Blink`:** every `Blink` instance shares one start time and toggles on a fixed grid of
`waitMs` steps from it, so instances with the same `waitMs` blink together and never drift apart.
Nothing extra is needed to start several at the same moment, as in `Performance6()`:

```cpp
BETWEEN(0,0,08.200,  0,0,13.000) { Blink::Update(leds, 1, {CRGB::Blue,    0,  85, 300, true});
                                   Blink::Update(leds, 2, {CRGB::Red,    85, 170, 300, true});
                                   Blink::Update(leds, 3, {CRGB::Green, 170, 256, 300, true}); }
```

A `Blink` instance that has not been called for `BLINK_RESTART_GAP_MS` (100 ms, set in
`LED_Effects.h`) counts as stopped, for example after a blackout window. The next time Blinks are
used, they all start together, ON first, on a fresh shared clock, so a leftover instance from an
earlier window never keeps its old phase. Windows that follow each other with no gap just carry on.

**Synchronized `ColorWipeFill` / `ColorWipeDot`:** these two share one clock and step on a fixed grid
of `waitMs` ticks, so any mix of wipes and dots with the same `waitMs` advance in lockstep and never
drift apart. Again nothing extra is needed in the show:

```cpp
BETWEEN(0,0,00.000,  0,0,10.000) { ColorWipeFill::Update(leds, 1, {CRGB::Blue,   0,  40, 20, true});
                                   ColorWipeFill::Update(leds, 2, {CRGB::Red,   40,  80, 20, true});
                                   ColorWipeDot::Update (leds, 1, {CRGB::Green, 80, 120, 20, true}); }
```

An instance not called for `WIPE_RESTART_GAP_MS` (100 ms, in `LED_Effects.h`) counts as stopped, so
wipes used again after a blackout window all start together at their first LED. Before this, an
instance with unchanged settings carried on mid-wipe. Wipes over ranges of different lengths still finish
and loop at different times.

Each `Update()` ends in a `FastLED.show()` of about 8 ms, so on the strip the second and third
instance change about 8 and 16 ms after the first. That is data transfer time, not drift, and is not
visible. An instance that starts (or is reset with `Blink::ForceInactive(n)`) while others are blinking
rejoins them in the matching phase.

**`CD77_palettecrossfade`:** the class keeps its own current palette and nudges it toward the target
palette you pass in, while a colour pattern keeps moving. Change the palette in the next `BETWEEN`
window and the colours blend into it with no pop; keep the same palette and it just keeps moving.
The first call after a gap (more than `PALXFADE_RESTART_GAP_MS`, 100 ms, for example after a blackout
window) starts fresh on the target palette. To fade IN from a chosen palette, call
`CD77_palettecrossfade.startFrom(palette)` before the first `update()` of the window. Slow blend:
`fade` 1. The pattern keeps its place from window to window, so changing `speed` or `dir` never makes it jump. The crossfade step (10 ms) and frame time (20 ms) are constants at the top of `CD77_palettecrossfade.h`. See `Performance8()`:

```cpp
BETWEEN(0,0,00.000,  0,0,08.000) { CD77_palettecrossfade.update(PAL_NORTH, hc5_p,      6, 18); }
BETWEEN(0,0,08.000,  0,0,16.000) { CD77_palettecrossfade.update(PAL_NORTH, IceColors_p, 1, 18); }  // slow blend hc5 -> ice
```

Chase functions take an optional last argument: ms between frames (default 50).

### Selecting a show

`loop()` in `Halloweend2026.cpp` calls the show by its real name, as the original did:

```cpp
void loop() {
  gTimeCode = millis() - gTimeCodeBase;
  Performance12();   // or Performance(), Performance2(), Performancex2(), Performancex3(), Performance4c(), Performance5(), Performance6(), Performance7(), Performance8()
}
```

| Function | Length |
|---|---|
| `Performance6()` | 27.2 s |
| `Performance()` | 52.2 s |
| `Performance2()` | 12 s |
| `Performancex2()` | 104.2 s |
| `Performancex3()` | 60.2 s |
| `Performance4c()` | 12 s |
| `Performance5()` | 95.2 s |
| `Performance7()` | 95.2 s |
| `Performance8()` | 48.2 s (`CD77_palettecrossfade` demo) |

### Example: add your own show

```cpp
void MyShow() {
  BETWEEN(0,0,00.000,  0,0,10.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 30); }
  BETWEEN(0,0,10.000,  0,0,20.000) { CD77_Halloween_Colors.update(600);
                                     CD77_Chase_Halloween_fixed.update(700, 8); }
  BETWEEN(0,0,20.000,  0,0,30.000) { Fire2012WithPalette.update(HeatColors_p, 75, 80); }
  BETWEEN(0,0,30.000,  0,0,30.200) { CD77_blackout.update(); }
  AT(0,0,30.200)                   { RestartPerformance(); }
}
```
Then call `MyShow();` from `loop()` in place of `Performance5();`.
Keep windows from overlapping, and end the last one where `AT()` restarts.

## Behaviour changes to be aware of

- **Brightness 75**: set once in `setup()`. The per-show `FastLED.setBrightness(BRIGHTNESS)` calls were removed.
- **Spooky colors**: the old `map(..., 0, BRIGHTNESS)` scaling was removed. FastLED's global brightness already dims the output, and keeping both would dim twice at 75.
- **Palette/offset/speed globals** are gone: palette, direction and speed are now `CD77_palette.update()` arguments. In `Performancex3` the palette is now set before the sweep is drawn (the old code set it after).
- **TC() rounds to the nearest ms**, so times like `39.3` do not truncate to 39299.
- **Blackout windows** replace the repeated `FastLED.clear(); FastLED.show();` lines; it re-shows black every 50 ms.
- Effects keep their state across back-to-back windows (e.g. the chase keeps its cadence when the color changes); `RestartPerformance()` resets all of them.
- **`FadetoPaletteAnimation` instance**: declared in `Halloweend2026.cpp` right after its header is included (the header expects it there).
- **`CD77_palettecrossfade`**: `update()` copies the target palette into a private non-const variable before blending, because FastLED's `nblendPaletteTowardPalette()` does not accept a `const` target.
- **Startup hold**: `delay(1000)` stays in `setup()` and runs once at power-up. Every effect and the show timeline are non-blocking; the only thing that takes time in `loop()` is `FastLED.show()` itself (about 8 ms for 256 LEDs).

## Restart behaviour (`RestartPerformance()` / `ResetAllEffects()`)

`RestartPerformance()` zeroes the clock and calls `ResetAllEffects()`, which now covers every effect:

- Older effects (flicker, chases, both fires, palette sweep, palette crossfade, blackout): `.reset()`.
- `FadetoPaletteAnimation.forceInactive()`: back to black with no fade in progress.
- `SolidFill::ForceInactiveAll()`, `Blink::ForceInactiveAll()`, `ColorWipeFill::ForceInactiveAll()`, `ColorWipeDot::ForceInactiveAll()`: every instance (1 through its `_COUNT`) is marked "not started".

Without the last two groups, those effects would skip repainting or resume mid-animation on the second
and later laps. `ForceInactiveAll()` means a new instance number used in a future show needs no extra
line in `ResetAllEffects()`. All of these are state-only: no LED paint and no `FastLED.show()`, so
calling several back-to-back cannot make the next real paint fail to latch (the older `reset()` of
`FadeToPaletteAnimation` and the `Reset()` of the `LED_Effects` classes do call `show()`).

## Hardware notes

- GPIO 12 is an ESP32 strapping pin (flash voltage at boot). It usually works as a data pin, but if the board will not boot with the strip attached, try another pin (13, 25, 26, 27) by changing `LED_PIN` in `Config.h`.
- Tie the LED power supply ground to the D32 ground.
- `FastLED.setMaxPowerInVoltsAndMilliamps(5, 8500)` limits current; change `MAX_MILLIAMPS` in `Config.h` for your supply.

## Rope matrix fire

`Fire2012Rope` uses the spreadsheet map (`RopeMap.h`), not the LED number order, so flames rise
the right way on both the up-going and down-going side of every strand. Columns are 22-29 LEDs tall
with their tops aligned (sheet row 1), so each column ignites at its own bottom LED. Example:

```cpp
BETWEEN(0,0,00.000,  0,0,15.000) { Fire2012Rope.update(HeatColors_p, 75, 80); }
BETWEEN(0,0,15.000,  0,0,30.000) { Fire2012Rope.update(IceColors_p,  75, 80); }
```
