#ifndef Time_performance
#define Time_performance

/* Based on the Time Performance sketch by Mark Kriegsman of FastLED:
   https://gist.github.com/kriegsman/a916be18d32ec675fea8
*/

#define TC(HOURS,MINUTES,SECONDS) \
  ((uint32_t)(((uint32_t)((HOURS)*(uint32_t)(3600000))) + \
  ((uint32_t)((MINUTES)*(uint32_t)(60000))) + \
  ((uint32_t)((SECONDS)*(uint32_t)(1000)))))

#define AT(HOURS,MINUTES,SECONDS)   if( atTC(TC(HOURS,MINUTES,SECONDS)) )
#define FROM(HOURS,MINUTES,SECONDS) if( fromTC(TC(HOURS,MINUTES,SECONDS)) )

static bool atTC(uint32_t tc) {
  bool maybe = false;
  if (gTimeCode >= tc && gLastTimeCodeDoneAt < tc) { maybe = true; gLastTimeCodeDoneAt = tc; }
  return maybe;
}

static bool fromTC(uint32_t tc) {
  bool maybe = false;
  if (gTimeCode >= tc && gLastTimeCodeDoneFrom <= tc) { maybe = true; gLastTimeCodeDoneFrom = tc; }
  return maybe;
}

void RestartPerformance() {
  gLastTimeCodeDoneAt = 0;
  gLastTimeCodeDoneFrom = 0;
  gTimeCodeBase = millis();
}

// ════════════════════════════════════════════════════════════════════
//  QUICK REFERENCE
// ════════════════════════════════════════════════════════════════════
//
//  QUAD LAYOUT (always exactly 4 quads, each QUAD_WIDTH x QUAD_HEIGHT):
//    QUAD_WIDTH  = MATRIX_WIDTH  / 2
//    QUAD_HEIGHT = MATRIX_HEIGHT / 2
//
//    (1,1) Top-L  |  (2,1) Top-R
//    (1,2) Bot-L  |  (2,2) Bot-R
//
//  Examples:
//    16x16 -> quads of  8x8     32x16 -> quads of 16x8
//     8x8  -> quads of  4x4     24x24 -> quads of 12x12
//
//  -- ROTATING SPIRALS (CD77_Spiral_FX.h) --
//    fullSpiral.Update(armColor, bgColor, speedMs, thickness, CW);
//    updateMatrix_4quad_Spiral(qCol, qRow, armColor, bgColor, speedMs, thickness, CW);
//    FastLED.show();
//
//  -- PALETTE ROTATING SPIRALS (CD77_Spiral_FX_P.h) --
//    fullSpiralP.Update(palette, bgColor, speedMs, thickness, CW, palSpeed);
//    updateMatrix_4quad_SpiralP(qCol, qRow, palette, bgColor, speedMs, thickness, CW, palSpeed);
//    FastLED.show();
//    palSpeed: 0=static  3=gentle  10=fast  20=psychedelic
//
//  -- BOX-FILL SPIRALS (Matrix_loop_functions.h) --
//    updateMatrixBoxes(color, intervalMs, holdMs, reverse);
//    updateMatrix_4quad_Boxes(color, intervalMs, holdMs, reverse);
//    updateMatrix_4quad_BoxesQ(qCol, qRow, color, intervalMs, holdMs, reverse);
//    FastLED.show();
//
//  -- PALETTE BOX-FILL SPIRALS (Matrix_loop_functions_P.h) --
//    updateMatrixBoxesP(palette, bgColor, intervalMs, holdMs, reverse, palSpeed);
//    updateMatrix_4quad_BoxesP(palette, bgColor, intervalMs, holdMs, reverse, palSpeed);
//    updateMatrix_4quad_BoxesQP(qCol, qRow, palette, intervalMs, holdMs, reverse, palSpeed);
//    FastLED.show();
//
//  -- RESET SENTINELS (add to AT() restart line as needed) --
//    sp_ring  = -1;   updateMatrixBoxes / updateMatrixBoxesP
//    sp8_ring = -1;   updateMatrix_4quad_Boxes / updateMatrix_4quad_BoxesP
//    sp8q_reset();    updateMatrix_4quad_BoxesQ / updateMatrix_4quad_BoxesQP
// ════════════════════════════════════════════════════════════════════


void Performance()
{
// Spiral animation part
FROM(0,0,00.000)   { fullSpiralP.Update(RainbowColors_p, CRGB::Black, 1500, 0.60f, true, 3);
                     FastLED.show(); }

FROM(0,0,07.500)   { fullSpiralP.Update(RainbowColors_p, CRGB::Black, 1500, 0.60f, false, 3);
                     FastLED.show(); }

FROM(0,0,15.000)   { FastLED.clear(true); FastLED.show(); sp_ring = -1; sp8q_reset(); }

FROM(0,0,15.100)   { updateMatrix_4quad_SpiralP(1, 1, PartyColors_p, CRGB::Black, 1500, 0.60f, true, 3);
                     FastLED.show(); }

FROM(0,0,20.000)   { updateMatrix_4quad_SpiralP(1, 1, PartyColors_p,  CRGB::Black, 1500, 0.60f, true,  3);
                     updateMatrix_4quad_SpiralP(2, 2, ForestColors_p,  CRGB::Black, 1500, 0.60f, false, 3);
                     FastLED.show(); }

FROM(0,0,25.000)   { updateMatrix_4quad_SpiralP(1, 1, PartyColors_p,  CRGB::Black, 1500, 0.60f, true,  3);
                     updateMatrix_4quad_SpiralP(2, 1, OceanColors_p,  CRGB::Black, 1500, 0.60f, false, 3);
                     updateMatrix_4quad_SpiralP(1, 2, ForestColors_p, CRGB::Black, 1500, 0.60f, false, 3);
                     updateMatrix_4quad_SpiralP(2, 2, LavaColors_p,   CRGB::Black, 1500, 0.60f, true,  3);
                     FastLED.show(); }
//=========================================================================================================

// Boxes animation part                     

FROM(0,0,30.000)   { FastLED.clear(); FastLED.show(); sp_ring = -1; sp8q_reset(); }

FROM(0,0,30.200)   { updateMatrixBoxesP(RainbowColors_p, CRGB::Black, 40, 5000, false, 3);
                     FastLED.show(); }


FROM(0,0,43.000)   { FastLED.clear(); FastLED.show(); sp_ring = -1; sp8q_reset(); }

FROM(0,0,43.200)   { updateMatrix_4quad_BoxesQ(2, 1, CRGB::Blue, 80, 5000, false);
                     FastLED.show(); }

FROM(0,0,50.000)   { FastLED.clear(); FastLED.show(); sp_ring = -1; sp8q_reset(); }

FROM(0,0,50.200)   { updateMatrix_4quad_BoxesQ( 2, 1, CRGB::Blue,        80, 5000, false);
                     updateMatrix_4quad_BoxesQP(1, 2, RainbowColors_p,   80, 5000, true);
                     FastLED.show(); }

FROM(0,0,57.000)   { FastLED.clear(); FastLED.show(); sp_ring = -1; sp8q_reset(); }

FROM(0,0,57.200)   { updateMatrix_4quad_BoxesQ( 1, 2, CRGB::Blue,        80, 2000, false);
                     updateMatrix_4quad_BoxesQP(1, 1, RainbowColors_p,   80, 2000, false);
                     updateMatrix_4quad_BoxesQ( 2, 1, CRGB::Red,         80, 2000, true);
                     updateMatrix_4quad_BoxesQP(2, 2, RainbowColors_p,   80, 2000, true);
                     FastLED.show(); }

FROM(0,1,04.000)   { FastLED.clear(); FastLED.show(); sp_ring = -1; sp8q_reset(); }

FROM(0,1,04.200)   {  updateMatrix_4quad_SpiralP(1, 1, PartyColors_p,  CRGB::Black, 1500, 0.60f, true,  3);
                     updateMatrix_4quad_BoxesQ( 1, 2, CRGB::Blue,        80, 4000, false);
                     updateMatrix_4quad_SpiralP(2, 2, PartyColors_p,  CRGB::Black, 1500, 0.60f, false,  3);
                     updateMatrix_4quad_BoxesQ( 2, 1, CRGB::Red,         80, 4000, true);
                     FastLED.show(); }

FROM(0,1,12.000)   { FastLED.clear(); FastLED.show(); sp_ring = -1; sp8q_reset(); }

AT(0,1,12.000)     { RestartPerformance(); }
}

#endif
