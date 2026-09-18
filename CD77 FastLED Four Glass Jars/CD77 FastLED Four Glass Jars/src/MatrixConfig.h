#pragma once
#include <FastLED.h>

// ---------- Hardware configuration ----------
#define LED_PIN      12
#define BRIGHTNESS  85
#define LED_TYPE     WS2812B
#define COLOR_ORDER  GRB

#define NUM_JARS     4
#define MATRIX_ROWS  8
#define MATRIX_COLS  8
#define NUM_LEDS     (NUM_JARS * MATRIX_ROWS * MATRIX_COLS) // 256

// ---------- LED index map ----------
// jarMatrix[jar][row][col] = physical LED index in the single 256-LED
// daisy-chained strip, taken from 4_jars_matrices_20260809.xlsx. The 4
// jars are wired in series on one data pin: jar 0 = LEDs 0-63, jar 1 =
// 64-127, jar 2 = 128-191, jar 3 = 192-255. Each jar uses the same
// column-wise serpentine wiring as glass_jar_v2, just offset by 64 per
// jar.
const uint8_t jarMatrix[NUM_JARS][MATRIX_ROWS][MATRIX_COLS] = {
  // Jar 0 (LEDs 0-63)
  {
    {0, 15, 16, 31, 32, 47, 48, 63},
    {1, 14, 17, 30, 33, 46, 49, 62},
    {2, 13, 18, 29, 34, 45, 50, 61},
    {3, 12, 19, 28, 35, 44, 51, 60},
    {4, 11, 20, 27, 36, 43, 52, 59},
    {5, 10, 21, 26, 37, 42, 53, 58},
    {6, 9,  22, 25, 38, 41, 54, 57},
    {7, 8,  23, 24, 39, 40, 55, 56}
  },
  // Jar 1 (LEDs 64-127)
  {
    {64, 79, 80, 95, 96, 111, 112, 127},
    {65, 78, 81, 94, 97, 110, 113, 126},
    {66, 77, 82, 93, 98, 109, 114, 125},
    {67, 76, 83, 92, 99, 108, 115, 124},
    {68, 75, 84, 91, 100, 107, 116, 123},
    {69, 74, 85, 90, 101, 106, 117, 122},
    {70, 73, 86, 89, 102, 105, 118, 121},
    {71, 72, 87, 88, 103, 104, 119, 120}
  },
  // Jar 2 (LEDs 128-191)
  {
    {128, 143, 144, 159, 160, 175, 176, 191},
    {129, 142, 145, 158, 161, 174, 177, 190},
    {130, 141, 146, 157, 162, 173, 178, 189},
    {131, 140, 147, 156, 163, 172, 179, 188},
    {132, 139, 148, 155, 164, 171, 180, 187},
    {133, 138, 149, 154, 165, 170, 181, 186},
    {134, 137, 150, 153, 166, 169, 182, 185},
    {135, 136, 151, 152, 167, 168, 183, 184}
  },
  // Jar 3 (LEDs 192-255) -- columns 5-7 inferred, see note above
  {
    {192, 207, 208, 223, 224, 239, 240, 255},
    {193, 206, 209, 222, 225, 238, 241, 254},
    {194, 205, 210, 221, 226, 237, 242, 253},
    {195, 204, 211, 220, 227, 236, 243, 252},
    {196, 203, 212, 219, 228, 235, 244, 251},
    {197, 202, 213, 218, 229, 234, 245, 250},
    {198, 201, 214, 217, 230, 233, 246, 249},
    {199, 200, 215, 216, 231, 232, 247, 248}
  }
};
/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.

*/
