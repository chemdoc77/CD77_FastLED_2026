// CD77 FastLED Matrix Fun 2026 by Chemdoc77


/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/

// Full-matrix and per-quadrant rotating Archimedean spirals
// + concentric box-fill spirals for any W x H WS2812b matrix.
//
// Hardware (default)
// ------------------
// MCU    : Lolin D32 (ESP32)
// Data   : GPIO 12
// Matrix : 16x16 WS2812b, serpentine wiring
//          LED 0 = top-left, even cols top->bottom, odd cols bottom->top
//
// HOW TO CONFIGURE FOR YOUR MATRIX
// ---------------------------------
// 1. Set MATRIX_WIDTH and MATRIX_HEIGHT below.
//    Both must be even numbers and >= 8
//    (for best spiral results both should also be multiples of 4).
//
// 2. Set WIRING_ORIGIN to the corner where LED 0 is physically located.
//
// 3. Set WIRING_DIR to the direction the serpentine stripes run.
//
// 4. sp_init() in setup() builds the LED map and initialises all quad
//    objects automatically -- nothing else needs to change.
//
// QUAD LAYOUT (always exactly 4 quads)
// --------------------------------------
//   QUAD_WIDTH  = MATRIX_WIDTH  / 2
//   QUAD_HEIGHT = MATRIX_HEIGHT / 2
//
//   (1,1) Top-L  |  (2,1) Top-R
//   (1,2) Bot-L  |  (2,2) Bot-R
//
//   16x16 matrix -> 4 quads of  8x8
//   32x16 matrix -> 4 quads of 16x8
//    8x8  matrix -> 4 quads of  4x4
//   24x24 matrix -> 4 quads of 12x12
//*************** Important - LOOK *************************************************
// EFFECT FUNCTIONS
// ----------------
//   fullSpiral.Update(armColor, bgColor, speedMs, thickness, CW)
//   fullSpiralP.Update(palette, bgColor, speedMs, thickness, CW, palSpeed)
//   updateMatrix_4quad_Spiral( qCol, qRow, armColor, bgColor, speedMs, thickness, CW)
//   updateMatrix_4quad_SpiralP(qCol, qRow, palette,  bgColor, speedMs, thickness, CW, palSpeed)
//   updateMatrixBoxes( color,   intervalMs, holdMs, reverse)
//   updateMatrix_4quad_Boxes( color,   intervalMs, holdMs, reverse)
//   updateMatrix_4quad_BoxesQ( qCol, qRow, color,   intervalMs, holdMs, reverse)
//   updateMatrixBoxesP(  palette, bgColor, intervalMs, holdMs, reverse, palSpeed)
//   updateMatrix_4quad_BoxesP(palette, bgColor, intervalMs, holdMs, reverse, palSpeed)
//   updateMatrix_4quad_BoxesQP(qCol, qRow, palette, intervalMs, holdMs, reverse, palSpeed)

#include <FastLED.h>

// Matrix dimensions -- change these for your hardware
#define MATRIX_WIDTH    16
#define MATRIX_HEIGHT   16
#define NUM_LEDS        (MATRIX_WIDTH * MATRIX_HEIGHT)

// Wiring Origin Options
//   WIRING_ORIGIN_TOP_LEFT      Top-left corner (default)
//   WIRING_ORIGIN_TOP_RIGHT     Top-right corner
//   WIRING_ORIGIN_BOTTOM_LEFT   Bottom-left corner
//   WIRING_ORIGIN_BOTTOM_RIGHT  Bottom-right corner
//
// Wiring Direction Options
//   WIRING_DIR_COLUMNS              Serpentine -- even cols top->bottom, odd cols bottom->top (default)
//   WIRING_DIR_ROWS                 Serpentine -- even rows left->right, odd rows right->left
//   WIRING_DIR_COLUMNS_PROGRESSIVE  Progressive -- every column runs the same direction
//   WIRING_DIR_ROWS_PROGRESSIVE     Progressive -- every row runs the same direction
//
// Serpentine is the most common wiring for WS2812b panels.
// Progressive (typewriter-style) wiring is less common but fully supported.
//
// Wiring configuration -- set to match your physical wiring
#define WIRING_ORIGIN   WIRING_ORIGIN_TOP_LEFT
#define WIRING_DIR      WIRING_DIR_COLUMNS

// Time Performance globals (must come before Time_performance.h)
uint32_t gTimeCodeBase         = 0;
uint32_t gTimeCode             = 0;
uint32_t gLastTimeCodeDoneAt   = 0;
uint32_t gLastTimeCodeDoneFrom = 0;

// FastLED hardware settings
#define LED_PIN       12
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB
#define BRIGHTNESS    75

CRGB leds[NUM_LEDS];


// Effect headers (order matters)

#include "CD77_Spiral_FX.h"
#include "CD77_Spiral_FX_P.h"
#include "CD77_Matrix_loop_functions.h"
#include "CD77_Matrix_loop_functions_P.h"
#include "Time_performance.h"



void setup() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)
           .setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 8500);
    set_max_power_indicator_LED(13);

    // Build LED map and initialise all 4 quad objects.
    sp_init();

    FastLED.clear(true);
    RestartPerformance();
}

void loop() {
    sp8q_tick(sp8q_intervalMs, sp8q_holdMs);
    gTimeCode = millis() - gTimeCodeBase;
    Performance();
}
