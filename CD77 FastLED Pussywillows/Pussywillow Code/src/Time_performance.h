#pragma once

/* This code is based on the Time Performance sketch by Mark Kriegsman of FastLED at:
   https://gist.github.com/kriegsman/a916be18d32ec675fea8
*/

#define START(HOURS,MINUTES,SECONDS) \
  ((uint32_t)(((uint32_t)((HOURS)*(uint32_t)(3600000))) + \
  ((uint32_t)((MINUTES)*(uint32_t)(60000))) + \
  ((uint32_t)((SECONDS)*(uint32_t)(1000)))))


#define AT(HOURS,MINUTES,SECONDS) if( atSTART(START(HOURS,MINUTES,SECONDS)) )
#define FROM(HOURS,MINUTES,SECONDS) if( fromSTART(START(HOURS,MINUTES,SECONDS)) )
#define BETWEEN(startTime, endTime) \
  if( gTimeCode >= (startTime) && gTimeCode < (endTime) )
#define TO(HOURS,MINUTES,SECONDS) START(HOURS,MINUTES,SECONDS)

static bool atSTART( uint32_t t)
{
  bool maybe = false;
  if( gTimeCode >= t) {
    if( gLastTimeCodeDoneAt < t) {
      maybe = true;
      gLastTimeCodeDoneAt = t;
    }
  }
  return maybe;
}

static bool fromSTART( uint32_t t)
{
  bool maybe = false;
  if( gTimeCode >= t) {
    if( gLastTimeCodeDoneFrom <= t) {
      maybe = true;
      gLastTimeCodeDoneFrom = t;
    }
  }
  return maybe;
} 

void RestartPerformance()
{
  gLastTimeCodeDoneAt = 0;
  gLastTimeCodeDoneFrom = 0;
  gTimeCodeBase = millis();
  resetPussywillowEffects();   // force all pussywillow state machines to re-init from scratch
  FadetoPaletteAnimation.reset();    // snap palette back to black so fade-in starts clean
  sp_ring       = -1;          // reset full-matrix box-fill
  spa_front_ring = -1;         // reset front area box-fill
  spa_back_ring  = -1;         // reset back area box-fill
  resetJladder();    // resets speed back to startMs (saved automatically from updateJladder call)
  resetJladderP();   // resets speed back to startMs (saved automatically from updateJladderP call)
}

//note: add  random16_add_entropy( random(0,65535)); and FastLED.delay(1000 / FRAMES_PER_SECOND); for Fire2012 entries
//
// ── TIME MACROS ──────────────────────────────────────────────────────────────
//   AT(h,m,s)              { }  fires exactly ONCE at that timestamp
//   FROM(h,m,s)            { }  fires every loop() FROM that timestamp onward (never stops)
//   BETWEEN(START(h1,m1,s1), TO(h2,m2,s2))  { }  fires every loop() only within that time window
//                                     — preferred over FROM() to prevent effect interference
//
//   Use BETWEEN() for all continuous effects so each effect stops automatically
//   when its window ends.  No resetPussywillowEffects() needed at transitions.
//   Use FROM() only when an effect should run to the end of the performance.
//   Use AT() for one-shot events: clears, resets, transitions.
// ─────────────────────────────────────────────────────────────────────────────



  //updatePussywillowRow(CRGB::Red,    200, 2000, true);   // colour, ms per row, ms hold, direction: false=bottom→top   true=top→bottom  [calls FastLED.show() internally]
  //updatePussywillowRowP(RainbowColors_p, 200, 2000, false, 10); // palette, ms per row, ms hold, direction, paletteSpeed(1-20)  [calls FastLED.show() internally]
  //updatePussywillowCol(CRGB::Blue,   200, 2000, false);  // colour, ms per col, ms hold, direction: false=left→right   true=right→left  [calls FastLED.show() internally]
  //updatePussywillowColP(RainbowColors_p, 200, 2000, false, 25); // palette, ms per col, ms hold, direction, paletteSpeed(1-20)  [calls FastLED.show() internally]
  //updatePussywillowDotFill(CRGB::Green, 50, 2000, false);// colour, ms per LED, ms hold, direction: false=clockwise     true=counter-clockwise  [calls FastLED.show() internally]
  //updatePussywillowDotFillP(RainbowColors_p, 50, 2000, false, 2); // palette, ms per LED, ms hold, direction, paletteSpeed(1-10)  [calls FastLED.show() internally]
  //updatePussywillowRowBounce(CRGB::Red, 80, 500, false, true);      // colour, ms per row, ms hold, reverse: false=bottom first, pingPong: true=bounce false=loop  [calls FastLED.show() internally]
  //updatePussywillowRowBouncePalette(RainbowColors_p, 80, 500, false, true); // palette, ms per row, ms hold, reverse: false=bottom first, pingPong: true=bounce false=loop  [calls FastLED.show() internally]
  //updatePussywillowSinelon(CRGB::Red, 13, 20, 220, false);          // colour, bpm, fadeAmount, saturation, reverse: false=start bottom  [calls FastLED.show() internally]
  //updatePussywillowBreathe(CRGB::White, 10, 10, 255);               // colour, bpm, minBrightness, maxBrightness  [calls FastLED.show() internally]
  //updatePussywillowBreathePalette(RainbowColors_p, 10, 10, 255);    // palette, bpm, minBrightness, maxBrightness  [calls FastLED.show() internally]
  //updatePussywillowMeteor(5, 400, 1200, 40);                        // tailLength, minSpawnGap ms, maxSpawnGap ms, frameInterval ms  [calls FastLED.show() internally]
  //updatePussywillowDualRow(CRGB::Red, CRGB::Blue, 80, 500);        // colorUp, colorDown, ms per row, holdMs  [calls FastLED.show() internally]
  //updatePussywillowDualRowP(RainbowColors_p, RainbowColors_p, 80, 500, 5); // palette1, palette2, ms per row, holdMs, paletteSpeed(1-20)  [calls FastLED.show() internally]
  //updatePussywillowDualCol(CRGB::Red, CRGB::Blue, 80, 500);        // colorLeft, colorRight, ms per col, holdMs  [calls FastLED.show() internally]
  //updatePussywillowDualColP(RainbowColors_p, RainbowColors_p, 80, 500, 5); // palette1, palette2, ms per col, holdMs, paletteSpeed(1-20)  [calls FastLED.show() internally]
  //
  // ── JLADDER — exponentially accelerating row fill (pw_jladder.h) ── all call FastLED.show() internally ──
  //   Rows fill at a uniform speed per cycle; each successive fill cycle is accelPct% faster than the last.
  //   accelPct: % reduction applied to the interval at the start of each new fill cycle (1–90).
  //   Larger = faster acceleration across loops.
  //   10 = very gentle   20 = gentle   30 = moderate   45 = aggressive   70 = near-instant
  //updateJladder(CRGB::Red, 400, 30, 1500, false);                         // colour, startMs, accelPct(1-90), holdMs, reverse: false=bottom→top  true=top→bottom
  //updateJladderP(RainbowColors_p, 400, 30, 1500, false, 10);              // palette, startMs, accelPct(1-90), holdMs, reverse, paletteSpeed(1-20)
  //   Reset: call resetJladder() and resetJladderP() from RestartPerformance() — no arguments needed,
  //          startMs is saved automatically on every updateJladder/updateJladderP call.
  //
  // ── MATRIX FILL FUNCTIONS (matrix_functions.h) ── all call FastLED.show() internally ──
  //   Non-blocking state machines — call every loop() inside a BETWEEN() block.
  //   Each function computes beat8(speed) internally; no globals need to be set.
  //   All 13 functions share the same signature: (palette, speed, intervalMs)
  //     palette    – CRGBPalette16 to colour the LEDs
  //     speed      – animation speed for beat8() (1–255; higher = faster movement)
  //     intervalMs – ms between redraws (default 20 = ~50 FPS; higher = slower/choppier)
  //   Only one should be active at a time — they share a single frame-rate timer.
  //updatePhysicalOrderPalette(RainbowColors_p, 40, 20);     // colours follow physical wire order
  //updateNorthPalette(RainbowColors_p, 40, 20);             // colours flow downward
  //updateSouthPalette(RainbowColors_p, 40, 20);             // colours flow upward
  //updateEastPalette(RainbowColors_p, 40, 20);              // colours flow leftward
  //updateWestPalette(RainbowColors_p, 40, 20);              // colours flow rightward
  //updateNorthEastPalette(RainbowColors_p, 40, 20);         // diagonal: down-left
  //updateNorthWestPalette(RainbowColors_p, 40, 20);         // diagonal: down-right
  //updateSouthEastPalette(RainbowColors_p, 40, 20);         // diagonal: up-left
  //updateSouthWestPalette(RainbowColors_p, 40, 20);         // diagonal: up-right
  //updateInwardPalette(RainbowColors_p, 40, 20);            // colours flow inward toward centre
  //updateOutwardPalette(RainbowColors_p, 40, 20);           // colours flow outward from centre
  //updateClockwisePalette(RainbowColors_p, 40, 20);         // colours rotate clockwise
  //updateCounterClockwisePalette(RainbowColors_p, 40, 20);  // colours rotate counter-clockwise
  //
  // ── PALETTE FADE ANIMATION (CD77_slowchangepalette.h) ── FastLED.show() called internally ──
  //   Class: FadeToPaletteAnimation   Instance: FadetoPaletteAnimation
  //   Single-line call — sets target palette, animates the crossfade, and calls FastLED.show():
  //   FadetoPaletteAnimation.Update(palette);             // e.g. FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red));
  //   FadetoPaletteAnimation.Update();                    // animate toward already-set target, no palette argument needed
  //   FadetoPaletteAnimation.setTargetPalette(palette);   // set target without animating
  //   FadetoPaletteAnimation.setMaxChanges(1-48);         // blend speed (default 48 = fast)
  //   FadetoPaletteAnimation.setUpdateInterval(ms);       // refresh rate ms (default 100 = 10 FPS)
  //   FadetoPaletteAnimation.setMotionSpeed(speed);       // colour index animation speed (default 2)
  //   FadetoPaletteAnimation.reset();                     // snap to black — called by RestartPerformance()
  //
  // ── SPIRAL & BOX-FILL (pw_matrix_fx.h) ── all call FastLED.show() internally ──
  //fullSpiral.Update(CRGB::Red, CRGB::Black, 1500, 0.60f, true);              // armColor, bgColor, speedMs, thickness(0.05-0.90), CW
  //fullSpiralP.Update(RainbowColors_p, CRGB::Black, 1500, 0.60f, true, 3);   // palette, bgColor, speedMs, thickness, CW, palSpeed(0=static 3=gentle 10=fast)
  //updateMatrixBoxes(CRGB::Red, 40, 500, false);                              // colour, ms per LED, holdMs, reverse: false=outside→in true=inside→out
  //updateMatrixBoxesP(RainbowColors_p, CRGB::Black, 40, 500, false, 3);      // palette, bgColor, ms per LED, holdMs, reverse, palSpeed
  //   Reset sentinel for full-matrix box functions: sp_ring = -1;
  //
  // ── FRONT / BACK AREA SPIRAL & BOX-FILL (pw_matrix_fx.h) ── all call FastLED.show() internally ──
  //   area variable is FRONT or BACK
  //updateAreaSpiral(FRONT, CRGB::Red, CRGB::Black, 1500, 0.60f, true);       // area, armColor, bgColor, speedMs, thickness(0.05-0.90), CW
  //updateAreaSpiralP(BACK, RainbowColors_p, CRGB::Black, 1500, 0.60f, true, 3); // area, palette, bgColor, speedMs, thickness, CW, palSpeed
  //updateAreaBoxes(FRONT, CRGB::Red, 40, 500, false);                        // area, colour, ms per LED, holdMs, reverse: false=outside→in true=inside→out
  //updateAreaBoxesP(BACK, RainbowColors_p, CRGB::Black, 40, 500, false, 3); // area, palette, bgColor, ms per LED, holdMs, reverse, palSpeed
  //   Reset sentinels for area box functions: spa_front_ring = -1;  spa_back_ring = -1;





void Performance()

{
  BETWEEN(START(0,0,0.000),  TO(0,0,10.000))  { updatePussywillowMeteor(6, 400, 800, 40);   } 

  AT(0,0,10.100)                             { FastLED.clear(); FastLED.show(); }

  BETWEEN(START(0,0,10.300), TO(0,0,20.000))  { FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red)); }

  BETWEEN(START(0,0,20.000), TO(0,0,30.000))  { FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Navy)); }

  BETWEEN(START(0,0,30.000), TO(0,0,40.000))  { FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red)); }
 
   AT(0,0,40.100)                             { FastLED.clear(); FastLED.show(); }

  BETWEEN(START(0,0,40.300),  TO(0,0,52.000)) {updateJladderP(Rainbow_gp, 225, 30, 15, false, 30);}

 BETWEEN(START(0,0,52.000), TO(0,1,00.000))  { updatePussywillowCol(CRGB::Blue,   150, 150, false);  }

  AT(0,1,00.100)                            { FastLED.clear(); FastLED.show(); }


  BETWEEN(START(0,1,00.300),  TO(0,1,10.000)) {updatePussywillowDualRow(CRGB::Red, CRGB::Blue, 60, 10);}//  //{updatePussywillowRowP(RainbowColors_p, 200, 200, false, 10);}  ://{ updatePussywillowDualRowP(RainbowColors_p, RainbowColors_p, 80, 10, 15); }

  AT(0,1,10.000)                            { FastLED.clear(); FastLED.show(); }

BETWEEN(START(0,1,10.200), TO(0,1,20.000))  { updateNorthPalette(Rainbow_gp, 40, 20); }


BETWEEN(START(0,1,20.000), TO(0,1,34.000))  { updatePussywillowDotFillP(RainbowColors_p, 50, 20, false, 5); }

  AT(0,1,34.000)                            { FastLED.clear(); FastLED.show(); }

BETWEEN(START(0,1,34.200), TO(0,1,44.000))   {updateOutwardPalette(RainbowColors_p, 40, 20);  }


BETWEEN(START(0,1,44.000),  TO(0,1,54.000))   {   updateAreaBoxes(BACK, CRGB::Red, 40, 500, false);  updateAreaBoxes(FRONT, CRGB::Blue, 40, 500, false);    }


  AT(0,1,54.100)                            { FastLED.clear(true); FastLED.show(); }
  AT(0,1,54.300)                            { RestartPerformance(); }
}




/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/








