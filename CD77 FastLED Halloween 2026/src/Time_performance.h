#pragma once
/* This code is based on the Time Performance sketch by Mark Kriegsman of FastLED at:
   https://gist.github.com/kriegsman/a916be18d32ec675fea8

   ===========================================================================
   CLIFF NOTES
   ===========================================================================
   Every effect is a non-blocking state machine with an update() call. Nothing
   in this file ever calls delay(). In loop(), set the clock and call ONE show:
       gTimeCode = millis() - gTimeCodeBase;
       Performance5();

   STARTUP HOLD
   ---------------------------------------------------------------------------
   The 1 s power-up hold is delay(1000) at the top of setup(). It runs once at
   start-up only, never in loop(). setup() ends with RestartPerformance(), so the
   show clock starts at 0 right after the hold. The full loop() is just:
       void loop() {
         gTimeCode = millis() - gTimeCodeBase;
         Performance5();
       }

   TIMING MACROS  (hours, minutes, seconds - seconds may have decimals)
   ---------------------------------------------------------------------------
   BETWEEN(h1,m1,s1, h2,m2,s2) { ... }
       Runs the block on every loop pass while  start <= time < end.
       The block stops running by itself at the end time. This replaces FROM
       (which kept running until a later FROM took over).
   BETWEEN_ENTERED()
       True on the FIRST pass inside a BETWEEN window only. Use for one-shot
       setup:  BETWEEN(...) { if (BETWEEN_ENTERED()) Fire2012WithPalette.reset(); ... }
   AT(h,m,s) { ... }
       Runs the block ONCE when the clock passes that time. Used for
       RestartPerformance() at the end of each show.
   FROM(h,m,s) { ... }
       Old behaviour (runs until a later FROM). Kept for compatibility.

   EFFECT UPDATE FUNCTIONS (call every pass inside a BETWEEN block)
   ---------------------------------------------------------------------------
   Each line shows the call, what the arguments mean, and a real example.

   CD77_flicker.update(color, minMs, maxMs)
       Random on/off flicker; the wait between flips is random from minMs to maxMs.
       CD77_flicker.update(CRGB::OrangeRed, 5, 30);

   CD77_Halloween_Colors.update(waitMs)
       Picks a new random orange/red/yellow every waitMs. It only picks the color:
       the two spooky-color chases below use it, so call it in the same block.
       CD77_Halloween_Colors.update(600);

   CD77_Chase_Halloween_fixed.update(waitMs, dots [, frameMs=50])
       Chase in the spooky color; a burst of 'dots' pixels every waitMs.
       CD77_Halloween_Colors.update(600);
       CD77_Chase_Halloween_fixed.update(300, 3);

   CD77_Chase_Halloween_random.update(minMs, maxMs, dots [, frameMs=50])
       Same, but the time between bursts is re-rolled from minMs to maxMs each burst.
       CD77_Halloween_Colors.update(600);
       CD77_Chase_Halloween_random.update(300, 900, 3);

   CD77_Chase_Halloween_fixed_color.update(color, waitMs, dots [, frameMs=50])
       Chase in a color you choose.
       CD77_Chase_Halloween_fixed_color.update(CRGB::Purple, 700, 8);

   Fire2012WithPalette.update(palette, cooling, sparking [, fps=60])
       Fire2012 along the LED numbers. cooling 20-100 (higher = shorter flames),
       sparking 50-200 (higher = busier). 4th argument is frames per second.
       Fire2012WithPalette.update(HeatColors_p, 75, 80);
       Fire2012WithPalette.update(IceColors_p, 75, 80, 30);

   Fire2012Rope.update(palette, cooling, sparking [, intervalMs=20])
       Fire2012 laid out on the rope matrix (10 columns, flames start at the bottom
       LED of each column, layout from RopeMap.h). cooling and sparking work as above.
       4th argument is the MILLISECONDS between frames (20 = about 50 FPS).
       Fire2012Rope.update(HeatColors_p, 75, 80);
       Fire2012Rope.update(IceColors_p, 75, 80, 30);

   CD77_palette.update(dir, palette, speed [, frameMs=16])
       Palette sweep. dir is one of PAL_NORTH, PAL_NORTH_EAST, PAL_EAST,
       PAL_SOUTH_EAST, PAL_SOUTH, PAL_SOUTH_WEST, PAL_WEST, PAL_NORTH_WEST,
       PAL_CLOCKWISE, PAL_COUNTER_CLOCKWISE, PAL_INWARD, PAL_OUTWARD.
       speed is beats per minute (higher = faster).
       CD77_palette.update(PAL_NORTH, IceColors_p, 40);
       CD77_palette.update(PAL_INWARD, hc5_p, 30);

   CD77_palettecrossfade.update(dir, palette [, fade=6, speed=20, repeat=1])
       (CD77_palettecrossfade.h, after Mark Kriegsman's PaletteCrossfade)
       A moving colour pattern, like CD77_palette.update(), whose palette crossfades
       smoothly. The class keeps its own currentPalette and nudges it toward the
       palette you pass in (nblendPaletteTowardPalette). Pass a DIFFERENT palette in
       the next BETWEEN window and the colours blend into it while the pattern keeps
       moving, with no pop. Pass the same palette and it just keeps moving.
         dir      same PAL_ directions as CD77_palette.update()
         palette  target palette to crossfade toward
         fade     1-12, how fast the colours blend into the new palette
                  (1 = very slow, 6 = a few seconds, 12 = fastest). Bigger = faster.
         speed    pattern speed in beats per minute, same meaning as the speed of
                  CD77_palette.update() (default 20, 0 = pattern stands still)
         repeat   1-8, how many times the palette repeats across the display
       Only dir and palette are required; fade, speed and repeat are optional, in
       that order. The crossfade step time (10 ms) and frame time (20 ms) are
       constants at the top of CD77_palettecrossfade.h (PALXFADE_BLEND_MS,
       PALXFADE_FRAME_MS). The pattern keeps its place between windows, so changing
       speed or direction never makes it jump.
       The first call of a window that follows a gap (e.g. after a blackout) starts
       fresh on the target palette (no blend); a window that follows another
       with no gap carries on and crossfades. For a slow blend use fade 1.
       CD77_palettecrossfade.update(PAL_NORTH, hc5_p);
       CD77_palettecrossfade.update(PAL_INWARD, IceColors_p, 3, 30);
       CD77_palettecrossfade.update(PAL_CLOCKWISE, hc2_p, 6, 18, 2);
       CD77_palettecrossfade.startFrom(palette)  optional, call before the first
       update() of a window to crossfade IN from that palette.

   CD77_blackout.update([frameMs=50])
       All LEDs off. Use it between sections.
       CD77_blackout.update();

   FadetoPaletteAnimation.Update(palette [, fadeDurationMs=1500, updateInterval=20])
       (CD77_slowchangepalette.h)
       Smooth, time-based crossfade of the WHOLE strip, one solid colour, from the
       colour it is showing now to the palette's slot-0 colour. A new palette given
       mid-fade re-targets from the current colour (no pop). Calls FastLED.show().
         palette         the colour to fade toward. Only the palette's FIRST entry
                         (slot 0) is used, so pass a flat one-colour palette such as
                         CRGBPalette16(CRGB::Red). A multi-colour palette like
                         OceanColors_p gives just its first colour.
         fadeDurationMs  how long a full crossfade takes, in ms (default 1500 = 1.5 s).
                         Time-based, so it always finishes in this time however often
                         Update() is called. Bigger = slower fade.
         updateInterval  ms between animation frames (default 20 = about 50 FPS).
                         Smaller = smoother but more FastLED.show() calls.
       Both timing arguments are optional. Leave them off for 1500 ms and 20 ms (set
       by DEFAULT_FADE_DURATION_MS and DEFAULT_UPDATE_INTERVAL_MS in the class).
       FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red), 1500, 20);
       FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Blue));          // defaults
       FadetoPaletteAnimation.Update(OceanColors_p, 3000, 20);            // slow 3 s fade
       Other methods: Update() (no arguments: animate toward the current target),
       setTargetPalette(p), setFadeDuration(ms), setUpdateInterval(ms),
       setBrightness(0-255), reset() (paints black + show()),
       forceInactive() (state-only reset, used by ResetAllEffects).

   LED_Effects.h  (static classes; 2nd argument is the 1-BASED INSTANCE number;
                   params are {color, fromLed, toLed, waitMs, forward}; toLed is exclusive)
       SolidFill::Update    (leds, 1, {CRGB::Red,    0,  24, 500, true});
           Paints the range and keeps it; repaints the SAME colour every waitMs
           as a keep-alive (waitMs 0 = paint once). Never blinks.
       Blink::Update        (leds, 1, {CRGB::Yellow, 0, 128, 300, true});
           ON/OFF every waitMs, ON first. 'forward' is ignored.
           SYNCHRONIZED: all Blink instances share one start time and toggle on a
           fixed grid of waitMs steps from it, so instances with the same waitMs
           blink in lockstep and never drift apart. Nothing extra is needed to start
           several together, just call them in the BETWEEN window (see Performance6):
             BETWEEN(...) { Blink::Update(leds, 1, {CRGB::Blue,    0,  85, 300, true});
                            Blink::Update(leds, 2, {CRGB::Red,    85, 170, 300, true});
                            Blink::Update(leds, 3, {CRGB::Green, 170, 256, 300, true}); }
           An instance that has not been called for BLINK_RESTART_GAP_MS (100 ms,
           in LED_Effects.h) counts as stopped, so each time Blinks start again,
           e.g. after a blackout window, they all begin together, ON first.
           Windows that follow each other with no gap just carry on.
           (Each Update() ends in a ~8 ms FastLED.show(), so on the strip the second
           and third instance light up ~8 and ~16 ms after the first: that is the
           time to send the data, not drift, and is not visible.)
       ColorWipeFill::Update(leds, 1, {CRGB::Blue,  24,  48,  20, true});
           Fills one LED per waitMs, leaves them lit, clears and restarts.
       ColorWipeDot::Update (leds, 1, {CRGB::Green, 48,  72,  20, false});
           One dot moves through the range, one LED per waitMs.
       SYNCHRONIZED WIPES: ColorWipeFill and ColorWipeDot share ONE start time and
           step on a fixed grid of waitMs ticks from it, so any mix of wipes and
           dots with the same waitMs advance in lockstep and never drift apart. Just
           call them in the BETWEEN window, nothing extra is needed:
             BETWEEN(...) { ColorWipeFill::Update(leds, 1, {CRGB::Blue,   0, 40, 20, true});
                            ColorWipeFill::Update(leds, 2, {CRGB::Red,   40, 80, 20, true});
                            ColorWipeDot::Update (leds, 1, {CRGB::Green, 80,120, 20, true}); }
           An instance not called for WIPE_RESTART_GAP_MS (100 ms, in LED_Effects.h)
           counts as stopped, so each time wipes start again, e.g. after a blackout
           window, they all begin together at their first LED. (Before, an instance
           with unchanged settings carried on mid-wipe.) Windows that follow each
           other with no gap just carry on. Ranges of different lengths still finish
           and loop at different times. Each Update() ends in a ~8 ms show(), so the
           2nd/3rd instance change ~8/16 ms after the 1st; that is not drift.
       Rules: ONE instance can only run ONE wipe/dot/fill at a time. Two
       simultaneous wipes of the same type need DIFFERENT instance numbers.
       Instance counts are SOLID_FILL_COUNT, BLINK_COUNT, COLOR_WIPE_FILL_COUNT
       and COLOR_WIPE_DOT_COUNT (4 each) in LED_Effects.h; raise them for more.

   RESTART / RESET (what RestartPerformance() -> ResetAllEffects() does)
       Every effect above is returned to a fresh start at the end of each lap:
         * the older effects (flicker, chases, fires, palette, palettecrossfade,
           blackout) with .reset()
         * FadetoPaletteAnimation.forceInactive()
         * SolidFill / Blink / ColorWipeFill / ColorWipeDot ::ForceInactiveAll()
       The last two groups are STATE-ONLY (no LED paint, no FastLED.show()), so
       stacking them back-to-back cannot make the next real paint fail to latch.
       ForceInactiveAll() covers every instance number, so you never have to add
       a line here when you use a new instance in a show. (A single instance can
       still be reset with e.g. ColorWipeFill::ForceInactive(2).)

   Palettes you can pass: HeatColors_p, PartyColors_p, IceColors_p, hc1_p ... hc5_p,
   or your own, e.g.
   CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White).

   SHOW CONTROL
   ---------------------------------------------------------------------------
   Performance();  Performance2();  Performancex2();  Performancex3();
   Performance4c();  Performance5();  Performance6();  Performance7();  Performance8();
   the shows: call ONE of them from loop().
                                    (Performance8 demos CD77_palettecrossfade.)
   RestartPerformance();            restart the clock + reset every effect.

   EXAMPLE 1 - a 30 second show (flicker, chase, fire)
   ---------------------------------------------------------------------------
   void MyShow() {
     BETWEEN(0,0,00.000,  0,0,10.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 30); }
     BETWEEN(0,0,10.000,  0,0,20.000) { CD77_Halloween_Colors.update(600);
                                        CD77_Chase_Halloween_fixed.update(700, 8); }
     BETWEEN(0,0,20.000,  0,0,30.000) { Fire2012WithPalette.update(HeatColors_p, 75, 80); }
     BETWEEN(0,0,30.000,  0,0,30.200) { CD77_blackout.update(); }
     AT(0,0,30.200)                   { RestartPerformance(); }
   }

   EXAMPLE 2 - a 45 second rope show (fire, ice fire, then a palette sweep)
   ---------------------------------------------------------------------------
   void MyRopeShow() {
     BETWEEN(0,0,00.000,  0,0,15.000) { Fire2012Rope.update(HeatColors_p, 75, 80); }
     BETWEEN(0,0,15.000,  0,0,30.000) { Fire2012Rope.update(IceColors_p,  75, 80, 30); }
     BETWEEN(0,0,30.000,  0,0,45.000) { CD77_palette.update(PAL_NORTH, hc5_p, 30); }
     BETWEEN(0,0,45.000,  0,0,45.200) { CD77_blackout.update(); }
     AT(0,0,45.200)                   { RestartPerformance(); }
   }

   To run either one, put its name in loop() in place of Performance5():
       void loop() {
         gTimeCode = millis() - gTimeCodeBase;
         MyRopeShow();
       }
   Rules: windows must not overlap, they must start where the last one ended, and
   the last BETWEEN should end where AT() restarts. Define your show ABOVE where
   it is called (anywhere after this header is included).
   ===========================================================================
*/
// Requires (included before this file in Halloweend2026.cpp):
//   Config.h, ropestuff.h, Halloween_chase.h, Fire2012withPalette.h, Rope_Fire2012.h, Flicker.h,
//   CD77_palettecrossfade.h, CD77_slowchangepalette.h (and the FadetoPaletteAnimation instance declared in the .cpp),
//   LED_Effects.h

// ---------- Time Performance state ----------
uint32_t gTimeCodeBase         = 0;
uint32_t gTimeCode             = 0;
uint32_t gLastTimeCodeDoneAt   = 0;
uint32_t gLastTimeCodeDoneFrom = 0;
uint32_t gActiveWindowStart    = 0xFFFFFFFF;  // start of the BETWEEN window last run
bool     gBetweenEntered       = false;       // true on first pass of a window

// seconds are rounded to the nearest millisecond (avoids 39.3*1000 = 39299.99 truncation)
#define TC(HOURS,MINUTES,SECONDS) \
  ((uint32_t)((HOURS)*3600000.0 + (MINUTES)*60000.0 + (SECONDS)*1000.0 + 0.5))

#define AT(HOURS,MINUTES,SECONDS)    if( atTC(TC(HOURS,MINUTES,SECONDS)) )
#define FROM(HOURS,MINUTES,SECONDS)  if( fromTC(TC(HOURS,MINUTES,SECONDS)) )
#define BETWEEN(H1,M1,S1,H2,M2,S2)   if( betweenTC(TC(H1,M1,S1), TC(H2,M2,S2)) )
#define BETWEEN_ENTERED()            (gBetweenEntered)

static bool atTC(uint32_t tc)
{
  bool maybe = false;
  if (gTimeCode >= tc) {
    if (gLastTimeCodeDoneAt < tc) {
      maybe = true;
      gLastTimeCodeDoneAt = tc;
    }
  }
  return maybe;
}

static bool fromTC(uint32_t tc)
{
  bool maybe = false;
  if (gTimeCode >= tc) {
    if (gLastTimeCodeDoneFrom <= tc) {
      maybe = true;
      gLastTimeCodeDoneFrom = tc;
    }
  }
  return maybe;
}

// true while start <= gTimeCode < end
static bool betweenTC(uint32_t start, uint32_t end)
{
  if (gTimeCode >= start && gTimeCode < end) {
    gBetweenEntered    = (gActiveWindowStart != start);
    gActiveWindowStart = start;
    return true;
  }
  return false;
}

void ResetAllEffects()
{
  CD77_flicker.reset();
  CD77_Halloween_Colors.reset();
  CD77_Chase_Halloween_fixed.reset();
  CD77_Chase_Halloween_random.reset();
  CD77_Chase_Halloween_fixed_color.reset();
  Fire2012WithPalette.reset();
  Fire2012Rope.reset();
  CD77_palette.reset();
  CD77_palettecrossfade.reset();
  CD77_blackout.reset();

  // CD77_slowchangepalette.h: state-only reset (black, no fade in progress).
  // Uses forceInactive() rather than reset() because reset() calls
  // FastLED.show(), and stacking show() calls right before the next
  // effect's own first paint can make that paint fail to latch.
  FadetoPaletteAnimation.forceInactive();

  // LED_Effects.h: SolidFill, Blink, ColorWipeFill and ColorWipeDot remember
  // whether they have already started. Without these calls each one would
  // skip repainting (SolidFill) or resume mid-animation (the others) on the
  // second and later laps. ForceInactiveAll() covers EVERY instance number
  // (1..its _COUNT), so a new instance used in a future show needs no extra
  // line here. State-only: no LED paint, no FastLED.show().
  SolidFill::ForceInactiveAll();
  Blink::ForceInactiveAll();
  ColorWipeFill::ForceInactiveAll();
  ColorWipeDot::ForceInactiveAll();
}

void RestartPerformance()
{
  gLastTimeCodeDoneAt   = 0;
  gLastTimeCodeDoneFrom = 0;
  gActiveWindowStart    = 0xFFFFFFFF;
  gBetweenEntered       = false;
  gTimeCodeBase         = millis();
  ResetAllEffects();
}

// ===========================================================================
//  Shows. Every FROM(start) block of the old file became BETWEEN(start, next start).
// ===========================================================================

//============================================

void Performance12()
{
  BETWEEN(0,0,00.000,  0,0,03.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 30); }
  BETWEEN(0,0,03.000,  0,0,06.000) { CD77_flicker.update(CRGB::Purple,    5, 30); }
  BETWEEN(0,0,06.000,  0,0,09.000) { CD77_flicker.update(CRGB::Green,     5, 30); }

  BETWEEN(0,0,09.000,  0,0,14.000) { if (BETWEEN_ENTERED()) { FastLED.clear(); FastLED.show(); }   // flicker can end with the strip lit
                                     ColorWipeFill::Update(leds, 1, {CRGB::Green,   0,  256, 20, true}); }
  BETWEEN(0,0,14.000,  0,0,14.200) { CD77_blackout.update(); }
  BETWEEN(0,0,14.200,  0,0,19.200) { ColorWipeFill::Update(leds, 1, {CRGB::OrangeRed,   0,  256, 20, true}); }      // thick and heavy
  BETWEEN(0,0,19.200,  0,0,19.400) { CD77_blackout.update(); }
  BETWEEN(0,0,19.400,  0,0,24.400) { ColorWipeFill::Update(leds, 1, {CRGB::Purple,   0,  256, 20, true}); }
  BETWEEN(0,0,24.400,  0,0,24.600) { CD77_blackout.update(); }

  BETWEEN(0,0,24.600,  0,0,37.800) {Fire2012Rope.update(HeatColors_p, 55, 120, 30);}
 
  BETWEEN(0,0,37.800,  0,0,38.000) { CD77_blackout.update(); }

  BETWEEN(0,0,38.000,  0,0,43.000) { ColorWipeFill::Update(leds, 1, {CRGB::Green,   0,  256, 20, true}); }
 
  BETWEEN(0,0,43.000,  0,0,48.000) { ColorWipeFill::Update(leds, 2, {CRGB::OrangeRed,   0,  256, 20, false}); }      // thick and heavy
 
  BETWEEN(0,0,48.000,  0,0,53.000) { ColorWipeFill::Update(leds, 3, {CRGB::Purple,   0,  256, 20, true}); }
  BETWEEN(0,0,53.000,  0,0,53.200) { CD77_blackout.update(); }


 BETWEEN(0,0,53.200,  0,0,58.200) { ColorWipeFill::Update(leds, 1, {CRGB::Green,   0,  85, 55, true});
                                     ColorWipeFill::Update(leds, 2, {CRGB::OrangeRed,   85,  170, 55, true});
                                     ColorWipeFill::Update(leds, 3, {CRGB::Purple,   170,  256, 55, true}); }
  BETWEEN(0,0,58.200,  0,0,58.400) { CD77_blackout.update(); }

  BETWEEN(0,0,58.400,  0,1,06.200) { CD77_palette.update(PAL_NORTH, hc5_p, 30); }
  BETWEEN(0,1,06.200,  0,1,14.200) { CD77_palette.update(PAL_SOUTH, hc5_p, 30); }
  BETWEEN(0,1,14.200,  0,1,14.400) { CD77_blackout.update(); }
  BETWEEN(0,1,14.400,  0,1,18.200) { CD77_Chase_Halloween_fixed_color.update(CRGB::OrangeRed, 700, 8); }
  BETWEEN(0,1,18.200,  0,1,22.200) { CD77_Chase_Halloween_fixed_color.update(CRGB::Purple, 700, 8); }
  BETWEEN(0,1,22.200,  0,1,26.200) { CD77_Chase_Halloween_fixed_color.update(CRGB::Green, 700, 8); }
  BETWEEN(0,1,26.200,  0,1,30.200) { CD77_Chase_Halloween_fixed_color.update(CRGB::OrangeRed, 700, 8); }
  BETWEEN(0,1,30.200,  0,1,35.200) { CD77_Chase_Halloween_fixed_color.update(CRGB::Purple, 700, 8); }
  BETWEEN(0,1,35.200,  0,1,39.200) { CD77_Chase_Halloween_fixed_color.update(CRGB::Green, 700, 8); }
  BETWEEN(0,1,39.200,  0,1,39.500) { CD77_blackout.update(); }
  BETWEEN(0,1,39.500,  0,1,47.200) { CD77_palette.update(PAL_NORTH,   hc6_p, 30); }
  BETWEEN(0,1,47.200,  0,1,55.200) { CD77_palette.update(PAL_SOUTH,   hc6_p, 30); }
  BETWEEN(0,1,55.200,  0,2,03.200) { CD77_palette.update(PAL_INWARD,  hc6_p, 30); }
  BETWEEN(0,2,03.200,  0,2,11.200) { CD77_palette.update(PAL_OUTWARD, IceColors_p, 30); }
  BETWEEN(0,2,11.200,  0,2,11.400) { CD77_blackout.update(); }
  AT(0,2,11.400)                   { RestartPerformance(); }
}




//=======================================================


void Performance7()
{
  BETWEEN(0,0,00.000,  0,0,03.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 30); }
  BETWEEN(0,0,03.000,  0,0,06.000) { CD77_flicker.update(CRGB::Purple,    5, 30); }
  BETWEEN(0,0,06.000,  0,0,09.000) { CD77_flicker.update(CRGB::Green,     5, 30); }
  BETWEEN(0,0,09.000,  0,0,023.000) {Fire2012Rope.update(HeatColors_p, 55, 120, 30);}
  /*
  BETWEEN(0,0,09.000,  0,0,12.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 30); }
  BETWEEN(0,0,12.000,  0,0,15.000) { CD77_flicker.update(CRGB::Purple,    5, 30); }
  BETWEEN(0,0,15.000,  0,0,23.000) { CD77_flicker.update(CRGB::Green,     5, 30); }
  */

  BETWEEN(0,0,23.000,  0,0,31.000) { CD77_palette.update(PAL_NORTH, hc5_p, 30); }
  BETWEEN(0,0,31.000,  0,0,39.000) { CD77_palette.update(PAL_SOUTH, hc5_p, 30); }
  BETWEEN(0,0,39.000,  0,0,39.200) { CD77_blackout.update(); }
  BETWEEN(0,0,39.200,  0,0,43.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::OrangeRed, 700, 8); }
  BETWEEN(0,0,43.000,  0,0,47.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Purple, 700, 8); }
  BETWEEN(0,0,47.000,  0,0,51.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Green, 700, 8); }
  BETWEEN(0,0,51.000,  0,0,55.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::OrangeRed, 700, 8); }
  BETWEEN(0,0,55.000,  0,0,59.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Purple, 700, 8); }
  BETWEEN(0,0,59.000,  0,1,03.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Green, 700, 8); }
  BETWEEN(0,1,03.000,  0,1,03.300) { CD77_blackout.update(); }
  BETWEEN(0,1,03.300,  0,1,11.000) { CD77_palette.update(PAL_NORTH,   hc6_p, 30); }
  BETWEEN(0,1,11.000,  0,1,19.000) { CD77_palette.update(PAL_SOUTH,   hc6_p, 30); }
  BETWEEN(0,1,19.000,  0,1,27.000) { CD77_palette.update(PAL_INWARD,  hc6_p, 30); }
  BETWEEN(0,1,27.000,  0,1,35.000) { CD77_palette.update(PAL_OUTWARD, IceColors_p, 30); }
  BETWEEN(0,1,35.000,  0,1,35.200) { CD77_blackout.update(); }
  AT(0,1,35.200)                   { RestartPerformance(); }
}



//======================

void Performance6()
{
  BETWEEN(0,0,00.000,  0,0,01.000) { SolidFill::Update    (leds, 1, {CRGB::Red,    0,  256, 500, true}); }
  BETWEEN(0,0,01.000,  0,0,02.000) { SolidFill::Update    (leds, 1, {CRGB::Blue,    0,  256, 500, true}); }
  BETWEEN(0,0,02.000,  0,0,03.000) { SolidFill::Update    (leds, 1, {CRGB::Yellow,    0,  256, 500, true}); }
  BETWEEN(0,0,03.000,  0,0,08.000) { Blink::Update        (leds, 1, {CRGB::Blue, 0, 256, 600, true}); }
  BETWEEN(0,0,08.000,  0,0,08.200) { CD77_blackout.update(); }
  BETWEEN(0,0,08.200,  0,0,13.000) { Blink::Update        (leds, 1, {CRGB::Blue, 0, 85, 300, true});
                                     Blink::Update        (leds, 2, {CRGB::Red, 85, 170, 300, true});
                                     Blink::Update        (leds, 3, {CRGB::Green, 170, 256, 300, true});
  }

  BETWEEN(0,0,13.000,  0,0,22.000) { Blink::Update        (leds, 1, {CRGB::Blue, 0, 85, 120, true});
                                     Blink::Update        (leds, 2, {CRGB::Red, 85, 170, 100, true});
                                     Blink::Update        (leds, 3, {CRGB::Green, 170, 256, 80, true});
  }



 /* BETWEEN(0,0,30.000,  0,0,30.500) { CD77_blackout.update(); }   // 20 s dark gap + 0.2 s settle, as before
  BETWEEN(0,0,30.500,  0,0,42.000) { Fire2012WithPalette.update(HeatColors_p, 75, 80); }
  BETWEEN(0,0,42.000,  0,0,52.000) { Fire2012WithPalette.update(IceColors_p,  75, 80); }
   BETWEEN(0,0,11.000,  0,0,11.200) { CD77_blackout.update(); }
 */ BETWEEN(0,0,22.000,  0,0,22.200) { CD77_blackout.update(); }

 BETWEEN(0,0,22.200,  0,0,27.000) {
  ColorWipeFill::Update(leds, 1, {CRGB::Red,  0,  85,  20, true});
  ColorWipeFill::Update(leds, 2, {CRGB::Blue,  85,  170,  20, true});
  ColorWipeFill::Update(leds, 3, {CRGB::Green,  170,  256,  20, true});
 }  
BETWEEN(0,0,27.000,  0,0,27.200) { CD77_blackout.update(); }
  AT(0,0,27.200)                   { RestartPerformance(); }
}





// 12 s: three flicker colors, then restart
void Performance2()
{
  BETWEEN(0,0,00.000,  0,0,04.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 30); }
  BETWEEN(0,0,04.000,  0,0,08.000) { CD77_flicker.update(CRGB::Purple,    5, 30); }
  BETWEEN(0,0,08.000,  0,0,12.000) { CD77_flicker.update(CRGB::Green,     5, 30); }
  AT(0,0,12.000)                   { RestartPerformance(); }
}

// 12 s: color chase
void Performance4c()
{
  BETWEEN(0,0,00.000,  0,0,02.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::OrangeRed, 700, 8); }
  BETWEEN(0,0,02.000,  0,0,04.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Purple, 700, 8); }
  BETWEEN(0,0,04.000,  0,0,06.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Green, 700, 8); }
  BETWEEN(0,0,06.000,  0,0,08.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::OrangeRed, 700, 8); }
  BETWEEN(0,0,08.000,  0,0,10.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Purple, 700, 8); }
  BETWEEN(0,0,10.000,  0,0,12.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Green, 700, 8); }
  AT(0,0,12.000)                   { RestartPerformance(); }
}

// 1:35.2 : flicker, hc5 sweeps, color chase, ice sweeps
void Performance5()
{
  BETWEEN(0,0,00.000,  0,0,03.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 30); }
  BETWEEN(0,0,03.000,  0,0,06.000) { CD77_flicker.update(CRGB::Purple,    5, 30); }
  BETWEEN(0,0,06.000,  0,0,09.000) { CD77_flicker.update(CRGB::Green,     5, 30); }
  BETWEEN(0,0,09.000,  0,0,023.000) {Fire2012Rope.update(HeatColors_p, 55, 120, 30);}
  /*
  BETWEEN(0,0,09.000,  0,0,12.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 30); }
  BETWEEN(0,0,12.000,  0,0,15.000) { CD77_flicker.update(CRGB::Purple,    5, 30); }
  BETWEEN(0,0,15.000,  0,0,23.000) { CD77_flicker.update(CRGB::Green,     5, 30); }
  */

  BETWEEN(0,0,23.000,  0,0,31.000) { CD77_palette.update(PAL_NORTH, hc5_p, 30); }
  BETWEEN(0,0,31.000,  0,0,39.000) { CD77_palette.update(PAL_SOUTH, hc5_p, 30); }
  BETWEEN(0,0,39.000,  0,0,39.200) { CD77_blackout.update(); }
  BETWEEN(0,0,39.200,  0,0,43.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::OrangeRed, 700, 8); }
  BETWEEN(0,0,43.000,  0,0,47.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Purple, 700, 8); }
  BETWEEN(0,0,47.000,  0,0,51.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Green, 700, 8); }
  BETWEEN(0,0,51.000,  0,0,55.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::OrangeRed, 700, 8); }
  BETWEEN(0,0,55.000,  0,0,59.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Purple, 700, 8); }
  BETWEEN(0,0,59.000,  0,1,03.000) { CD77_Chase_Halloween_fixed_color.update(CRGB::Green, 700, 8); }
  BETWEEN(0,1,03.000,  0,1,03.300) { CD77_blackout.update(); }
  BETWEEN(0,1,03.300,  0,1,11.000) { CD77_palette.update(PAL_NORTH,   IceColors_p, 40); }
  BETWEEN(0,1,11.000,  0,1,19.000) { CD77_palette.update(PAL_SOUTH,   IceColors_p, 40); }
  BETWEEN(0,1,19.000,  0,1,27.000) { CD77_palette.update(PAL_INWARD,  IceColors_p, 30); }
  BETWEEN(0,1,27.000,  0,1,35.000) { CD77_palette.update(PAL_OUTWARD, IceColors_p, 30); }
  BETWEEN(0,1,35.000,  0,1,35.200) { CD77_blackout.update(); }
  AT(0,1,35.200)                   { RestartPerformance(); }
}

// 1:44.2 : flicker x2, Halloween chase, fire (heat then ice)
void Performancex2()
{
  BETWEEN(0,0,00.000,  0,0,04.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 30); }
  BETWEEN(0,0,04.000,  0,0,08.000) { CD77_flicker.update(CRGB::Purple,    5, 30); }
  BETWEEN(0,0,08.000,  0,0,12.000) { CD77_flicker.update(CRGB::Green,     5, 30); }
  BETWEEN(0,0,12.000,  0,0,16.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 30); }
  BETWEEN(0,0,16.000,  0,0,20.000) { CD77_flicker.update(CRGB::Purple,    5, 30); }
  BETWEEN(0,0,20.000,  0,0,24.000) { CD77_flicker.update(CRGB::Green,     5, 30); }
  BETWEEN(0,0,24.000,  0,0,39.000) { CD77_Halloween_Colors.update(600); CD77_Chase_Halloween_fixed.update(300, 3); }
  BETWEEN(0,0,39.000,  0,0,54.000) { CD77_Halloween_Colors.update(600); CD77_Chase_Halloween_fixed.update(700, 8); }
  BETWEEN(0,0,54.000,  0,1,14.200) { CD77_blackout.update(); }   // 20 s dark gap + 0.2 s settle, as before
  BETWEEN(0,1,14.200,  0,1,29.000) { Fire2012WithPalette.update(HeatColors_p, 75, 80); }
  BETWEEN(0,1,29.000,  0,1,44.000) { Fire2012WithPalette.update(IceColors_p,  75, 80); }
  BETWEEN(0,1,44.000,  0,1,44.200) { CD77_blackout.update(); }
  AT(0,1,44.200)                   { RestartPerformance(); }
}

// 1:00.2 : flicker x2, Halloween chase, party palette sweep
void Performancex3()
{
  BETWEEN(0,0,00.000,  0,0,04.000) { CD77_flicker.update(CRGB::OrangeRed, 5, 20); }
  BETWEEN(0,0,04.000,  0,0,08.000) { CD77_flicker.update(CRGB::Purple,    5, 20); }
  BETWEEN(0,0,08.000,  0,0,12.000) { CD77_flicker.update(CRGB::Green,     5, 20); }
  BETWEEN(0,0,12.000,  0,0,16.000) { CD77_flicker.update(CRGB::OrangeRed, 1, 30); }
  BETWEEN(0,0,16.000,  0,0,20.000) { CD77_flicker.update(CRGB::Purple,    1, 30); }
  BETWEEN(0,0,20.000,  0,0,24.200) { CD77_flicker.update(CRGB::Green,     1, 30); }
  BETWEEN(0,0,24.200,  0,0,32.000) { CD77_Halloween_Colors.update(600); CD77_Chase_Halloween_fixed.update(300, 3); }
  BETWEEN(0,0,32.000,  0,0,40.000) { CD77_Halloween_Colors.update(600); CD77_Chase_Halloween_fixed.update(700, 8); }
  BETWEEN(0,0,40.000,  0,0,40.100) { CD77_blackout.update(); }
  BETWEEN(0,0,40.100,  0,1,00.000) { CD77_palette.update(PAL_NORTH, PartyColors_p, 30); }
  BETWEEN(0,1,00.000,  0,1,00.200) { CD77_blackout.update(); }
  AT(0,1,00.200)                   { RestartPerformance(); }
}

// 52.2 s : Halloween chase, fire (heat then ice)
void Performance()
{
  BETWEEN(0,0,00.000,  0,0,15.000) { CD77_Halloween_Colors.update(600); CD77_Chase_Halloween_fixed.update(300, 3); }
  BETWEEN(0,0,15.000,  0,0,30.000) { CD77_Halloween_Colors.update(600); CD77_Chase_Halloween_fixed.update(700, 8); }
  BETWEEN(0,0,30.000,  0,0,30.500) { CD77_blackout.update(); }   // 20 s dark gap + 0.2 s settle, as before
  BETWEEN(0,0,30.500,  0,0,42.000) { Fire2012WithPalette.update(HeatColors_p, 75, 80); }
  BETWEEN(0,0,42.000,  0,0,52.000) { Fire2012WithPalette.update(IceColors_p,  75, 80); }
  BETWEEN(0,0,52.000,  0,0,52.200) { CD77_blackout.update(); }
  AT(0,0,52.200)                   { RestartPerformance(); }
}
