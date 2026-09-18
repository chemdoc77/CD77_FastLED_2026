
// FastLED Four Glass Jars by Chemdoc77

/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/


#include <Arduino.h>
#include <FastLED.h>
#include "MatrixConfig.h"
CRGB leds[NUM_LEDS];

byte coordsX[NUM_LEDS] = { 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 16, 16, 16, 16, 16, 16, 16, 16, 25, 25, 25, 25, 25, 25, 25, 25, 33, 33, 33, 33, 33, 33, 33, 33, 41, 41, 41, 41, 41, 41, 41, 41, 49, 49, 49, 49, 49, 49, 49, 49, 58, 58, 58, 58, 58, 58, 58, 58, 66, 66, 66, 66, 66, 66, 66, 66, 74, 74, 74, 74, 74, 74, 74, 74, 82, 82, 82, 82, 82, 82, 82, 82, 90, 90, 90, 90, 90, 90, 90, 90, 99, 99, 99, 99, 99, 99, 99, 99, 107, 107, 107, 107, 107, 107, 107, 107, 115, 115, 115, 115, 115, 115, 115, 115, 123, 123, 123, 123, 123, 123, 123, 123, 132, 132, 132, 132, 132, 132, 132, 132, 140, 140, 140, 140, 140, 140, 140, 140, 148, 148, 148, 148, 148, 148, 148, 148, 156, 156, 156, 156, 156, 156, 156, 156, 165, 165, 165, 165, 165, 165, 165, 165, 173, 173, 173, 173, 173, 173, 173, 173, 181, 181, 181, 181, 181, 181, 181, 181, 189, 189, 189, 189, 189, 189, 189, 189, 197, 197, 197, 197, 197, 197, 197, 197, 206, 206, 206, 206, 206, 206, 206, 206, 214, 214, 214, 214, 214, 214, 214, 214, 222, 222, 222, 222, 222, 222, 222, 222, 230, 230, 230, 230, 230, 230, 230, 230, 239, 239, 239, 239, 239, 239, 239, 239, 247, 247, 247, 247, 247, 247, 247, 247, 255, 255, 255, 255, 255, 255, 255, 255 };
byte coordsY[NUM_LEDS] = { 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0, 0, 36, 73, 109, 146, 182, 219, 255, 255, 219, 182, 146, 109, 73, 36, 0 };
byte angles[NUM_LEDS] = { 10, 8, 5, 3, 0, 252, 250, 247, 247, 250, 252, 0, 3, 5, 8, 11, 11, 9, 6, 3, 0, 252, 249, 246, 246, 249, 252, 0, 3, 6, 9, 12, 13, 10, 7, 3, 0, 252, 248, 245, 244, 248, 251, 0, 4, 7, 11, 14, 15, 12, 8, 4, 0, 251, 247, 243, 242, 246, 251, 0, 4, 9, 13, 17, 19, 15, 10, 5, 0, 250, 245, 240, 239, 244, 249, 0, 6, 11, 16, 21, 24, 19, 13, 7, 0, 248, 242, 236, 233, 240, 247, 0, 8, 15, 22, 27, 32, 26, 19, 10, 0, 245, 236, 229, 223, 231, 242, 0, 13, 24, 32, 38, 45, 40, 32, 19, 0, 236, 223, 215, 204, 210, 223, 0, 32, 45, 51, 54, 64, 64, 64, 64, 0, 191, 191, 191, 178, 172, 159, 128, 96, 83, 77, 74, 83, 88, 96, 109, 128, 146, 159, 167, 159, 151, 141, 128, 114, 104, 96, 90, 96, 101, 109, 118, 128, 137, 146, 154, 149, 143, 136, 128, 119, 112, 106, 100, 104, 109, 114, 121, 128, 134, 141, 146, 144, 139, 133, 128, 122, 116, 111, 106, 109, 113, 118, 122, 128, 133, 137, 142, 141, 136, 132, 128, 123, 119, 114, 111, 112, 116, 119, 123, 128, 132, 136, 139, 138, 135, 131, 128, 124, 120, 117, 113, 114, 118, 121, 124, 128, 131, 134, 137, 137, 134, 131, 128, 124, 121, 118, 115, 116, 119, 122, 125, 128, 130, 133, 136, 136, 133, 130, 128, 125, 122, 119, 117 };
byte radii[NUM_LEDS] = { 255, 252, 249, 248, 247, 248, 249, 252, 237, 234, 232, 232, 232, 234, 237, 240, 225, 221, 219, 217, 216, 217, 219, 221, 206, 203, 202, 201, 202, 203, 206, 210, 196, 191, 188, 186, 186, 186, 188, 191, 176, 173, 171, 170, 171, 173, 176, 181, 167, 161, 158, 155, 155, 155, 158, 161, 147, 143, 140, 139, 140, 143, 147, 152, 138, 132, 128, 125, 124, 125, 128, 132, 118, 113, 109, 108, 109, 113, 118, 125, 111, 104, 98, 94, 93, 94, 98, 104, 90, 83, 79, 77, 79, 83, 90, 99, 87, 77, 69, 64, 62, 64, 69, 77, 66, 56, 49, 46, 49, 56, 66, 77, 69, 56, 44, 35, 31, 35, 44, 56, 49, 35, 22, 15, 22, 35, 49, 64, 62, 46, 31, 15, 0, 15, 31, 46, 49, 35, 22, 15, 22, 35, 49, 64, 69, 56, 44, 35, 31, 35, 44, 56, 66, 56, 49, 46, 49, 56, 66, 77, 87, 77, 69, 64, 62, 64, 69, 77, 90, 83, 79, 77, 79, 83, 90, 99, 111, 104, 98, 94, 93, 94, 98, 104, 118, 113, 109, 108, 109, 113, 118, 125, 138, 132, 128, 125, 124, 125, 128, 132, 147, 143, 140, 139, 140, 143, 147, 152, 167, 161, 158, 155, 155, 155, 158, 161, 176, 173, 171, 170, 171, 173, 176, 181, 196, 191, 188, 186, 186, 186, 188, 191, 206, 203, 202, 201, 202, 203, 206, 210, 225, 221, 219, 217, 216, 217, 219, 221, 237, 234, 232, 232, 232, 234, 237, 240 };


// Note: `offset`/`speed` globals and the `offset = beat8(speed)` line
// that array_functions.h's original 12 functions relied on have been
// removed -- CD77_ArrayPaletteMotion (array_functions.h) now manages
// its own animation offset internally, non-blocking, matching the
// rest of this sketch's effect classes.

#include "RowColFill.h"
#include "GradientFill.h"
#include "SquareFill.h"
#include "PerJarFill.h"
#include "CD77_slowchangepalette.h"
#include "CD77_slowchangepalette_PerJar.h"
#include "basic_functions.h"
#include "array_functions.h"
#include "CascadeLapTracker.h"
#include "classic_effects.h"
#include "ThreeDots.h"



// ---------- Effect instances ----------
// Every effect instance used by Performance()/Performance2() (declared
// below via Time_performance.h) is now declared inside its own effect
// header, right after that header's class definition(s) -- e.g.
// rowFill/colFill live in RowColFill.h, fadeJar[] lives in
// CD77_slowchangepalette_PerJar.h, twinkleJar[] lives in
// classic_effects.h, and so on. Each header pulls in MatrixConfig.h and
// an `extern CRGB leds[NUM_LEDS];` declaration for the `leds` array
// defined above, so instance construction there works identically to
// how it worked here. See each header's own comments for details on
// its specific instance(s).
//
// NOTE: because these instances are now defined (not just declared) at
// global scope inside headers included with #pragma once, this only
// stays safe as long as each header is included by exactly one .cpp
// (or .ino) in the whole build, as it is here -- including one of
// these headers from a second .cpp/.ino would cause duplicate-symbol
// link errors.

#include "Time_performance.h"

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 8000);
   set_max_power_indicator_LED(13);
  FastLED.clear();
  FastLED.show();

  // No RestartPerformance() call needed here -- performanceUpdate.Update()
  // detects its own first call and does that automatically, same as
  // every other effect class's first Update() in this sketch.
}

void loop() {
  // NOTE: gTimeCode is now (re)computed fresh inside
  // PerformanceSelector::Update() itself, right before it runs whichever
  // show is currently active -- see Time_performance.h. This line here
  // just keeps gTimeCode initialized/sane before the very first
  // Update() call of all (when nothing has registered/reset anything
  // yet); it's harmless to leave in, but the one inside Update() is
  // what actually matters once shows start handing off to each other.
  gTimeCode = millis() - gTimeCodeBase;
   performanceUpdate.Update(Performance5); 
 // performanceUpdate.Update(Performance3); // runs to the end of its own lap, then...
 // performanceUpdate.Update(Performance4); // ...hands off to this one, then back to Performance2, etc.
}
