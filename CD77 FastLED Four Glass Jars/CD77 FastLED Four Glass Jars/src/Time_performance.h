#ifndef Time_performance
#define Time_performance

/* Based on the Time Performance sketch by Mark Kriegsman of FastLED:
   https://gist.github.com/kriegsman/a916be18d32ec675fea8
   Extended with BETWEEN()/START()/TO(). See README.md for full details
   on every class and parameter -- this file just lists what's callable.
*/

// ════════════════════════════════════════════════════════════════════
//  Update() FUNCTIONS AVAILABLE IN Performance() / Performance2()
// ════════════════════════════════════════════════════════════════════
//  Mirrored (all 4 jars):
//    rowFill.Update(color, rowInterval, direction, runID)
//    colFill.Update(color, colInterval, direction, runID)
//    rowGradientFill.Update(startColor, endColor, rowInterval, direction, runID)
//    colGradientFill.Update(startColor, endColor, colInterval, direction, runID)
//    squareFill.Update(color, ringInterval, direction, runID)
//    squareFillMulti.Update(ringColors[4], ringInterval, direction, runID)
//    FadetoPaletteAnimation.Update(palette, colorBlendAmount, updateIntervalMs, brightnessFadeAmount, colorStepIntervalMs, targetBrightness)
//    solidColorAllJars.update(color)
//    blinkColorAllJars.update(color, xOnMs, yOffMs)
//    chaseOnAllJars.update(color, xStepMs)
//    paletteMotion.Update(palette, direction, updateIntervalMs, speedBPM, runID)
//    twinkleAllJars.update(color, fadeAmount, spawnChance, updateIntervalMs)
//    theaterChaseAllJars.update(color, spacing, stepIntervalMs)
//    cometBounceAllJars.update(color, tailFade, stepIntervalMs)
//    threeDots.Update()   -- no params; whole-strip only, no per-jar variant, no runID/reset();
//                            does NOT call FastLED.show() internally (only class here that doesn't) --
//                            you must call FastLED.show() yourself right after it, every time
//                            (see ThreeDots.h / README.md -- setBall*() configures color/speed/size separately)
//
//  Per-jar (index [0]-[3]):
//    rowFillJar[i].Update(color, rowInterval, direction, runID)
//    colFillJar[i].Update(color, colInterval, direction, runID)
//    rowGradientFillJar[i].Update(startColor, endColor, rowInterval, direction, runID)
//    colGradientFillJar[i].Update(startColor, endColor, colInterval, direction, runID)
//    squareFillJar[i].Update(color, ringInterval, direction, runID)
//    squareFillMultiJar[i].Update(ringColors[4], ringInterval, direction, runID)
//    fadeJar[i].Update(palette, colorBlendAmount, updateIntervalMs, brightnessFadeAmount, colorStepIntervalMs, targetBrightness)
//    solidColorJar[i].update(color)
//    blinkColorJar[i].update(color, xOnMs, yOffMs)
//    chaseOnJar[i].update(color, xStepMs)
//    twinkleJar[i].update(color, fadeAmount, spawnChance, updateIntervalMs)
//    theaterChaseJar[i].update(color, spacing, stepIntervalMs)
//    cometBounceJar[i].update(color, tailFade, stepIntervalMs)
//
//  Cascade helper (CascadeLapTracker.h): squareCascadeTracker, rowGradCascadeTracker
//    .checkNewLap(runID)  .reset()  .stage()  .advance()
//
//  Variables:
//    color, startColor, endColor   CRGB             solid color(s)
//    ringColors                    CRGB[4]          per-ring colors, [0]=outer ... [3]=center
//    palette                       CRGBPalette16    target/sample palette (e.g. gb6_p, RainbowColors_p)
//    direction                     enum             TOP_DOWN/BOTTOM_UP, LEFT_RIGHT/RIGHT_LEFT,
//                                                    CENTER_TO_OUTSIDE/OUTSIDE_TO_CENTER, or
//                                                    PM_CLOCKWISE..PM_NORTHWEST (paletteMotion)
//    rowInterval/colInterval/ringInterval   unsigned long   ms between each build step
//    updateIntervalMs              ms               redraw/refresh interval
//    colorBlendAmount              1-255            fade blend step size
//    brightnessFadeAmount          1-255            brightness ramp step size
//    targetBrightness              0-255            this call's brightness ceiling (FadetoPaletteAnimation /
//                                                     fadeJar[i] only, default BRIGHTNESS) -- pure red/green/
//                                                     blue/yellow don't look equally bright to the eye at the
//                                                     same numeric value, so different jars/colors often want
//                                                     different targets here to look visually matched
//    colorStepIntervalMs           ms               how often color blends a step
//    speedBPM                      uint8_t          paletteMotion rotation speed, beats/min
//    xOnMs / yOffMs                unsigned long    BlinkColor on/off durations
//    xStepMs                       unsigned long    ChaseOn delay per LED
//    fadeAmount / tailFade         1-255            Twinkle/CometBounce fade step size
//    spawnChance                   0-255            Twinkle: chance per tick a new spark appears
//    spacing                       uint8_t          TheaterChase: LEDs between each lit pixel
//    stepIntervalMs                unsigned long    TheaterChase/CometBounce delay per step
//    runID                         unsigned long    changes once per lap -- forces a restart
//
//  Full explanations, class details, and design notes: README.md
// ════════════════════════════════════════════════════════════════════

// ---------- Globals ----------
uint32_t gTimeCodeBase         = 0;
uint32_t gTimeCode             = 0;
uint32_t gLastTimeCodeDoneAt   = 0;
uint32_t gLastTimeCodeDoneFrom = 0;
uint32_t gPerformanceRunID     = 0; // pass as runID; changes once per lap (see RestartPerformance)

// ---------- Performance selector: function-pointer type ----------
// A PerformanceFunc is just "a function that takes nothing and returns
// nothing" -- exactly the signature of Performance(), Performance2(),
// Performance3(), and Performance4(). This lets PerformanceSelector.Update()
// (bottom of this file) take one of those function NAMES directly as its
// argument, instead of a separate id/enum standing in for it.
typedef void (*PerformanceFunc)();

// ---------- Time macros ----------
#define TC(HOURS,MINUTES,SECONDS) \
  ((uint32_t)(((uint32_t)((HOURS)*(uint32_t)(3600000))) + \
  ((uint32_t)((MINUTES)*(uint32_t)(60000))) + \
  ((uint32_t)((SECONDS)*(uint32_t)(1000)))))

#define AT(HOURS,MINUTES,SECONDS) if( atTC(TC(HOURS,MINUTES,SECONDS)) )       // fires once
#define FROM(HOURS,MINUTES,SECONDS) if( fromTC(TC(HOURS,MINUTES,SECONDS)) )   // fires from tc onward, never stops
#define START(HOURS,MINUTES,SECONDS) TC(HOURS,MINUTES,SECONDS)
#define TO(HOURS,MINUTES,SECONDS)    TC(HOURS,MINUTES,SECONDS)
#define BETWEEN(STARTTC, ENDTC) if( betweenTC((STARTTC), (ENDTC)) )           // fires only within [start,end)

static bool atTC( uint32_t tc)
{
  bool maybe = false;
  if( gTimeCode >= tc) {
    if( gLastTimeCodeDoneAt < tc) {
      maybe = true;
      gLastTimeCodeDoneAt = tc;
    }
  }
  return maybe;
}

static bool fromTC( uint32_t tc)
{
  bool maybe = false;
  if( gTimeCode >= tc) {
    if( gLastTimeCodeDoneFrom <= tc) {
      maybe = true;
      gLastTimeCodeDoneFrom = tc;
    }
  }
  return maybe;
}

static bool betweenTC( uint32_t startTc, uint32_t endTc)
{
  return (gTimeCode >= startTc) && (gTimeCode < endTc);
}

// ---------- RestartPerformance: resets timeline + all effects for a new lap ----------
void RestartPerformance()
{
  gLastTimeCodeDoneAt   = 0;
  gLastTimeCodeDoneFrom = 0;
  gTimeCodeBase         = millis();
  gPerformanceRunID++;

  FadetoPaletteAnimation.reset();
  for (uint8_t jar = 0; jar < NUM_JARS; jar++) fadeJar[jar].reset();

  solidColorAllJars.reset();
  blinkColorAllJars.reset();
  chaseOnAllJars.reset();
  for (uint8_t jar = 0; jar < NUM_JARS; jar++) {
    solidColorJar[jar].reset();
    blinkColorJar[jar].reset();
    chaseOnJar[jar].reset();
  }

  twinkleAllJars.reset();
  theaterChaseAllJars.reset();
  cometBounceAllJars.reset();
  for (uint8_t jar = 0; jar < NUM_JARS; jar++) {
    twinkleJar[jar].reset();
    theaterChaseJar[jar].reset();
    cometBounceJar[jar].reset();
  }

  FastLED.setBrightness(BRIGHTNESS); // safety-net ceiling: set once here, and no effect class
                                      // (FadeToPaletteAnimation included, as of the double-
                                      // brightness-scaling fix) should ever touch it again --
                                      // see CD77_slowchangepalette.h's file header
}




// ════════════════════════════════════════════════════════════════════
//   zzz

void Performance()
{

  //BETWEEN(START(0,0,00.000), TO(0,0,05.000)) { threeDots.Update(); FastLED.show(); }// step x1 -- three bouncing balls (see ThreeDots.h); FastLED.show() is REQUIRED here -- ThreeDots::Update() is the one class in this sketch that does NOT call FastLED.show() internally, so leaving this out means nothing lights up
  BETWEEN(START(0,0,00.000), TO(0,0,05.000)) {// step x2
  colFill.Update(CRGB::Red, 150, LEFT_RIGHT, gPerformanceRunID); // 32 steps x 150ms = 4.8s
 }

  BETWEEN(START(0,0,05.000), TO(0,0,05.200)) {// step x3
    rowGradCascadeTracker.reset();
    for (uint8_t j = 1; j < NUM_JARS; j++) {
      fill_solid(&leds[j * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, CRGB::Black);
    }
    FastLED.show();
  }

  BETWEEN(START(0,0,05.200), TO(0,0,15.000)) {// step x4
    rowGradientFillJar[0].Update(CRGB::Red, CRGB::Blue, 100, TOP_DOWN, gPerformanceRunID);
    if (rowGradCascadeTracker.stage() == 0 && rowGradientFillJar[0].isComplete()) {
      rowGradCascadeTracker.advance();
    }

    if (rowGradCascadeTracker.stage() >= 1) {
      rowGradientFillJar[1].Update(CRGB::Red, CRGB::Blue, 100, TOP_DOWN, gPerformanceRunID);
      if (rowGradCascadeTracker.stage() == 1 && rowGradientFillJar[1].isComplete()) {
        rowGradCascadeTracker.advance();
      }
    }

    if (rowGradCascadeTracker.stage() >= 2) {
      rowGradientFillJar[2].Update(CRGB::Indigo, CRGB::Blue, 100, TOP_DOWN, gPerformanceRunID);
      if (rowGradCascadeTracker.stage() == 2 && rowGradientFillJar[2].isComplete()) {
        rowGradCascadeTracker.advance();
      }
    }

    if (rowGradCascadeTracker.stage() >= 3) {
      rowGradientFillJar[3].Update(CRGB::Indigo, CRGB::Blue, 100, TOP_DOWN, gPerformanceRunID);
    }
  }

  BETWEEN(START(0,0,15.000), TO(0,0,20.000)) {// step x5
    colGradientFill.Update(CRGB::Red, CRGB::Blue, 150, LEFT_RIGHT, gPerformanceRunID); // 32 steps x 150ms = 4.8s
  }

  BETWEEN(START(0,0,20.000), TO(0,0,25.000)) {// step x6
    squareFill.Update(CRGB::Purple, 400, CENTER_TO_OUTSIDE, gPerformanceRunID);
  }

  BETWEEN(START(0,0,25.000), TO(0,0,30.000)) {// step x7
    static CRGB ringColors[4] = { CRGB::Red, CRGB::Red, CRGB::Blue, CRGB::Blue };
    squareFillMulti.Update(ringColors, 400, OUTSIDE_TO_CENTER, gPerformanceRunID);
  }

  BETWEEN(START(0,0,30.000), TO(0,0,35.000)) {// step x8
    rowFillJar[0].Update(CRGB::Blue,   500, TOP_DOWN, gPerformanceRunID);
    rowFillJar[1].Update(CRGB::Red,    500, TOP_DOWN, gPerformanceRunID);
    rowFillJar[2].Update(CRGB::Green,  500, TOP_DOWN, gPerformanceRunID);
    rowFillJar[3].Update(CRGB::Yellow, 500, TOP_DOWN, gPerformanceRunID);
  }

  BETWEEN(START(0,0,35.000), TO(0,0,39.000)) {// step x9
    FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red), 3, 20, 255, 0);
  }

  BETWEEN(START(0,0,39.000), TO(0,0,47.000)) {// step x10
    fadeJar[0].Update(CRGBPalette16(CRGB::Red),    3, 20, 255, 0);
    fadeJar[1].Update(CRGBPalette16(CRGB::Blue),   3, 20, 255, 0);
    fadeJar[2].Update(CRGBPalette16(CRGB::Green),  3, 20, 255, 0);
    fadeJar[3].Update(CRGBPalette16(CRGB::Yellow), 3, 20, 255, 0);
  }

  BETWEEN(START(0,0,47.000), TO(0,0,50.000)) {// step x11
    solidColorAllJars.update(CRGB::Blue);
  }

  BETWEEN(START(0,0,50.000), TO(0,0,53.000)) {// step x12
    solidColorJar[0].update(CRGB::Blue);
    solidColorJar[1].update(CRGB::Red);
    solidColorJar[2].update(CRGB::Green);
    solidColorJar[3].update(CRGB::Yellow);
  }

  BETWEEN(START(0,0,53.000), TO(0,0,58.000)) {// step x13
    blinkColorAllJars.update(CRGB::Red, 500, 250);// DELETE
  }

  BETWEEN(START(0,0,58.000), TO(0,0,63.000)) {// step x14
    blinkColorJar[0].update(CRGB::Blue,   300, 300);
    blinkColorJar[1].update(CRGB::Red,    400, 200);
    blinkColorJar[2].update(CRGB::Green,  250, 250);
    blinkColorJar[3].update(CRGB::Yellow, 600, 300);
  }

  BETWEEN(START(0,0,63.000), TO(0,0,70.000)) {// step x15 zzz
    chaseOnAllJars.update(CRGB::Green, 25); // 256 steps x 25ms = 6.4s
  }

  BETWEEN(START(0,0,70.000), TO(0,0,75.000)) {// step x16  zzz
    chaseOnJar[0].update(CRGB::Blue,   70);
    chaseOnJar[1].update(CRGB::Red,    70);
    chaseOnJar[2].update(CRGB::Green,  70);
    chaseOnJar[3].update(CRGB::Yellow, 70);
  }

  BETWEEN(START(0,0,75.000), TO(0,0,80.000)) {// step x17
    static CRGB jar0Rings[4] = { CRGB::Red,    CRGB::Orange,  CRGB::Yellow,  CRGB::Green };
    static CRGB jar1Rings[4] = { CRGB::Blue,   CRGB::Cyan,    CRGB::Teal,    CRGB::White };
    static CRGB jar2Rings[4] = { CRGB::Purple, CRGB::Magenta, CRGB::Pink,    CRGB::White };
    static CRGB jar3Rings[4] = { CRGB::Black,  CRGB::Grey,    CRGB::Silver,  CRGB::White };

    squareFillMultiJar[0].Update(jar0Rings, 400, CENTER_TO_OUTSIDE, gPerformanceRunID);
    squareFillMultiJar[1].Update(jar1Rings, 400, CENTER_TO_OUTSIDE, gPerformanceRunID);
    squareFillMultiJar[2].Update(jar2Rings, 400, CENTER_TO_OUTSIDE, gPerformanceRunID);
    squareFillMultiJar[3].Update(jar3Rings, 400, CENTER_TO_OUTSIDE, gPerformanceRunID);
  }

  BETWEEN(START(0,0,80.000), TO(0,0,83.000)) {// step x18
    paletteMotion.Update(gb6_p, PM_CLOCKWISE, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,83.000), TO(0,0,86.000)) {// step x19
    paletteMotion.Update(gb6_p, PM_COUNTER_CLOCKWISE, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,86.000), TO(0,0,89.000)) {// step x20
    paletteMotion.Update(IceColors_p, PM_OUTWARD, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,89.000), TO(0,0,92.000)) {// step x21
    paletteMotion.Update(gb11_p, PM_INWARD, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,92.000), TO(0,0,95.000)) {// step x22
    paletteMotion.Update(gb11_p, PM_NORTH, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,95.000), TO(0,0,98.000)) {// step x23
    paletteMotion.Update(gb11_p, PM_NORTHEAST, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,98.000), TO(0,0,101.000)) {// step x24
    paletteMotion.Update(gb11_p, PM_EAST, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,101.000), TO(0,0,104.000)) {// step x25
    paletteMotion.Update(gb11_p, PM_SOUTHEAST, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,104.000), TO(0,0,107.000)) {// step x26
    paletteMotion.Update(IceColors_p, PM_SOUTH, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,107.000), TO(0,0,110.000)) {// step x27
    paletteMotion.Update(IceColors_p, PM_SOUTHWEST, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,110.000), TO(0,0,113.000)) {// step x28
    paletteMotion.Update(IceColors_p, PM_WEST, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,113.000), TO(0,0,116.000)) {// step x29
    paletteMotion.Update(IceColors_p, PM_NORTHWEST, 20, 60, gPerformanceRunID);
  }

  AT(0,0,116.000) {
    RestartPerformance();
  }
}

void Performance2()
{

  BETWEEN(START(0,0,00.000), TO(0,0,10.000)) { threeDots.Update(); FastLED.show(); }// step x1 -- three bouncing balls (see ThreeDots.h); FastLED.show() is REQUIRED here -- ThreeDots::Update() is the one class in this sketch that does NOT call FastLED.show() internally, so leaving this out means nothing lights up
 // BETWEEN(START(0,0,00.000), TO(0,0,05.000)) {// step x2
   // colFill.Update(CRGB::Red, 150, LEFT_RIGHT, gPerformanceRunID); // 32 steps x 150ms = 4.8s
  //}

  BETWEEN(START(0,0,10.000), TO(0,0,10.200)) {// step x3
    rowGradCascadeTracker.reset();
    for (uint8_t j = 1; j < NUM_JARS; j++) {
      fill_solid(&leds[j * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, CRGB::Black);
    }
    FastLED.show();
  }

  BETWEEN(START(0,0,10.200), TO(0,0,15.000)) {// step x4
    rowGradientFillJar[0].Update(CRGB::Red, CRGB::Blue, 100, TOP_DOWN, gPerformanceRunID);
    if (rowGradCascadeTracker.stage() == 0 && rowGradientFillJar[0].isComplete()) {
      rowGradCascadeTracker.advance();
    }

    if (rowGradCascadeTracker.stage() >= 1) {
      rowGradientFillJar[1].Update(CRGB::Red, CRGB::Blue, 100, TOP_DOWN, gPerformanceRunID);
      if (rowGradCascadeTracker.stage() == 1 && rowGradientFillJar[1].isComplete()) {
        rowGradCascadeTracker.advance();
      }
    }

    if (rowGradCascadeTracker.stage() >= 2) {
      rowGradientFillJar[2].Update(CRGB::Indigo, CRGB::Blue, 100, TOP_DOWN, gPerformanceRunID);
      if (rowGradCascadeTracker.stage() == 2 && rowGradientFillJar[2].isComplete()) {
        rowGradCascadeTracker.advance();
      }
    }

    if (rowGradCascadeTracker.stage() >= 3) {
      rowGradientFillJar[3].Update(CRGB::Indigo, CRGB::Blue, 100, TOP_DOWN, gPerformanceRunID);
    }
  }


  AT(0,0,15.000) {
    RestartPerformance();
  }
}

//=================zzzz======================
void Performance3()
{

  //BETWEEN(START(0,0,00.000), TO(0,0,05.000)) { threeDots.Update(); FastLED.show(); }// step x1 -- three bouncing balls (see ThreeDots.h); FastLED.show() is REQUIRED here -- ThreeDots::Update() is the one class in this sketch that does NOT call FastLED.show() internally, so leaving this out means nothing lights up
  BETWEEN(START(0,0,00.000), TO(0,0,05.000)) {// step x2
  colFill.Update(CRGB::Red, 150, LEFT_RIGHT, gPerformanceRunID); // 32 steps x 150ms = 4.8s
 }

  BETWEEN(START(0,0,05.000), TO(0,0,05.200)) {// step x3
    rowGradCascadeTracker.reset();
    for (uint8_t j = 1; j < NUM_JARS; j++) {
      fill_solid(&leds[j * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, CRGB::Black);
    }
    FastLED.show();
  }

  
  BETWEEN(START(0,0,5.200), TO(0,0,10.000)) {// step x5
    colGradientFill.Update(CRGB::Red, CRGB::Blue, 150, LEFT_RIGHT, gPerformanceRunID); // 32 steps x 150ms = 4.8s
  }

  BETWEEN(START(0,0,10.000), TO(0,0,15.000)) {// step x8
    rowFillJar[0].Update(CRGB::Blue,   500, TOP_DOWN, gPerformanceRunID);
    rowFillJar[1].Update(CRGB::Red,    500, TOP_DOWN, gPerformanceRunID);
    rowFillJar[2].Update(CRGB::Green,  500, TOP_DOWN, gPerformanceRunID);
    rowFillJar[3].Update(CRGB::Yellow, 500, TOP_DOWN, gPerformanceRunID);
  }

  BETWEEN(START(0,0,15.000), TO(0,0,20.000)) {// step x9
    FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red), 3, 20, 255, 0);
  }

  BETWEEN(START(0,0,20.000), TO(0,0,25.000)) {// step x10
    fadeJar[0].Update(CRGBPalette16(CRGB::Red),    3, 20, 255, 0);
    fadeJar[1].Update(CRGBPalette16(CRGB::Blue),   3, 20, 255, 0);
    fadeJar[2].Update(CRGBPalette16(CRGB::Green),  3, 20, 255, 0);
    fadeJar[3].Update(CRGBPalette16(CRGB::Yellow), 3, 20, 255, 0);
  }

  BETWEEN(START(0,0,25.000), TO(0,0,30.000)) {// step x13
    blinkColorAllJars.update(CRGB::Red, 500, 250);// DELETE
  }

  BETWEEN(START(0,0,30.000), TO(0,0,35.000)) {// step x14
    blinkColorJar[0].update(CRGB::Blue,   300, 300);
    blinkColorJar[1].update(CRGB::Red,    400, 200);
    blinkColorJar[2].update(CRGB::Green,  250, 250);
    blinkColorJar[3].update(CRGB::Yellow, 600, 300);
  }
 

 AT(0,0,35.000) {
    RestartPerformance();
  }
   
}
//=============================== Performance4=====================

void Performance4()
{
 
  BETWEEN(START(0,0,00.000), TO(0,0,03.000)) {// step x2
    paletteMotion.Update(gb6_p, PM_COUNTER_CLOCKWISE, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,03.000), TO(0,0,06.000)) {// step x3
    paletteMotion.Update(IceColors_p, PM_OUTWARD, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,06.000), TO(0,0,09.000)) {// step x4
    paletteMotion.Update(gb11_p, PM_INWARD, 20, 60, gPerformanceRunID);
  }

  
  BETWEEN(START(0,0,09.000), TO(0,0,12.000)) {// step x7
    paletteMotion.Update(gb11_p, PM_EAST, 20, 60, gPerformanceRunID);
  }
 
  BETWEEN(START(0,0,12.000), TO(0,0,15.000)) {// step x9
    paletteMotion.Update(IceColors_p, PM_SOUTH, 20, 60, gPerformanceRunID);
  }
 

  BETWEEN(START(0,0,15.000), TO(0,0,18.000)) {// step x11
    paletteMotion.Update(IceColors_p, PM_WEST, 20, 60, gPerformanceRunID);
  }


  AT(0,0,18.000) {
    RestartPerformance();
  }
  
}
//=========================== Performance5========================

void Performance5()
{

  //BETWEEN(START(0,0,00.000), TO(0,0,05.000)) { threeDots.Update(); FastLED.show(); }// step x1 -- three bouncing balls (see ThreeDots.h); FastLED.show() is REQUIRED here -- ThreeDots::Update() is the one class in this sketch that does NOT call FastLED.show() internally, so leaving this out means nothing lights up
  BETWEEN(START(0,0,00.000), TO(0,0,05.000)) {// step x2
  colFill.Update(CRGB::Red, 150, LEFT_RIGHT, gPerformanceRunID); // 32 steps x 150ms = 4.8s
 }


BETWEEN(START(0,0,05.000), TO(0,0,08.000)) {// step x2
    paletteMotion.Update(gb6_p, PM_COUNTER_CLOCKWISE, 20, 60, gPerformanceRunID);
}

  BETWEEN(START(0,0,08.000), TO(0,0,8.200)) {// step x3
    rowGradCascadeTracker.reset();
    for (uint8_t j = 1; j < NUM_JARS; j++) {
      fill_solid(&leds[j * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, CRGB::Black);
    }
    FastLED.show();
  }

  
  BETWEEN(START(0,0,8.200), TO(0,0,13.000)) {// step x5
    colGradientFill.Update(CRGB::Red, CRGB::Green, 150, LEFT_RIGHT, gPerformanceRunID); // 32 steps x 150ms = 4.8s
  }

  BETWEEN(START(0,0,13.000), TO(0,0,18.000)) {// step x8
    rowFillJar[0].Update(CRGB::Blue,   500, TOP_DOWN, gPerformanceRunID);
    rowFillJar[1].Update(CRGB::Red,    500, TOP_DOWN, gPerformanceRunID);
    rowFillJar[2].Update(CRGB::Green,  500, TOP_DOWN, gPerformanceRunID);
    rowFillJar[3].Update(CRGB::Yellow, 500, TOP_DOWN, gPerformanceRunID);
  }

BETWEEN(START(0,0,18.000), TO(0,0,23.000)) {// step x3
    paletteMotion.Update(IceColors_p, PM_OUTWARD, 20, 60, gPerformanceRunID);
  }
  BETWEEN(START(0,0,23.000), TO(0,0,28.000)) {// step x4
    paletteMotion.Update(gb11_p, PM_INWARD, 20, 60, gPerformanceRunID);
  }


  BETWEEN(START(0,0,28.000), TO(0,0,33.000)) {// step x9
    // Matches the boosted brightness (150) used by the per-jar segment
    // right after this one, so the transition from this mirrored red
    // into that segment doesn't have a visible brightness jump.
    FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red), 3, 20, 255, 0, 150);
  }

  BETWEEN(START(0,0,33.000), TO(0,0,38.000)) {// step x10
    // All 4 jars get a brightness target here, tuned per color -- pure
    // red, green, blue, and yellow at the exact same numeric brightness
    // value don't look equally bright to the eye (green in particular
    // reads as much more luminous than red or blue at equal values --
    // and WS2812 dies themselves often have unequal per-channel
    // efficiency too), so matching them visually means giving them
    // different numeric targets. Tune all four numbers (0-255) to taste
    // for your specific LEDs -- there's no "correct" ratio to calculate,
    // since it depends on your eyes and your exact hardware.
    fadeJar[0].Update(CRGBPalette16(CRGB::Red),    3, 20, 255, 0, 150); // Red   -- boosted brighter
    fadeJar[1].Update(CRGBPalette16(CRGB::Blue),   3, 20, 255, 0, 150); // Blue  -- boosted brighter
    fadeJar[2].Update(CRGBPalette16(CRGB::Green),  3, 20, 255, 0, 150); // Green -- boosted brighter
    fadeJar[3].Update(CRGBPalette16(CRGB::Yellow), 3, 20, 255, 0, 150); // Yellow-- boosted brighter
  }



  BETWEEN(START(0,0,38.000), TO(0,0,43.000)) {// step x7
    paletteMotion.Update(gb11_p, PM_EAST, 20, 60, gPerformanceRunID);
  }
 
  BETWEEN(START(0,0,43.000), TO(0,0,48.000)) {// step x9
    paletteMotion.Update(IceColors_p, PM_SOUTH, 20, 60, gPerformanceRunID);
  }
 

  BETWEEN(START(0,0,48.000), TO(0,0,53.000)) {// step x13
    blinkColorAllJars.update(CRGB::Red, 500, 250);// DELETE
  }

  BETWEEN(START(0,0,53.000), TO(0,0,58.000)) {// step x14
    blinkColorJar[0].update(CRGB::Blue,   300, 300);
    blinkColorJar[1].update(CRGB::Red,    400, 200);
    blinkColorJar[2].update(CRGB::Green,  250, 250);
    blinkColorJar[3].update(CRGB::Yellow, 600, 300);
  }
 

BETWEEN(START(0,0,58.000), TO(0,1,03.000)) {// step x11
    paletteMotion.Update(IceColors_p, PM_WEST, 20, 60, gPerformanceRunID);
  }

BETWEEN(START(0,1,03.000), TO(0,1,18.000)) { threeDots.Update(); FastLED.show(); }

BETWEEN(START(0,1,18.000), TO(0,1,23.000)) {// step x3
    paletteMotion.Update(IceColors_p, PM_OUTWARD, 20, 60, gPerformanceRunID);
}
 AT(0,1,23.000) {
    RestartPerformance();
  }
   
}
//========================
// ════════════════════════════════════════════════════════════════════
//  PerformanceSelector
//
//  Non-blocking state machine, same single-call-per-effect Update()
//  pattern as everything else in this sketch -- but instead of naming
//  just one show, you can call Update() once per loop() pass FOR EACH
//  show you want in rotation, back to back, e.g.:
//
//    PerformanceSelector performanceUpdate;   // construct once, globally
//    // every loop() pass:
//    performanceUpdate.Update(Performance2);
//    performanceUpdate.Update(Performance3);
//
//  With two (or more) Update() calls like that, the FIRST show named
//  runs every frame until it completes its own lap -- i.e. until its
//  own internal AT(...) { RestartPerformance(); } block fires, at the
//  end of whatever timeline that function itself defines -- and only
//  then does control hand off to the NEXT show named that same frame.
//  That one then runs to the end of ITS lap, then hands off to the
//  next, wrapping back around to the first once every show named has
//  had its turn. Nothing about Performance()/Performance2()/etc.
//  themselves needs to change for this to work -- they already each
//  call RestartPerformance() once per lap, which is exactly the signal
//  this class watches for to know when to switch.
//
//  With just ONE Update() call in loop() (e.g. only
//  `performanceUpdate.Update(Performance4);`), there's nothing to hand
//  off to, so that one show simply loops forever, same as before.
//
//  Order matters, and matters consistently: keep the same functions in
//  the same order across every loop() pass. The rotation is discovered
//  the first time each function name is seen, in the order it's named,
//  so calling Update() with a different set of functions on different
//  passes isn't a supported way to build a dynamic playlist.
// ════════════════════════════════════════════════════════════════════
class PerformanceSelector {
public:
  void Update(PerformanceFunc show) {
    if (show == nullptr) return;

    // Find (or, the first time it's named, register) this show's slot
    // in the rotation.
    int8_t slot = _indexOf(show);
    if (slot < 0) {
      if (_count >= MAX_SHOWS) return; // rotation full; ignore extras
      slot = _count;
      _shows[_count++] = show;
    }

    if (slot != _activeSlot) {
      return; // not this show's turn yet -- do nothing this frame
    }

    if (_firstEverCall) {
      _firstEverCall = false;
      RestartPerformance(); // clean start: t=0, every effect reset
    }

    // Recompute gTimeCode fresh, relative to the CURRENT gTimeCodeBase,
    // right before running this show. This matters because a hand-off
    // may have just reset gTimeCodeBase (via RestartPerformance()) a
    // moment ago in THIS SAME loop() pass -- e.g. the show before this
    // one in the chain just finished its lap and called
    // RestartPerformance() internally. Without this line, the
    // newly-active show's very first frame would run using a stale,
    // already-elapsed gTimeCode left over from the show before it (the
    // single "gTimeCode = millis() - gTimeCodeBase;" at the top of
    // loop() only runs once per frame, BEFORE any hand-off happens) --
    // which makes its own AT()/BETWEEN() blocks fire wildly out of
    // order, often completing (and handing off again) in the very same
    // frame it started, before ever drawing anything. That's why, with
    // two shows chained in loop() (e.g. Performance6 then Performance7),
    // the second one could appear to never run at all.
    gTimeCode = millis() - gTimeCodeBase;

    uint32_t runIDBefore = gPerformanceRunID;
    show(); // run the currently-active show for this frame

    if (gPerformanceRunID != runIDBefore) {
      // The active show just completed its own lap -- its own internal
      // AT(...) { RestartPerformance(); } fired, which already reset
      // the timeline. Hand off to the next show in rotation; it'll
      // start clean from t=0 the next time its own Update() call comes
      // around (next frame, if it's the very next line in loop()).
      _activeSlot = (_activeSlot + 1) % _count;
    }
  }

private:
  static const uint8_t MAX_SHOWS = 8;

  int8_t _indexOf(PerformanceFunc show) const {
    for (uint8_t i = 0; i < _count; i++) {
      if (_shows[i] == show) return (int8_t)i;
    }
    return -1;
  }

  PerformanceFunc _shows[MAX_SHOWS];
  uint8_t         _count         = 0;
  uint8_t         _activeSlot    = 0;
  bool            _firstEverCall = true;
};

// Single instance, constructed once here so it's available anywhere
// this header is included -- same pattern as every other effect
// instance in this sketch (which live in glass_jar_4jars.cpp instead,
// since they're declared before Time_performance.h is #included there;
// this one lives here since nothing else needs to reference it first).
PerformanceSelector performanceUpdate;

#endif
