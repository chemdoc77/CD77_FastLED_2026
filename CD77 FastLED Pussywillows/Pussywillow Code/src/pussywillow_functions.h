// pussywillow_both_functions.h
// Row fill and column fill effects for a WS2811 LED matrix.
// Each is a fully independent, non-blocking state machine.
// Wiring map sourced from USE_pussywillow_spreadsheet.xlsx
//
// ── HOW TO USE ────────────────────────────────────────────────────────────────
//  1. Define MATRIX_WIDTH, MATRIX_HEIGHT, NUM_LEDS, LED_PIN, LED_TYPE,
//     COLOR_ORDER and BRIGHTNESS in the .ino BEFORE the #include.
//  2. In setup() call only the FastLED hardware lines – no pussywillow calls.
//  3. In loop() call whichever effect(s) you want:
//       updatePussywillowRow(colour, intervalMs, holdMs, reverse)
//       updatePussywillowCol(colour, intervalMs, holdMs, reverse)
//     reverse: false = clockwise   true = counter-clockwise
//  The effects initialise themselves automatically on their first call.
// ─────────────────────────────────────────────────────────────────────────────

#pragma once
#include <FastLED.h>

// ─────────────────────────────────────────────────────────────────────────────
//  LED array reference  –  leds[] is defined in the .ino.
//  NUM_LEDS, MATRIX_WIDTH, MATRIX_HEIGHT must be #defined in the .ino first.
// ─────────────────────────────────────────────────────────────────────────────
extern CRGB leds[];

// ─────────────────────────────────────────────────────────────────────────────
//  LED index map  [row][col]
//  Row  0 = TOP of matrix    Row 14 = BOTTOM of matrix
//  Col  0 = LEFT of matrix   Col  9 = RIGHT of matrix
//  Sourced directly from USE_pussywillow_spreadsheet.xlsx
// ─────────────────────────────────────────────────────────────────────────────
static const uint8_t pw_ledMap[15][10] = {
  {  7,  22,  37,  52,  67,  82,  97, 112, 127, 142 },  // row  0  (top)
  {  8,  23,  38,  53,  68,  83,  98, 113, 128, 143 },  // row  1
  {  6,  21,  36,  51,  66,  81,  96, 111, 126, 141 },  // row  2
  {  9,  24,  39,  54,  69,  84,  99, 114, 129, 144 },  // row  3
  {  5,  20,  35,  50,  65,  80,  95, 110, 125, 140 },  // row  4
  { 10,  25,  40,  55,  70,  85, 100, 115, 130, 145 },  // row  5
  {  4,  19,  34,  49,  64,  79,  94, 109, 124, 139 },  // row  6
  { 11,  26,  41,  56,  71,  86, 101, 116, 131, 146 },  // row  7
  {  3,  18,  33,  48,  63,  78,  93, 108, 123, 138 },  // row  8
  { 12,  27,  42,  57,  72,  87, 102, 117, 132, 147 },  // row  9
  {  2,  17,  32,  47,  62,  77,  92, 107, 122, 137 },  // row 10
  { 13,  28,  43,  58,  73,  88, 103, 118, 133, 148 },  // row 11
  {  1,  16,  31,  46,  61,  76,  91, 106, 121, 136 },  // row 12
  { 14,  29,  44,  59,  74,  89, 104, 119, 134, 149 },  // row 13
  {  0,  15,  30,  45,  60,  75,  90, 105, 120, 135 }   // row 14 (bottom)
};

// ─────────────────────────────────────────────────────────────────────────────
//  Shared matrix dimensions
// ─────────────────────────────────────────────────────────────────────────────
static uint8_t pw_rows = MATRIX_HEIGHT;
static uint8_t pw_cols = MATRIX_WIDTH;

// ═════════════════════════════════════════════════════════════════════════════
//  ROW EFFECT
//  PW_CLOCKWISE        = bottom to top   (row 14 → row 0)
//  PW_COUNTERCLOCKWISE = top to bottom   (row 0  → row 14)
// ═════════════════════════════════════════════════════════════════════════════

enum PW_RowState { PW_ROW_RUNNING, PW_ROW_HOLD };

static PW_RowState pw_rowState      = PW_ROW_RUNNING;
static int8_t      pw_currentRow    = -1;   // -1 = not yet initialised
static uint32_t    pw_rowLastUpdate = 0;
static uint32_t    pw_rowHoldStart  = 0;
static bool        pw_rowReverse    = false;

static void _pw_initRow(bool reverse) {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_rowReverse    = reverse;
  pw_currentRow    = reverse ? 0 : pw_rows - 1;  // CW starts bottom, CCW starts top
  pw_rowLastUpdate = millis();
  pw_rowState      = PW_ROW_RUNNING;
}

// Call every loop() iteration to run the row effect
static void updatePussywillowRow(CRGB color, uint32_t intervalMs, uint32_t holdMs, bool reverse = false) {
  if (pw_currentRow == -1) _pw_initRow(reverse);   // auto-init on first call

  uint32_t now = millis();

  if (pw_rowState == PW_ROW_RUNNING) {
    if (now - pw_rowLastUpdate < intervalMs) return;
    pw_rowLastUpdate = now;

    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[pw_currentRow][c]] = color;
    FastLED.show();

    bool done = pw_rowReverse ? (pw_currentRow == pw_rows - 1)  // CCW: reached bottom
                              : (pw_currentRow == 0);            // CW:  reached top
    if (done) {
      pw_rowHoldStart = now;
      pw_rowState     = PW_ROW_HOLD;
    } else {
      pw_currentRow += pw_rowReverse ? 1 : -1;
    }

  } else {  // PW_ROW_HOLD
    if (now - pw_rowHoldStart < holdMs) return;

    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    FastLED.show();
    pw_currentRow    = pw_rowReverse ? 0 : pw_rows - 1;
    pw_rowLastUpdate = millis();
    pw_rowState      = PW_ROW_RUNNING;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  COLUMN EFFECT
//  PW_CLOCKWISE        = left to right   (col 0 → col 9)
//  PW_COUNTERCLOCKWISE = right to left   (col 9 → col 0)
// ═════════════════════════════════════════════════════════════════════════════

enum PW_ColState { PW_COL_RUNNING, PW_COL_HOLD };

static PW_ColState pw_colState      = PW_COL_RUNNING;
static int8_t      pw_currentCol    = -1;   // -1 = not yet initialised
static uint32_t    pw_colLastUpdate = 0;
static uint32_t    pw_colHoldStart  = 0;
static bool        pw_colReverse    = false;

static void _pw_initCol(bool reverse) {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_colReverse    = reverse;
  pw_currentCol    = reverse ? pw_cols - 1 : 0;  // CW starts left, CCW starts right
  pw_colLastUpdate = millis();
  pw_colState      = PW_COL_RUNNING;
}

// Call every loop() iteration to run the column effect
static void updatePussywillowCol(CRGB color, uint32_t intervalMs, uint32_t holdMs, bool reverse = false) {
  if (pw_currentCol == -1) _pw_initCol(reverse);   // auto-init on first call

  uint32_t now = millis();

  if (pw_colState == PW_COL_RUNNING) {
    if (now - pw_colLastUpdate < intervalMs) return;
    pw_colLastUpdate = now;

    for (uint8_t r = 0; r < pw_rows; r++)
      leds[pw_ledMap[r][pw_currentCol]] = color;
    FastLED.show();

    bool done = pw_colReverse ? (pw_currentCol == 0)            // CCW: reached left
                              : (pw_currentCol == pw_cols - 1); // CW:  reached right
    if (done) {
      pw_colHoldStart = now;
      pw_colState     = PW_COL_HOLD;
    } else {
      pw_currentCol += pw_colReverse ? -1 : 1;
    }

  } else {  // PW_COL_HOLD
    if (now - pw_colHoldStart < holdMs) return;

    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    FastLED.show();
    pw_currentCol    = pw_colReverse ? pw_cols - 1 : 0;
    pw_colLastUpdate = millis();
    pw_colState      = PW_COL_RUNNING;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  ROW EFFECT  –  palette colour
//
//  Identical behaviour to updatePussywillowRow() but each row is coloured
//  from a CRGBPalette16.  The palette offset advances by paletteSpeed each
//  row step so every row gets a different colour as the fill progresses.
//  When the matrix is full it holds for holdMs, clears, and restarts with
//  the palette offset continuing from where it left off for seamless cycling.
//
//  Call signature:
//    updatePussywillowRowP(palette, intervalMs, holdMs, reverse, paletteSpeed)
//      palette       – CRGBPalette16 to colour each row
//      intervalMs    – milliseconds between each row step
//      holdMs        – milliseconds to hold when fully filled before clearing
//      reverse       – false = bottom → top   true = top → bottom
//      paletteSpeed  – how much the palette index advances each row step (1–20)
// ═════════════════════════════════════════════════════════════════════════════

enum PW_RowPState { PW_ROWP_RUNNING, PW_ROWP_HOLD };

static PW_RowPState pw_rowPState      = PW_ROWP_RUNNING;
static int8_t       pw_currentRowP    = -1;   // -1 = not yet initialised
static uint32_t     pw_rowPLastUpdate = 0;
static uint32_t     pw_rowPHoldStart  = 0;
static bool         pw_rowPReverse    = false;
static uint8_t      pw_rowPOffset     = 0;    // cycling palette offset

static void _pw_initRowP(bool reverse) {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_rowPReverse    = reverse;
  pw_currentRowP    = reverse ? 0 : pw_rows - 1;
  pw_rowPLastUpdate = millis();
  pw_rowPState      = PW_ROWP_RUNNING;
}

// Call every loop() iteration to run the palette row effect
static void updatePussywillowRowP(CRGBPalette16 palette, uint32_t intervalMs,
                                   uint32_t holdMs, bool reverse = false,
                                   uint8_t paletteSpeed = 10) {
  if (pw_currentRowP == -1) _pw_initRowP(reverse);

  uint32_t now = millis();

  if (pw_rowPState == PW_ROWP_RUNNING) {
    if (now - pw_rowPLastUpdate < intervalMs) return;
    pw_rowPLastUpdate = now;

    CRGB color = ColorFromPalette(palette, pw_rowPOffset, 255, LINEARBLEND);
    pw_rowPOffset += paletteSpeed;

    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[pw_currentRowP][c]] = color;
    FastLED.show();

    bool done = pw_rowPReverse ? (pw_currentRowP == pw_rows - 1)
                               : (pw_currentRowP == 0);
    if (done) {
      pw_rowPHoldStart = now;
      pw_rowPState     = PW_ROWP_HOLD;
    } else {
      pw_currentRowP += pw_rowPReverse ? 1 : -1;
    }

  } else {  // PW_ROWP_HOLD
    if (now - pw_rowPHoldStart < holdMs) return;

    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    FastLED.show();
    pw_currentRowP    = pw_rowPReverse ? 0 : pw_rows - 1;
    pw_rowPLastUpdate = millis();
    pw_rowPState      = PW_ROWP_RUNNING;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  COLUMN EFFECT  –  palette colour
//
//  Identical behaviour to updatePussywillowCol() but each column is coloured
//  from a CRGBPalette16.  The palette offset advances by paletteSpeed each
//  column step so every column gets a different colour as the fill progresses.
//  When the matrix is full it holds for holdMs, clears, and restarts with
//  the palette offset continuing from where it left off for seamless cycling.
//
//  Call signature:
//    updatePussywillowColP(palette, intervalMs, holdMs, reverse, paletteSpeed)
//      palette       – CRGBPalette16 to colour each column
//      intervalMs    – milliseconds between each column step
//      holdMs        – milliseconds to hold when fully filled before clearing
//      reverse       – false = left → right   true = right → left
//      paletteSpeed  – how much the palette index advances each column step (1–20)
// ═════════════════════════════════════════════════════════════════════════════

enum PW_ColPState { PW_COLP_RUNNING, PW_COLP_HOLD };

static PW_ColPState pw_colPState      = PW_COLP_RUNNING;
static int8_t       pw_currentColP    = -1;   // -1 = not yet initialised
static uint32_t     pw_colPLastUpdate = 0;
static uint32_t     pw_colPHoldStart  = 0;
static bool         pw_colPReverse    = false;
static uint8_t      pw_colPOffset     = 0;    // cycling palette offset

static void _pw_initColP(bool reverse) {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_colPReverse    = reverse;
  pw_currentColP    = reverse ? pw_cols - 1 : 0;
  pw_colPLastUpdate = millis();
  pw_colPState      = PW_COLP_RUNNING;
}

// Call every loop() iteration to run the palette column effect
static void updatePussywillowColP(CRGBPalette16 palette, uint32_t intervalMs,
                                   uint32_t holdMs, bool reverse = false,
                                   uint8_t paletteSpeed = 25) {
  if (pw_currentColP == -1) _pw_initColP(reverse);

  uint32_t now = millis();

  if (pw_colPState == PW_COLP_RUNNING) {
    if (now - pw_colPLastUpdate < intervalMs) return;
    pw_colPLastUpdate = now;

    CRGB color = ColorFromPalette(palette, pw_colPOffset, 255, LINEARBLEND);
    pw_colPOffset += paletteSpeed;

    for (uint8_t r = 0; r < pw_rows; r++)
      leds[pw_ledMap[r][pw_currentColP]] = color;
    FastLED.show();

    bool done = pw_colPReverse ? (pw_currentColP == 0)
                               : (pw_currentColP == pw_cols - 1);
    if (done) {
      pw_colPHoldStart = now;
      pw_colPState     = PW_COLP_HOLD;
    } else {
      pw_currentColP += pw_colPReverse ? -1 : 1;
    }

  } else {  // PW_COLP_HOLD
    if (now - pw_colPHoldStart < holdMs) return;

    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    FastLED.show();
    pw_currentColP    = pw_colPReverse ? pw_cols - 1 : 0;
    pw_colPLastUpdate = millis();
    pw_colPState      = PW_COLP_RUNNING;
  }
}


//
//  Clockwise        (reverse = false):
//    Rows travel bottom → top   (row 14 → row 0)
//    Within each row, LEDs light left → right  (col 0 → col 9)
//
//  Counter-clockwise (reverse = true):
//    Rows travel top → bottom   (row 0 → row 14)
//    Within each row, LEDs light right → left  (col 9 → col 0)
//
//  When the entire matrix is filled it holds for holdMs, then clears and
//  restarts from the beginning.
// ═════════════════════════════════════════════════════════════════════════════

enum PW_DotFillState { PW_DOTFILL_RUNNING, PW_DOTFILL_HOLD };

static PW_DotFillState pw_dotFillState      = PW_DOTFILL_RUNNING;
static int8_t       pw_dotFillRow        = -1;   // -1 = not yet initialised
static int8_t       pw_dotFillCol        = -1;
static uint32_t     pw_dotFillLastUpdate = 0;
static uint32_t     pw_dotFillHoldStart  = 0;
static bool         pw_dotFillReverse    = false;

static void _pw_initDotFill(bool reverse) {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_dotFillReverse    = reverse;
  pw_dotFillRow        = reverse ? 0           : pw_rows - 1;  // CW: start bottom, CCW: start top
  pw_dotFillCol        = reverse ? pw_cols - 1 : 0;            // CW: start left,   CCW: start right
  pw_dotFillLastUpdate = millis();
  pw_dotFillState      = PW_DOTFILL_RUNNING;
}

// Call every loop() iteration to run the fill effect
static void updatePussywillowDotFill(CRGB color, uint32_t intervalMs, uint32_t holdMs, bool reverse = false) {
  if (pw_dotFillRow == -1) _pw_initDotFill(reverse);   // auto-init on first call

  uint32_t now = millis();

  if (pw_dotFillState == PW_DOTFILL_RUNNING) {
    if (now - pw_dotFillLastUpdate < intervalMs) return;
    pw_dotFillLastUpdate = now;

    // Light the current LED
    leds[pw_ledMap[pw_dotFillRow][pw_dotFillCol]] = color;
    FastLED.show();

    // ── Determine whether this is the very last LED of the whole matrix ──
    bool lastCol = pw_dotFillReverse ? (pw_dotFillCol == 0)
                                  : (pw_dotFillCol == pw_cols - 1);
    bool lastRow = pw_dotFillReverse ? (pw_dotFillRow == pw_rows - 1)
                                  : (pw_dotFillRow == 0);

    if (lastCol && lastRow) {
      // Entire matrix is now filled – enter hold
      pw_dotFillHoldStart = now;
      pw_dotFillState     = PW_DOTFILL_HOLD;
      return;
    }

    if (lastCol) {
      // Finished this row – advance to the next row and reset the column
      pw_dotFillRow += pw_dotFillReverse ? 1 : -1;
      pw_dotFillCol  = pw_dotFillReverse ? pw_cols - 1 : 0;
    } else {
      // Still within the current row – advance one column
      pw_dotFillCol += pw_dotFillReverse ? -1 : 1;
    }

  } else {  // PW_DOTFILL_HOLD
    if (now - pw_dotFillHoldStart < holdMs) return;

    // Clear everything and restart
    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    FastLED.show();
    pw_dotFillRow        = pw_dotFillReverse ? 0           : pw_rows - 1;
    pw_dotFillCol        = pw_dotFillReverse ? pw_cols - 1 : 0;
    pw_dotFillLastUpdate = millis();
    pw_dotFillState      = PW_DOTFILL_RUNNING;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  FILL EFFECT  –  palette colour, one LED at a time
//
//  Identical behaviour to updatePussywillowDotFill() but each LED is coloured
//  from a CRGBPalette16.  The palette offset advances by paletteSpeed on every
//  single LED step so the colours shift continuously as the matrix fills in,
//  creating a flowing colour wash across the whole display.
//
//  Call signature:
//    updatePussywillowDotFillP(palette, intervalMs, holdMs, reverse, paletteSpeed)
//      palette       – CRGBPalette16 to colour each LED
//      intervalMs    – milliseconds between each individual LED step
//      holdMs        – milliseconds to hold when the matrix is full before clearing
//      reverse       – false = clockwise: rows bottom → top, LEDs left → right
//                      true  = counter-clockwise: rows top → bottom, LEDs right → left
//      paletteSpeed  – how much the palette index advances each LED step (1–10)
//                      lower = smoother wash across the matrix
//                      higher = faster colour cycling
// ═════════════════════════════════════════════════════════════════════════════

enum PW_DotFillPState { PW_DOTFILLP_RUNNING, PW_DOTFILLP_HOLD };

static PW_DotFillPState pw_dotFillPState      = PW_DOTFILLP_RUNNING;
static int8_t           pw_dotFillPRow        = -1;   // -1 = not yet initialised
static int8_t           pw_dotFillPCol        = -1;
static uint32_t         pw_dotFillPLastUpdate = 0;
static uint32_t         pw_dotFillPHoldStart  = 0;
static bool             pw_dotFillPReverse    = false;
static uint8_t          pw_dotFillPOffset     = 0;   // cycling palette offset

static void _pw_initDotFillP(bool reverse) {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_dotFillPReverse    = reverse;
  pw_dotFillPRow        = reverse ? 0           : pw_rows - 1;
  pw_dotFillPCol        = reverse ? pw_cols - 1 : 0;
  pw_dotFillPOffset     = 0;
  pw_dotFillPLastUpdate = millis();
  pw_dotFillPState      = PW_DOTFILLP_RUNNING;
}

// Call every loop() iteration to run the palette dot-fill effect
static void updatePussywillowDotFillP(CRGBPalette16 palette, uint32_t intervalMs,
                                       uint32_t holdMs, bool reverse = false,
                                       uint8_t paletteSpeed = 2) {
  if (pw_dotFillPRow == -1) _pw_initDotFillP(reverse);

  uint32_t now = millis();

  if (pw_dotFillPState == PW_DOTFILLP_RUNNING) {
    if (now - pw_dotFillPLastUpdate < intervalMs) return;
    pw_dotFillPLastUpdate = now;

    // Look up colour and advance offset for next LED
    CRGB color = ColorFromPalette(palette, pw_dotFillPOffset, 255, LINEARBLEND);
    pw_dotFillPOffset += paletteSpeed;

    leds[pw_ledMap[pw_dotFillPRow][pw_dotFillPCol]] = color;
    FastLED.show();

    bool lastCol = pw_dotFillPReverse ? (pw_dotFillPCol == 0)
                                      : (pw_dotFillPCol == pw_cols - 1);
    bool lastRow = pw_dotFillPReverse ? (pw_dotFillPRow == pw_rows - 1)
                                      : (pw_dotFillPRow == 0);

    if (lastCol && lastRow) {
      pw_dotFillPHoldStart = now;
      pw_dotFillPState     = PW_DOTFILLP_HOLD;
      return;
    }

    if (lastCol) {
      pw_dotFillPRow += pw_dotFillPReverse ? 1 : -1;
      pw_dotFillPCol  = pw_dotFillPReverse ? pw_cols - 1 : 0;
    } else {
      pw_dotFillPCol += pw_dotFillPReverse ? -1 : 1;
    }

  } else {  // PW_DOTFILLP_HOLD
    if (now - pw_dotFillPHoldStart < holdMs) return;

    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    FastLED.show();
    pw_dotFillPRow        = pw_dotFillPReverse ? 0           : pw_rows - 1;
    pw_dotFillPCol        = pw_dotFillPReverse ? pw_cols - 1 : 0;
    pw_dotFillPLastUpdate = millis();
    pw_dotFillPState      = PW_DOTFILLP_RUNNING;
  }
}


//
//  One row is lit at a time.  The previous row is cleared each step so only
//  one row glows at any time.
//
//  Two motion modes selected by the pingPong parameter:
//
//  pingPong = true  (bounce):
//    The row travels from one end to the other, pauses for holdMs, then
//    reverses and travels back.  Bounces indefinitely.
//
//  pingPong = false (loop):
//    The row travels from the start end to the far end, pauses for holdMs,
//    then snaps invisibly back to the start and repeats — always travelling
//    in the same direction.
//
//  reverse = false : start end is the bottom (row 14), travel upward first
//  reverse = true  : start end is the top    (row  0), travel downward first
//
//  Call signature:
//    updatePussywillowRowBounce(color, intervalMs, holdMs, reverse, pingPong)
//      color       – CRGB solid colour for the lit row
//      intervalMs  – milliseconds between each row step
//      holdMs      – milliseconds to pause at the end before reversing/looping
//      reverse     – false = start bottom, travel up first
//                    true  = start top,    travel down first
//      pingPong    – true  = bounce back and forth
//                    false = one-way loop, snap back to start each cycle
// ═════════════════════════════════════════════════════════════════════════════

enum PW_RowBounceState { PW_ROWBOUNCE_RUNNING, PW_ROWBOUNCE_HOLD };

static PW_RowBounceState pw_rowBounceState      = PW_ROWBOUNCE_RUNNING;
static int8_t            pw_rowBounceRow        = -1;   // -1 = not yet initialised
static bool              pw_rowBounceGoingUp    = true; // current travel direction
static bool              pw_rowBouncePingPong   = true; // true=bounce, false=loop
static bool              pw_rowBounceReverse    = false;// stored start direction
static uint32_t          pw_rowBounceLastUpdate = 0;
static uint32_t          pw_rowBounceHoldStart  = 0;

static void _pw_initRowBounce(bool reverse, bool pingPong) {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_rowBounceReverse    = reverse;
  pw_rowBouncePingPong   = pingPong;
  pw_rowBounceRow        = reverse ? 0 : pw_rows - 1;
  pw_rowBounceGoingUp    = !reverse;
  pw_rowBounceLastUpdate = millis();
  pw_rowBounceState      = PW_ROWBOUNCE_RUNNING;
}

// Call every loop() iteration to run the single-row bounce/loop effect
static void updatePussywillowRowBounce(CRGB color, uint32_t intervalMs, uint32_t holdMs, bool reverse = false, bool pingPong = true) {
  if (pw_rowBounceRow == -1) _pw_initRowBounce(reverse, pingPong);   // auto-init on first call

  uint32_t now = millis();

  if (pw_rowBounceState == PW_ROWBOUNCE_RUNNING) {
    if (now - pw_rowBounceLastUpdate < intervalMs) return;
    pw_rowBounceLastUpdate = now;

    // Clear all rows, then light only the current one
    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[pw_rowBounceRow][c]] = color;
    FastLED.show();

    bool atTop    = (pw_rowBounceRow == 0);
    bool atBottom = (pw_rowBounceRow == pw_rows - 1);
    bool atFarEnd = pw_rowBounceGoingUp ? atTop : atBottom;

    if (atFarEnd) {
      // Reached the far end – hold, then either reverse or snap back to start
      pw_rowBounceHoldStart = now;
      pw_rowBounceState     = PW_ROWBOUNCE_HOLD;
    } else {
      pw_rowBounceRow += pw_rowBounceGoingUp ? -1 : 1;
    }

  } else {  // PW_ROWBOUNCE_HOLD
    if (now - pw_rowBounceHoldStart < holdMs) return;

    if (pw_rowBouncePingPong) {
      // Bounce mode – reverse direction
      pw_rowBounceGoingUp = !pw_rowBounceGoingUp;
    } else {
      // Loop mode – snap back to the start row, keep same direction
      pw_rowBounceRow = pw_rowBounceReverse ? 0 : pw_rows - 1;
    }
    pw_rowBounceLastUpdate = millis();
    pw_rowBounceState      = PW_ROWBOUNCE_RUNNING;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  SINGLE ROW BOUNCE EFFECT  –  palette colour
//
//  Identical motion behaviour to updatePussywillowRowBounce() above, but each
//  LED in the lit row is coloured from a CRGBPalette16.  The palette is mapped
//  across the 10 columns of the row (col 0 → palette index 0,
//  col 9 → palette index 255) so the full palette gradient is always visible
//  in the single lit row.
//
//  pingPong = true  (bounce):  travels to far end, pauses, reverses back
//  pingPong = false (loop):    travels to far end, pauses, snaps back to start
//
//  Call signature:
//    updatePussywillowRowBouncePalette(palette, intervalMs, holdMs, reverse, pingPong)
//      palette     – CRGBPalette16 to colour the lit row
//      intervalMs  – milliseconds between each row step
//      holdMs      – milliseconds to pause at the end before reversing/looping
//      reverse     – false = start bottom, travel up first
//                    true  = start top,    travel down first
//      pingPong    – true  = bounce back and forth
//                    false = one-way loop, snap back to start each cycle
// ═════════════════════════════════════════════════════════════════════════════

enum PW_RowBouncePalState { PW_ROWBOUNCEPAL_RUNNING, PW_ROWBOUNCEPAL_HOLD };

static PW_RowBouncePalState pw_rowBouncePalState      = PW_ROWBOUNCEPAL_RUNNING;
static int8_t               pw_rowBouncePalRow        = -1;   // -1 = not yet initialised
static bool                 pw_rowBouncePalGoingUp    = true;
static bool                 pw_rowBouncePalPingPong   = true; // true=bounce, false=loop
static bool                 pw_rowBouncePalReverse    = false;// stored start direction
static uint32_t             pw_rowBouncePalLastUpdate = 0;
static uint32_t             pw_rowBouncePalHoldStart  = 0;

static void _pw_initRowBouncePal(bool reverse, bool pingPong) {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_rowBouncePalReverse    = reverse;
  pw_rowBouncePalPingPong   = pingPong;
  pw_rowBouncePalRow        = reverse ? 0 : pw_rows - 1;
  pw_rowBouncePalGoingUp    = !reverse;
  pw_rowBouncePalLastUpdate = millis();
  pw_rowBouncePalState      = PW_ROWBOUNCEPAL_RUNNING;
}

// Call every loop() iteration to run the palette single-row bounce/loop effect
static void updatePussywillowRowBouncePalette(CRGBPalette16 palette, uint32_t intervalMs, uint32_t holdMs, bool reverse = false, bool pingPong = true) {
  if (pw_rowBouncePalRow == -1) _pw_initRowBouncePal(reverse, pingPong);   // auto-init on first call

  uint32_t now = millis();

  if (pw_rowBouncePalState == PW_ROWBOUNCEPAL_RUNNING) {
    if (now - pw_rowBouncePalLastUpdate < intervalMs) return;
    pw_rowBouncePalLastUpdate = now;

    // Clear all rows, then paint the current row with the palette gradient
    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    for (uint8_t c = 0; c < pw_cols; c++) {
      uint8_t paletteIndex = map(c, 0, pw_cols - 1, 0, 255);
      leds[pw_ledMap[pw_rowBouncePalRow][c]] = ColorFromPalette(palette, paletteIndex, 255, LINEARBLEND);
    }
    FastLED.show();

    bool atTop    = (pw_rowBouncePalRow == 0);
    bool atBottom = (pw_rowBouncePalRow == pw_rows - 1);
    bool atFarEnd = pw_rowBouncePalGoingUp ? atTop : atBottom;

    if (atFarEnd) {
      pw_rowBouncePalHoldStart = now;
      pw_rowBouncePalState     = PW_ROWBOUNCEPAL_HOLD;
    } else {
      pw_rowBouncePalRow += pw_rowBouncePalGoingUp ? -1 : 1;
    }

  } else {  // PW_ROWBOUNCEPAL_HOLD
    if (now - pw_rowBouncePalHoldStart < holdMs) return;

    if (pw_rowBouncePalPingPong) {
      // Bounce mode – reverse direction
      pw_rowBouncePalGoingUp = !pw_rowBouncePalGoingUp;
    } else {
      // Loop mode – snap back to start row, keep same direction
      pw_rowBouncePalRow = pw_rowBouncePalReverse ? 0 : pw_rows - 1;
    }
    pw_rowBouncePalLastUpdate = millis();
    pw_rowBouncePalState      = PW_ROWBOUNCEPAL_RUNNING;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  SINELON EFFECT  –  sweeping dot with fading tail
//
//  A coloured dot sweeps back and forth across the matrix rows, following the
//  visual row order (row 0 = top, row 14 = bottom).  Each step the entire
//  matrix fades toward black, leaving a decaying trail behind the dot.
//  Gaps between the dot's current and previous position are filled so the
//  sweep always appears continuous with no missing rows.
//
//  The sweep position is driven by beatsin16() so it runs entirely on its own
//  internal clock — no intervalMs or holdMs are needed.
//
//  reverse = false : dot sweeps bottom → top → bottom  (default)
//  reverse = true  : dot sweeps top → bottom → top     (phase-inverted)
//
//  Call signature:
//    updatePussywillowSinelon(color, bpm, fadeAmount, saturation, reverse)
//      color       – CRGB base colour for the dot and tail (hue is taken from
//                    this colour; saturation and value are overridden by the
//                    saturation parameter and full brightness)
//      bpm         – sweep speed in beats per minute (replaces the hardcoded
//                    13 in the original sinelon); higher = faster sweep
//      fadeAmount  – how quickly the tail fades to black each frame (1–255);
//                    lower = longer tail, higher = shorter tail
//      saturation  – colour saturation of the dot (0 = white, 255 = fully
//                    saturated); default 220
//      reverse     – false = sweep starts from bottom
//                    true  = sweep starts from top (phase inverted)
// ═════════════════════════════════════════════════════════════════════════════

static bool     pw_sinelonInitDone   = false;
static bool     pw_sinelonFirstFrame = true;  // suppress fade on first frame
static uint16_t pw_sinelonPrevPos    = 0;
static uint32_t pw_sinelonTimeBase   = 0;   // millis() snapshot taken at init

static void _pw_initSinelon() {
  // Hard-clear the entire LED array so nothing from a previous effect remains
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_sinelonTimeBase   = millis();       // anchor the wave to now
  pw_sinelonPrevPos    = pw_rows - 1;   // start tracking from the bottom row
  pw_sinelonFirstFrame = true;
  pw_sinelonInitDone   = true;
}

// Call every loop() iteration to run the sinelon effect
static void updatePussywillowSinelon(CRGB color, uint8_t bpm = 13, uint8_t fadeAmount = 20, uint8_t saturation = 220, bool reverse = false) {
  if (!pw_sinelonInitDone) _pw_initSinelon();   // auto-init on first call

  // Extract hue from the supplied colour
  CHSV hsv = rgb2hsv_approximate(color);
  uint8_t hue = hsv.h;

  // Compute current dot row position via beatsin16.
  // Phase offset 49152 (= 3/4 of 65536) places the sine at its trough at
  // t=0. We then invert the output (pw_rows-1 minus rawPos) so that the
  // trough maps to row 14 (bottom) instead of row 0 (top).
  uint16_t phaseOffset = reverse ? 49152 : 49152;  // both start from trough; reverse flips the inversion below
  uint16_t rawPos = beatsin16(bpm, 0, pw_rows - 1, pw_sinelonTimeBase, phaseOffset);
  uint16_t pos    = reverse ? rawPos : (pw_rows - 1 - rawPos);

  if (pw_sinelonFirstFrame) {
    // On the very first frame, do a guaranteed hard clear of the full array
    // so no residual brightness from any previous effect can appear.
    // Skip the fade entirely — start from pure black.
    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;
    // Seed prevPos to the actual current beatsin16 position so the fill
    // loop on this first frame only lights the single starting row.
    pw_sinelonPrevPos    = pos;
    pw_sinelonFirstFrame = false;
  } else {
    // Normal operation — fade the whole matrix toward black for the tail
    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]].nscale8(255 - fadeAmount);
  }

  // Fill between previous and current position so there are no visual gaps
  if (pos < pw_sinelonPrevPos) {
    for (uint16_t r = pos; r <= pw_sinelonPrevPos; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CHSV(hue, saturation, 255);
  } else {
    for (uint16_t r = pw_sinelonPrevPos; r <= pos; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CHSV(hue, saturation, 255);
  }

  FastLED.show();
  pw_sinelonPrevPos = pos;
}


// ═════════════════════════════════════════════════════════════════════════════
//  BREATHE EFFECT  –  each branch inhales and exhales independently
//
//  Each of the 10 columns (branches) fades in and out at its own pace, driven
//  by a sine wave with a unique phase offset per column.  The result looks
//  like the branches are breathing — some are brightening while others are
//  dimming, creating a slow organic undulation across the whole spray.
//
//  All LEDs within a branch share the same brightness at any moment, but the
//  colour can be solid or shift slowly across the branches using a palette.
//
//  Call signature:
//    updatePussywillowBreathe(color, bpm, minBright, maxBright)
//      color      – CRGB base colour for all branches
//      bpm        – overall breathing rate in beats per minute;
//                   each branch is offset by 1/10 of a cycle from its
//                   neighbour so they cycle through one at a time
//      minBright  – minimum brightness at the bottom of the breath (0–255)
//      maxBright  – maximum brightness at the top of the breath (0–255)
// ═════════════════════════════════════════════════════════════════════════════

static bool     pw_breatheInitDone  = false;
static uint32_t pw_breatheTimeBase  = 0;
static uint32_t pw_breatheLastFrame = 0;

static void _pw_initBreathe() {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_breatheTimeBase  = millis();
  pw_breatheLastFrame = millis();
  pw_breatheInitDone  = true;
}

// Call every loop() iteration to run the breathe effect
static void updatePussywillowBreathe(CRGB color, uint8_t bpm = 10, uint8_t minBright = 10, uint8_t maxBright = 255) {
  if (!pw_breatheInitDone) _pw_initBreathe();

  // Limit to ~50 fps
  uint32_t now = millis();
  if (now - pw_breatheLastFrame < 20) return;
  pw_breatheLastFrame = now;

  CHSV hsv = rgb2hsv_approximate(color);
  uint32_t elapsed = now - pw_breatheTimeBase;
  uint32_t cycleMs = 60000UL / bpm;

  for (uint8_t c = 0; c < pw_cols; c++) {
    uint32_t phaseMs = ((uint32_t)c * cycleMs) / pw_cols;
    uint32_t t       = (elapsed + phaseMs) % cycleMs;
    uint8_t  angle   = (uint8_t)((t * 256UL) / cycleMs);
    uint8_t  sinVal  = sin8(angle);
    uint8_t  bright  = map(sinVal, 0, 255, minBright, maxBright);
    CRGB colColor = CHSV(hsv.h, hsv.s, bright);
    for (uint8_t r = 0; r < pw_rows; r++)
      leds[pw_ledMap[r][c]] = colColor;
  }

  FastLED.show();
}

// ═════════════════════════════════════════════════════════════════════════════
//  BREATHE PALETTE EFFECT  –  each branch breathes independently, palette colour
//
//  Identical breathing behaviour to updatePussywillowBreathe() — each branch
//  fades in and out with its own phase offset so the brightness ripples one
//  branch at a time across the spray.
//
//  Instead of a single solid colour, each branch takes its colour from a
//  CRGBPalette16, mapped evenly across the 10 columns (col 0 → palette index 0,
//  col 9 → palette index 255).  The result is a full palette gradient spread
//  across the branches, all slowly breathing in turn.
//
//  Call signature:
//    updatePussywillowBreathePalette(palette, bpm, minBright, maxBright)
//      palette    – CRGBPalette16 to colour the branches
//      bpm        – overall breathing rate in beats per minute
//      minBright  – minimum brightness at the bottom of the breath (0–255)
//      maxBright  – maximum brightness at the top of the breath (0–255)
// ═════════════════════════════════════════════════════════════════════════════

static bool     pw_breathePalInitDone  = false;
static uint32_t pw_breathePalTimeBase  = 0;
static uint32_t pw_breathePalLastFrame = 0;

static void _pw_initBreathePal() {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_breathePalTimeBase  = millis();
  pw_breathePalLastFrame = millis();
  pw_breathePalInitDone  = true;
}

// Call every loop() iteration to run the palette breathe effect
static void updatePussywillowBreathePalette(CRGBPalette16 palette, uint8_t bpm = 10, uint8_t minBright = 10, uint8_t maxBright = 255) {
  if (!pw_breathePalInitDone) _pw_initBreathePal();

  // Limit to ~50 fps
  uint32_t now = millis();
  if (now - pw_breathePalLastFrame < 20) return;
  pw_breathePalLastFrame = now;

  uint32_t elapsed = now - pw_breathePalTimeBase;
  uint32_t cycleMs = 60000UL / bpm;

  for (uint8_t c = 0; c < pw_cols; c++) {
    uint32_t phaseMs      = ((uint32_t)c * cycleMs) / pw_cols;
    uint32_t t            = (elapsed + phaseMs) % cycleMs;
    uint8_t  angle        = (uint8_t)((t * 256UL) / cycleMs);
    uint8_t  sinVal       = sin8(angle);
    uint8_t  bright       = map(sinVal, 0, 255, minBright, maxBright);
    uint8_t  paletteIndex = map(c, 0, pw_cols - 1, 0, 255);

    // Get the hue from the palette by looking it up at full white brightness,
    // then convert to HSV and drive brightness through the V channel only.
    // This avoids any per-channel RGB scaling which causes colour-dependent
    // flickering with hue-varying palettes like RainbowColors_p.
    CRGB     fullColor    = ColorFromPalette(palette, paletteIndex, 255, LINEARBLEND);
    CHSV     hsv          = rgb2hsv_approximate(fullColor);
    CRGB     colColor     = CHSV(hsv.h, hsv.s, bright);

    for (uint8_t r = 0; r < pw_rows; r++)
      leds[pw_ledMap[r][c]] = colColor;
  }

  FastLED.show();
}

// ═════════════════════════════════════════════════════════════════════════════
//  METEOR EFFECT  –  bright streaks shoot up random branches with fading tails
//
//  Multiple meteors streak upward from the base to the tip of randomly chosen
//  branches simultaneously.  Each meteor has a bright head and a linearly
//  fading tail behind it.  When a meteor finishes it waits a random gap then
//  re-spawns on a new random branch, so all 10 branches get coverage over time.
//  The whole matrix fades toward black each frame to create the tail decay.
//
//  The number of concurrent meteors is set by the PW_NUM_METEORS constant
//  (default 10).  Each meteor self-initialises and manages its own state.
//  Each meteor picks a fully random hue at spawn so the streaks vary in
//  colour naturally over time.
//
//  Call signature:
//    updatePussywillowMeteor(tailLength, minSpawnGap, maxSpawnGap, frameInterval)
//      tailLength    – number of fading tail LEDs behind the head (1–12)
//      minSpawnGap   – minimum ms a meteor waits before re-spawning (e.g. 400)
//      maxSpawnGap   – maximum ms a meteor waits before re-spawning (e.g. 1200)
//      frameInterval – ms between rendered frames — controls overall speed (e.g. 30–80)
// ═════════════════════════════════════════════════════════════════════════════

#define PW_NUM_METEORS 10

struct PW_MeteorState {
  uint8_t  branch;
  uint8_t  pos;        // 0..pw_rows-1 active, 255 = inactive
  uint8_t  speed;      // 0 = not yet initialised (sentinel)
  uint8_t  hue;
  uint32_t spawnTime;
};

static PW_MeteorState pw_meteors[PW_NUM_METEORS];
static uint32_t       pw_meteorLastFrame  = 0;
static bool           pw_meteorInitDone   = false;

static void _pw_initMeteor() {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  // Mark all meteors as uninitialised — speed==0 is the sentinel
  for (uint8_t m = 0; m < PW_NUM_METEORS; m++)
    pw_meteors[m].speed = 0;
  pw_meteorLastFrame = millis();
  pw_meteorInitDone  = true;
}

// Call every loop() iteration to run the meteor effect
static void updatePussywillowMeteor(uint8_t tailLength = 5,
                                     uint16_t minSpawnGap = 400,
                                     uint16_t maxSpawnGap = 1200,
                                     uint8_t  frameInterval = 40) {
  if (!pw_meteorInitDone) _pw_initMeteor();

  uint32_t now = millis();
  if (now - pw_meteorLastFrame < frameInterval) return;
  pw_meteorLastFrame = now;

  // Fade the whole matrix toward black — creates tail decay and clears
  // finished meteors
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]].nscale8(220);  // ~fadeToBlackBy 35

  for (uint8_t m = 0; m < PW_NUM_METEORS; m++) {

    // Self-initialise on very first call — stagger spawns by index
    if (pw_meteors[m].speed == 0) {
      pw_meteors[m].branch    = m % pw_cols;
      pw_meteors[m].pos       = 255;               // inactive until gap elapses
      pw_meteors[m].speed     = 1;                 // placeholder; re-randomised at spawn
      pw_meteors[m].hue       = m * (256 / PW_NUM_METEORS);  // evenly spread initial hues
      pw_meteors[m].spawnTime = (uint32_t)m * 300UL;          // stagger initial spawns
    }

    if (pw_meteors[m].pos == 255) {
      // Inactive — wait for random gap then spawn on a fresh random branch
      if ((now - pw_meteors[m].spawnTime) >= (uint32_t)random16(minSpawnGap, maxSpawnGap)) {
        pw_meteors[m].branch    = random8(pw_cols);
        pw_meteors[m].pos       = 0;         // start at base (row 14)
        pw_meteors[m].speed     = random8(1, 3);
        pw_meteors[m].hue       = random8(); // fully random hue each spawn
        pw_meteors[m].spawnTime = now;
      }
    } else {
      uint8_t col = pw_meteors[m].branch;
      uint8_t p   = pw_meteors[m].pos;       // 0 = base, pw_rows-1 = tip

      // Convert pos (0=base) to row index (pw_rows-1=base, 0=tip)
      uint8_t headRow = (pw_rows - 1) - p;

      // Head — full brightness
      leds[pw_ledMap[headRow][col]] = CHSV(pw_meteors[m].hue, 220, 255);

      // Tail — brightness fades linearly behind the head (toward base)
      for (uint8_t t = 1; t <= tailLength; t++) {
        if (p >= t) {
          uint8_t tailRow = (pw_rows - 1) - (p - t);
          uint8_t tailBri = 255 * (tailLength - t + 1) / (tailLength + 1);
          leds[pw_ledMap[tailRow][col]] |= CHSV(pw_meteors[m].hue, 220, tailBri);
        }
      }

      pw_meteors[m].pos += pw_meteors[m].speed;

      if (pw_meteors[m].pos >= pw_rows) {
        pw_meteors[m].pos       = 255;  // inactive sentinel
        pw_meteors[m].spawnTime = now;
      }
    }
  }

  FastLED.show();
}

// ═════════════════════════════════════════════════════════════════════════════
//  DUAL ROW COUNTER effect  –  two rows travelling in opposite directions
//
//  One row starts at the bottom and travels upward while a second row starts
//  at the top and travels downward simultaneously.  When both rows reach their
//  far ends they hold for holdMs, then both reverse and travel back.  The rows
//  pass through each other cleanly — when they share the same row position both
//  colours are blended using the |= operator so neither is lost.
//
//  Both rows are always visible at the same time.  The matrix is cleared and
//  both rows are redrawn each step so only the two active rows glow.
//
//  Call signature:
//    updatePussywillowDualRow(colorUp, colorDown, intervalMs, holdMs)
//      colorUp     – CRGB colour of the row travelling upward (bottom → top)
//      colorDown   – CRGB colour of the row travelling downward (top → bottom)
//      intervalMs  – milliseconds between each row step
//      holdMs      – milliseconds to pause at each end before reversing
// ═════════════════════════════════════════════════════════════════════════════

enum PW_DualRowState { PW_DUALROW_RUNNING, PW_DUALROW_HOLD };

static PW_DualRowState pw_dualRowState      = PW_DUALROW_RUNNING;
static int8_t          pw_dualRowUp         = -1;   // -1 = not yet initialised
static int8_t          pw_dualRowDown       = 0;
static bool            pw_dualRowGoingUp    = true; // true = up row moving upward
static uint32_t        pw_dualRowLastUpdate = 0;
static uint32_t        pw_dualRowHoldStart  = 0;

static void _pw_initDualRow() {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_dualRowUp         = pw_rows - 1;   // upward row starts at bottom
  pw_dualRowDown       = 0;             // downward row starts at top
  pw_dualRowGoingUp    = true;
  pw_dualRowLastUpdate = millis();
  pw_dualRowState      = PW_DUALROW_RUNNING;
}

// Call every loop() iteration to run the dual row effect
static void updatePussywillowDualRow(CRGB colorUp, CRGB colorDown,
                                      uint32_t intervalMs, uint32_t holdMs) {
  if (pw_dualRowUp == -1) _pw_initDualRow();   // auto-init on first call

  uint32_t now = millis();

  if (pw_dualRowState == PW_DUALROW_RUNNING) {
    if (now - pw_dualRowLastUpdate < intervalMs) return;
    pw_dualRowLastUpdate = now;

    // Clear the whole matrix then draw both rows
    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;

    // Draw upward row
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[pw_dualRowUp][c]] = colorUp;

    // Draw downward row — use |= so colours blend cleanly when rows cross
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[pw_dualRowDown][c]] |= colorDown;

    FastLED.show();

    // Check if both rows have reached their far ends
    bool upDone   = pw_dualRowGoingUp  ? (pw_dualRowUp   == 0)
                                       : (pw_dualRowUp   == pw_rows - 1);
    bool downDone = pw_dualRowGoingUp  ? (pw_dualRowDown == pw_rows - 1)
                                       : (pw_dualRowDown == 0);

    if (upDone && downDone) {
      pw_dualRowHoldStart = now;
      pw_dualRowState     = PW_DUALROW_HOLD;
    } else {
      // Advance both rows one step
      pw_dualRowUp   += pw_dualRowGoingUp ?  -1 :  1;  // row 0 = top
      pw_dualRowDown += pw_dualRowGoingUp ?   1 : -1;
    }

  } else {  // PW_DUALROW_HOLD
    if (now - pw_dualRowHoldStart < holdMs) return;
    // Reverse both directions and resume
    pw_dualRowGoingUp    = !pw_dualRowGoingUp;
    pw_dualRowLastUpdate = millis();
    pw_dualRowState      = PW_DUALROW_RUNNING;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  DUAL COLUMN COUNTER effect  –  two columns travelling in opposite directions
//
//  One column starts at the left (col 0) and travels rightward while a second
//  column starts at the right (col 9) and travels leftward simultaneously.
//  When both columns reach their far ends they hold for holdMs, then both
//  reverse and travel back.  The columns pass through each other cleanly —
//  when they share the same column position both colours are blended using
//  the |= operator so neither is lost.
//
//  Both columns are always visible at the same time.  The matrix is cleared
//  and both columns are redrawn each step so only the two active columns glow.
//
//  Call signature:
//    updatePussywillowDualCol(colorLeft, colorRight, intervalMs, holdMs)
//      colorLeft   – CRGB colour of the column travelling left → right
//      colorRight  – CRGB colour of the column travelling right → left
//      intervalMs  – milliseconds between each column step
//      holdMs      – milliseconds to pause at each end before reversing
// ═════════════════════════════════════════════════════════════════════════════

enum PW_DualColState { PW_DUALCOL_RUNNING, PW_DUALCOL_HOLD };

static PW_DualColState pw_dualColState      = PW_DUALCOL_RUNNING;
static int8_t          pw_dualColLeft       = -1;   // -1 = not yet initialised
static int8_t          pw_dualColRight      = 0;
static bool            pw_dualColGoingRight = true; // true = left col moving rightward
static uint32_t        pw_dualColLastUpdate = 0;
static uint32_t        pw_dualColHoldStart  = 0;

static void _pw_initDualCol() {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_dualColLeft       = 0;             // left-travelling col starts at col 0
  pw_dualColRight      = pw_cols - 1;   // right-travelling col starts at col 9
  pw_dualColGoingRight = true;
  pw_dualColLastUpdate = millis();
  pw_dualColState      = PW_DUALCOL_RUNNING;
}

// Call every loop() iteration to run the dual column effect
static void updatePussywillowDualCol(CRGB colorLeft, CRGB colorRight,
                                      uint32_t intervalMs, uint32_t holdMs) {
  if (pw_dualColLeft == -1) _pw_initDualCol();   // auto-init on first call

  uint32_t now = millis();

  if (pw_dualColState == PW_DUALCOL_RUNNING) {
    if (now - pw_dualColLastUpdate < intervalMs) return;
    pw_dualColLastUpdate = now;

    // Clear the whole matrix then draw both columns
    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;

    // Draw left-to-right column
    for (uint8_t r = 0; r < pw_rows; r++)
      leds[pw_ledMap[r][pw_dualColLeft]] = colorLeft;

    // Draw right-to-left column — use |= so colours blend cleanly when columns cross
    for (uint8_t r = 0; r < pw_rows; r++)
      leds[pw_ledMap[r][pw_dualColRight]] |= colorRight;

    FastLED.show();

    // Check if both columns have reached their far ends
    bool leftDone  = pw_dualColGoingRight ? (pw_dualColLeft  == pw_cols - 1)
                                          : (pw_dualColLeft  == 0);
    bool rightDone = pw_dualColGoingRight ? (pw_dualColRight == 0)
                                          : (pw_dualColRight == pw_cols - 1);

    if (leftDone && rightDone) {
      pw_dualColHoldStart = now;
      pw_dualColState     = PW_DUALCOL_HOLD;
    } else {
      // Advance both columns one step
      pw_dualColLeft  += pw_dualColGoingRight ?  1 : -1;
      pw_dualColRight += pw_dualColGoingRight ? -1 :  1;
    }

  } else {  // PW_DUALCOL_HOLD
    if (now - pw_dualColHoldStart < holdMs) return;
    // Reverse both directions and resume
    pw_dualColGoingRight = !pw_dualColGoingRight;
    pw_dualColLastUpdate = millis();
    pw_dualColState      = PW_DUALCOL_RUNNING;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  DUAL ROW COUNTER PALETTE effect  –  two rows, colours cycling from palette(s)
//
//  Identical motion behaviour to updatePussywillowDualRow() but each row's
//  colour cycles through a CRGBPalette16 automatically.  An internal palette
//  offset increments each step so the colours shift as the rows travel.
//  The two rows are offset by 128 in the palette so they always show
//  contrasting colours.  Both rows can use the same or different palettes.
//
//  Call signature:
//    updatePussywillowDualRowP(palette1, palette2, intervalMs, holdMs, paletteSpeed)
//      palette1      – CRGBPalette16 for the row travelling upward (bottom → top)
//      palette2      – CRGBPalette16 for the row travelling downward (top → bottom)
//      intervalMs    – milliseconds between each row step
//      holdMs        – milliseconds to pause at each end before reversing
//      paletteSpeed  – how much the palette index advances each step (1–20)
//                      higher = faster colour cycling
// ═════════════════════════════════════════════════════════════════════════════

enum PW_DualRowPState { PW_DUALROWP_RUNNING, PW_DUALROWP_HOLD };

static PW_DualRowPState pw_dualRowPState      = PW_DUALROWP_RUNNING;
static int8_t           pw_dualRowPUp         = -1;   // -1 = not yet initialised
static int8_t           pw_dualRowPDown       = 0;
static bool             pw_dualRowPGoingUp    = true;
static uint32_t         pw_dualRowPLastUpdate = 0;
static uint32_t         pw_dualRowPHoldStart  = 0;
static uint8_t          pw_dualRowPOffset     = 0;    // cycling palette offset

static void _pw_initDualRowP() {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_dualRowPUp         = pw_rows - 1;
  pw_dualRowPDown       = 0;
  pw_dualRowPGoingUp    = true;
  pw_dualRowPOffset     = 0;
  pw_dualRowPLastUpdate = millis();
  pw_dualRowPState      = PW_DUALROWP_RUNNING;
}

// Call every loop() iteration to run the palette dual row effect
static void updatePussywillowDualRowP(CRGBPalette16 palette1, CRGBPalette16 palette2,
                                       uint32_t intervalMs, uint32_t holdMs,
                                       uint8_t paletteSpeed = 5) {
  if (pw_dualRowPUp == -1) _pw_initDualRowP();

  uint32_t now = millis();

  if (pw_dualRowPState == PW_DUALROWP_RUNNING) {
    if (now - pw_dualRowPLastUpdate < intervalMs) return;
    pw_dualRowPLastUpdate = now;

    // Advance the palette offset each step so colours cycle as rows travel.
    // The two rows are offset by 128 so they show contrasting palette colours.
    pw_dualRowPOffset += paletteSpeed;
    CRGB colorUp   = ColorFromPalette(palette1, pw_dualRowPOffset,       255, LINEARBLEND);
    CRGB colorDown = ColorFromPalette(palette2, pw_dualRowPOffset + 128, 255, LINEARBLEND);

    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;

    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[pw_dualRowPUp][c]] = colorUp;

    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[pw_dualRowPDown][c]] |= colorDown;

    FastLED.show();

    bool upDone   = pw_dualRowPGoingUp ? (pw_dualRowPUp   == 0)
                                       : (pw_dualRowPUp   == pw_rows - 1);
    bool downDone = pw_dualRowPGoingUp ? (pw_dualRowPDown == pw_rows - 1)
                                       : (pw_dualRowPDown == 0);

    if (upDone && downDone) {
      pw_dualRowPHoldStart = now;
      pw_dualRowPState     = PW_DUALROWP_HOLD;
    } else {
      pw_dualRowPUp   += pw_dualRowPGoingUp ? -1 :  1;
      pw_dualRowPDown += pw_dualRowPGoingUp ?  1 : -1;
    }

  } else {  // PW_DUALROWP_HOLD
    if (now - pw_dualRowPHoldStart < holdMs) return;
    pw_dualRowPGoingUp    = !pw_dualRowPGoingUp;
    pw_dualRowPLastUpdate = millis();
    pw_dualRowPState      = PW_DUALROWP_RUNNING;
  }
}

// ═════════════════════════════════════════════════════════════════════════════
//  DUAL COLUMN COUNTER PALETTE effect  –  two columns, colours cycling from palette(s)
//
//  Identical motion behaviour to updatePussywillowDualCol() but each column's
//  colour cycles through a CRGBPalette16 automatically.  An internal palette
//  offset increments each step so the colours shift as the columns travel.
//  The two columns are offset by 128 in the palette so they always show
//  contrasting colours.  Both columns can use the same or different palettes.
//
//  Call signature:
//    updatePussywillowDualColP(palette1, palette2, intervalMs, holdMs, paletteSpeed)
//      palette1      – CRGBPalette16 for the column travelling left → right
//      palette2      – CRGBPalette16 for the column travelling right → left
//      intervalMs    – milliseconds between each column step
//      holdMs        – milliseconds to pause at each end before reversing
//      paletteSpeed  – how much the palette index advances each step (1–20)
//                      higher = faster colour cycling
// ═════════════════════════════════════════════════════════════════════════════

enum PW_DualColPState { PW_DUALCOLP_RUNNING, PW_DUALCOLP_HOLD };

static PW_DualColPState pw_dualColPState      = PW_DUALCOLP_RUNNING;
static int8_t           pw_dualColPLeft       = -1;   // -1 = not yet initialised
static int8_t           pw_dualColPRight      = 0;
static bool             pw_dualColPGoingRight = true;
static uint32_t         pw_dualColPLastUpdate = 0;
static uint32_t         pw_dualColPHoldStart  = 0;
static uint8_t          pw_dualColPOffset     = 0;    // cycling palette offset

static void _pw_initDualColP() {
  for (uint8_t r = 0; r < pw_rows; r++)
    for (uint8_t c = 0; c < pw_cols; c++)
      leds[pw_ledMap[r][c]] = CRGB::Black;
  FastLED.show();
  pw_dualColPLeft       = 0;
  pw_dualColPRight      = pw_cols - 1;
  pw_dualColPGoingRight = true;
  pw_dualColPOffset     = 0;
  pw_dualColPLastUpdate = millis();
  pw_dualColPState      = PW_DUALCOLP_RUNNING;
}

// Call every loop() iteration to run the palette dual column effect
static void updatePussywillowDualColP(CRGBPalette16 palette1, CRGBPalette16 palette2,
                                       uint32_t intervalMs, uint32_t holdMs,
                                       uint8_t paletteSpeed = 5) {
  if (pw_dualColPLeft == -1) _pw_initDualColP();

  uint32_t now = millis();

  if (pw_dualColPState == PW_DUALCOLP_RUNNING) {
    if (now - pw_dualColPLastUpdate < intervalMs) return;
    pw_dualColPLastUpdate = now;

    // Advance palette offset each step so colours cycle as columns travel.
    // The two columns are offset by 128 so they show contrasting colours.
    pw_dualColPOffset += paletteSpeed;
    CRGB colorLeft  = ColorFromPalette(palette1, pw_dualColPOffset,       255, LINEARBLEND);
    CRGB colorRight = ColorFromPalette(palette2, pw_dualColPOffset + 128, 255, LINEARBLEND);

    for (uint8_t r = 0; r < pw_rows; r++)
      for (uint8_t c = 0; c < pw_cols; c++)
        leds[pw_ledMap[r][c]] = CRGB::Black;

    for (uint8_t r = 0; r < pw_rows; r++)
      leds[pw_ledMap[r][pw_dualColPLeft]] = colorLeft;

    for (uint8_t r = 0; r < pw_rows; r++)
      leds[pw_ledMap[r][pw_dualColPRight]] |= colorRight;

    FastLED.show();

    bool leftDone  = pw_dualColPGoingRight ? (pw_dualColPLeft  == pw_cols - 1)
                                           : (pw_dualColPLeft  == 0);
    bool rightDone = pw_dualColPGoingRight ? (pw_dualColPRight == 0)
                                           : (pw_dualColPRight == pw_cols - 1);

    if (leftDone && rightDone) {
      pw_dualColPHoldStart = now;
      pw_dualColPState     = PW_DUALCOLP_HOLD;
    } else {
      pw_dualColPLeft  += pw_dualColPGoingRight ?  1 : -1;
      pw_dualColPRight += pw_dualColPGoingRight ? -1 :  1;
    }

  } else {  // PW_DUALCOLP_HOLD
    if (now - pw_dualColPHoldStart < holdMs) return;
    pw_dualColPGoingRight = !pw_dualColPGoingRight;
    pw_dualColPLastUpdate = millis();
    pw_dualColPState      = PW_DUALCOLP_RUNNING;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
//  RESET ALL PUSSYWILLOW EFFECTS
//  Call this whenever you restart a performance so every effect re-initialises
//  itself from scratch on its next loop() call, exactly as it did on power-up.
//  Place a call to resetPussywillowEffects() inside RestartPerformance().
// ─────────────────────────────────────────────────────────────────────────────
static void resetPussywillowEffects() {
  pw_currentRow          = -1;     // row fill effect
  pw_currentCol          = -1;     // column fill effect
  pw_currentRowP         = -1;     // row fill effect (palette)
  pw_currentColP         = -1;     // column fill effect (palette)
  pw_dotFillRow          = -1;     // dot fill effect
  pw_dotFillCol          = -1;
  pw_dotFillPRow         = -1;     // dot fill effect (palette)
  pw_dotFillPCol         = -1;
  pw_rowBounceRow        = -1;     // single-row bounce (solid)
  pw_rowBouncePalRow     = -1;     // single-row bounce (palette)
  pw_sinelonInitDone     = false;  // sinelon sweep
  pw_sinelonFirstFrame   = true;
  pw_breatheInitDone     = false;  // breathe (solid)
  pw_breatheTimeBase     = 0;
  pw_breatheLastFrame    = 0;
  pw_breathePalInitDone  = false;  // breathe (palette)
  pw_breathePalTimeBase  = 0;
  pw_breathePalLastFrame = 0;
  pw_meteorInitDone      = false;  // meteor
  pw_dualRowUp           = -1;     // dual row counter (solid)
  pw_dualRowPUp          = -1;     // dual row counter (palette)
  pw_dualColLeft         = -1;     // dual column counter (solid)
  pw_dualColPLeft        = -1;     // dual column counter (palette)
}
