// FastLED Pussywillows by Chemdoc77 


/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/


// CD77_pussywillows_both.ino
// Pussywillow row and column fill effects
// Hardware: Lolin D32  |  15 x 10 WS2811 matrix
//
// Effect logic lives in pussywillow_both_functions.h
// The effects initialise themselves automatically on their first loop() call.
#include <Arduino.h>
#include <FastLED.h>

// ═════════════════════════════════════════════════════════════════════════════
//  ▶  ADJUSTABLE VARIABLES  –  edit these for your project
// ═════════════════════════════════════════════════════════════════════════════

// Matrix dimensions
#define MATRIX_WIDTH    10        // number of columns
#define MATRIX_HEIGHT   15        // number of rows
#define NUM_LEDS        (MATRIX_WIDTH * MATRIX_HEIGHT)   // 150

// Hardware
#define LED_PIN         12        // Lolin D32 data pin to first LED
#define LED_TYPE        WS2811
#define COLOR_ORDER     RGB       // change to GRB if colours appear wrong
#define BRIGHTNESS      150       // 0 (off) – 255 (maximum)

//Time Performance code

uint32_t gTimeCodeBase = 0;
uint32_t gTimeCode = 0;
uint32_t gLastTimeCodeDoneAt = 0;
uint32_t gLastTimeCodeDoneFrom = 0;
//=================

// Pussywillow arrays
byte coordsX[NUM_LEDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 57, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 113, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 170, 198, 198, 198, 198, 198, 198, 198, 198, 198, 198, 198, 198, 198, 198, 198, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
byte coordsY[NUM_LEDS] = { 255, 219, 182, 146, 109, 73, 36, 0, 18, 55, 91, 128, 164, 200, 237, 255, 219, 182, 146, 109, 73, 36, 0, 18, 55, 91, 128, 164, 200, 237, 255, 219, 182, 146, 109, 73, 36, 0, 18, 55, 91, 128, 164, 200, 237, 255, 219, 182, 146, 109, 73, 36, 0, 18, 55, 91, 128, 164, 200, 237, 255, 219, 182, 146, 109, 73, 36, 0, 18, 55, 91, 128, 164, 200, 237, 255, 219, 182, 146, 109, 73, 36, 0, 18, 55, 91, 128, 164, 200, 237, 255, 219, 182, 146, 109, 73, 36, 0, 18, 55, 91, 128, 164, 200, 237, 255, 219, 182, 146, 109, 73, 36, 0, 18, 55, 91, 128, 164, 200, 237, 255, 219, 182, 146, 109, 73, 36, 0, 18, 55, 91, 128, 164, 200, 237, 255, 219, 182, 146, 109, 73, 36, 0, 18, 55, 91, 128, 164, 200, 237 };
byte angles[NUM_LEDS] = { 218, 225, 236, 251, 12, 25, 34, 40, 37, 30, 19, 4, 243, 230, 221, 214, 221, 232, 250, 15, 29, 38, 44, 41, 34, 23, 5, 240, 226, 217, 209, 215, 227, 248, 19, 35, 43, 48, 46, 40, 28, 7, 236, 220, 212, 203, 208, 219, 245, 26, 43, 50, 53, 52, 47, 36, 10, 229, 212, 205, 197, 200, 207, 236, 40, 52, 56, 58, 58, 55, 48, 19, 215, 203, 199, 191, 191, 191, 191, 64, 64, 64, 64, 64, 64, 64, 64, 191, 191, 191, 185, 182, 176, 146, 88, 75, 71, 69, 70, 73, 79, 109, 167, 180, 184, 179, 174, 164, 137, 101, 85, 78, 74, 76, 81, 91, 118, 154, 170, 177, 174, 167, 156, 134, 109, 93, 84, 79, 81, 88, 99, 121, 146, 162, 171, 169, 162, 150, 133, 113, 98, 89, 84, 86, 93, 105, 122, 142, 157, 166 };
byte radii[NUM_LEDS] = { 232, 190, 158, 142, 148, 173, 210, 255, 232, 190, 158, 142, 148, 173, 210, 216, 170, 133, 114, 121, 150, 192, 240, 216, 170, 133, 114, 121, 150, 192, 203, 153, 110, 86, 95, 130, 177, 229, 203, 153, 110, 86, 95, 130, 177, 192, 139, 91, 58, 71, 114, 166, 220, 192, 139, 91, 58, 71, 114, 166, 186, 130, 76, 32, 51, 103, 158, 214, 186, 130, 76, 32, 51, 103, 158, 184, 127, 71, 14, 42, 99, 156, 212, 184, 127, 71, 14, 42, 99, 156, 186, 130, 76, 32, 51, 103, 158, 214, 186, 130, 76, 32, 51, 103, 158, 192, 139, 91, 58, 71, 114, 166, 220, 192, 139, 91, 58, 71, 114, 166, 203, 153, 110, 86, 95, 130, 177, 229, 203, 153, 110, 86, 95, 130, 177, 216, 170, 133, 114, 121, 150, 192, 240, 216, 170, 133, 114, 121, 150, 192 };



uint8_t offset = 0;
uint8_t speed = 30;

//========================
CRGBPalette16 currentPalette = PartyColors_p;
#include "led_matrix.h"
#include "pussywillow_functions.h"
#include "CD77_slowchangepalette.h"
#include "matrix_functions.h"
#include "pw_matrix_fx.h"
// Create the palette animation object (palettes are handled internally)
FadeToPaletteAnimation FadetoPaletteAnimation(leds, NUM_LEDS, BRIGHTNESS);
#include "pw_jladder.h"        // MUST be after FadetoPaletteAnimation and before Time_performance.h
#include "Time_performance.h"

// ═════════════════════════════════════════════════════════════════════════════
//  LED array
// ═════════════════════════════════════════════════════════════════════════════
CRGB leds[NUM_LEDS];

// ─────────────────────────────────────────────────────────────────────────────
void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  
FastLED.setMaxPowerInVoltsAndMilliamps(5,8500);
 set_max_power_indicator_LED(13);
 fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  // Build sp_ledMap from pw_ledMap and compute spiral tightness
  pw_sp_init();

//Time Performance code
   RestartPerformance();

}

void loop() {

  //matrix functions stuff:
 offset = beat8(speed);
 //Time Performance code
  gTimeCode = millis() - gTimeCodeBase;  
  Performance();

}
