#pragma once
#include <FastLED.h>
#include "MatrixConfig.h" // for NUM_JARS, MATRIX_ROWS, MATRIX_COLS, BRIGHTNESS

/*
 * CD77_slowchangepalette_PerJar.h
 *
 * Per-jar-safe variant of FadeToPaletteAnimation (see CD77_slowchangepalette.h
 * for the original, full documentation, and license).
 *
 * WHY THIS VARIANT EXISTS:
 * FadeToPaletteAnimation ramps brightness by calling FastLED.setBrightness(),
 * which is a GLOBAL setting that scales the ENTIRE strip's output -- not just
 * the LEDs that instance owns. That's fine when only one instance ever runs
 * (e.g. fading the whole 4-jar strip as a single block of color). But if you
 * want each of the 4 jars fading independently AT THE SAME TIME, each with
 * its own brightness ramp, four instances all calling FastLED.setBrightness()
 * every frame would fight over that one global value -- whichever instance's
 * Update() runs last in that loop() pass would silently override the other
 * three jars' brightness too, causing flicker/fighting rather than four
 * independent fades.
 *
 * FadeToPaletteAnimationPerJar fixes this by baking brightness directly into
 * each pixel's color (via nscale8_video()) instead of touching the global
 * FastLED.setBrightness(). Set the overall ceiling brightness ONCE in
 * setup() with FastLED.setBrightness(BRIGHTNESS), and leave it alone --
 * each per-jar instance's own brightness ramp then scales its jar's pixels
 * relative to that ceiling, fully independently of the other jars.
 *
 * Class name: FadeToPaletteAnimationPerJar
 * Construct one instance per jar, pointed at that jar's own contiguous
 * slice of the shared leds[] array:
 *
 *   FadeToPaletteAnimationPerJar fadeJar[NUM_JARS] = {
 *     FadeToPaletteAnimationPerJar(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, BRIGHTNESS),
 *     FadeToPaletteAnimationPerJar(&leds[1 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, BRIGHTNESS),
 *     FadeToPaletteAnimationPerJar(&leds[2 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, BRIGHTNESS),
 *     FadeToPaletteAnimationPerJar(&leds[3 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, BRIGHTNESS)
 *   };
 *
 * Because each jar is a contiguous 64-LED block in the shared leds[] array
 * (jar 0 = indices 0-63, jar 1 = 64-127, etc.), pointing each instance at
 * &leds[jar * 64] with count 64 is all that's needed -- no matrix/row/col
 * addressing required for a solid-color fill.
 *
 * ── USAGE IN Performance() ───────────────────────────────────────────────────
 *   BETWEEN(START(0,0,0), TO(0,0,10)) {
 *     fadeJar[0].Update(CRGBPalette16(CRGB::Red),   6, 20, 255, 100, 150); // jar 0: red, boosted brighter
 *     fadeJar[1].Update(CRGBPalette16(CRGB::Blue),  6, 20, 255, 100, 150); // jar 1: blue, boosted brighter
 *     fadeJar[2].Update(CRGBPalette16(CRGB::Green), 6, 20, 255, 100, 150); // jar 2: green, boosted brighter
 *     fadeJar[3].Update(CRGBPalette16(CRGB::Yellow),6, 20, 255, 100, 150); // jar 3: yellow, boosted brighter
 *   }
 *
 * ── KEY METHODS (identical to FadeToPaletteAnimation) ─────────────────────────
 *   fadeJar[i].Update(palette, colorBlendAmount=24, updateIntervalMs=20,
 *                     brightnessFadeAmount=8, colorStepIntervalMs=0, targetBrightness=BRIGHTNESS)
 *                                                     targetBrightness (0-255, optional, last param) is this
 *                                                     jar's own brightness ceiling for THIS call -- e.g. pure
 *                                                     red/green/blue/yellow don't look equally bright to the
 *                                                     eye at the same numeric value, so different jars/colors
 *                                                     often want different targets. Omit it to keep the
 *                                                     previous/default value (BRIGHTNESS).
 *   fadeJar[i].Update()
 *   fadeJar[i].setTargetPalette(palette)
 *   fadeJar[i].setColorBlendAmount(1-255)
 *   fadeJar[i].setUpdateInterval(ms)
 *   fadeJar[i].setColorStepInterval(ms)
 *   fadeJar[i].setBrightness(0-255)            -- only needed OUTSIDE a Performance()/Time_performance.h
 *                                                  BETWEEN block; inside one, prefer the targetBrightness
 *                                                  argument on Update() itself instead, so every value that
 *                                                  varies per call stays in that one call, per this sketch's
 *                                                  single-call-per-effect convention (see basic_functions.h)
 *   fadeJar[i].setBrightnessFadeAmount(1-255)
 *   fadeJar[i].reset(startColor)              -- call once per jar from RestartPerformance()
 */

class FadeToPaletteAnimationPerJar {
  private:
    CRGB*         leds;
    CRGB          currentColor;
    CRGB          targetColor;
    uint16_t      numLeds;
    uint8_t       targetBrightness;
    uint8_t       currentBrightness;
    uint8_t       brightnessFadeAmount;
    unsigned long lastUpdate;
    uint16_t      updateInterval;
    unsigned long lastColorStep;
    uint16_t      colorStepInterval;
    uint8_t       colorBlendAmount;

    void FillLEDsFromCurrentColor() {
      // Bake brightness into the color directly (nscale8_video) instead of
      // calling the global FastLED.setBrightness() -- this is what lets
      // multiple jars ramp brightness independently without fighting.
      CRGB displayColor = currentColor;
      displayColor.nscale8_video(currentBrightness);
      for (uint16_t i = 0; i < numLeds; i++)
        leds[i] = displayColor;
    }

  public:
    FadeToPaletteAnimationPerJar(CRGB* ledArray, uint16_t numLeds, uint8_t brightness = 150) {
      this->leds                 = ledArray;
      this->numLeds              = numLeds;
      this->targetBrightness     = brightness;
      this->currentBrightness    = 0;
      this->brightnessFadeAmount = 8;
      this->currentColor         = CRGB::Black;
      this->targetColor          = CRGB::Red;
      this->lastUpdate           = 0;
      this->updateInterval       = 20;
      this->lastColorStep        = 0;
      this->colorStepInterval    = 0;
      this->colorBlendAmount     = 24;
    }

    void Update(CRGBPalette16 palette,
                uint8_t  colorBlendAmountParam      = 24,
                uint16_t updateIntervalParam        = 20,
                uint8_t  brightnessFadeAmountParam  = 8,
                uint16_t colorStepIntervalParam     = 0,
                uint8_t  targetBrightnessParam      = BRIGHTNESS) {
      targetColor           = ColorFromPalette(palette, 0, 255, NOBLEND);
      colorBlendAmount      = colorBlendAmountParam;
      updateInterval        = updateIntervalParam;
      brightnessFadeAmount  = brightnessFadeAmountParam;
      colorStepInterval     = colorStepIntervalParam;
      targetBrightness      = targetBrightnessParam;
      Update();
    }

    void Update() {
      unsigned long currentMillis = millis();
      if (currentMillis - lastUpdate >= updateInterval) {
        lastUpdate = currentMillis;

        uint16_t effectiveColorInterval = (colorStepInterval > 0) ? colorStepInterval : updateInterval;
        if (currentMillis - lastColorStep >= effectiveColorInterval) {
          lastColorStep = currentMillis;
          currentColor = blend(currentColor, targetColor, colorBlendAmount);
        }

        if (currentBrightness < targetBrightness) {
          currentBrightness = qadd8(currentBrightness, brightnessFadeAmount);
          if (currentBrightness > targetBrightness) currentBrightness = targetBrightness;
        } else if (currentBrightness > targetBrightness) {
          currentBrightness = qsub8(currentBrightness, brightnessFadeAmount);
          if (currentBrightness < targetBrightness) currentBrightness = targetBrightness;
        }
        // NOTE: no FastLED.setBrightness() call here -- see file header.

        FillLEDsFromCurrentColor();
        FastLED.show();
      }
    }

    void setTargetPalette(CRGBPalette16 palette)  { targetColor          = ColorFromPalette(palette, 0, 255, NOBLEND); }
    void setColorBlendAmount(uint8_t amount)      { colorBlendAmount     = amount; }
    void setUpdateInterval(uint16_t interval)     { updateInterval       = interval; }
    void setColorStepInterval(uint16_t interval)  { colorStepInterval    = interval; }
    void setBrightness(uint8_t newBrightness)     { targetBrightness     = newBrightness; }
    void setBrightnessFadeAmount(uint8_t amount)  { brightnessFadeAmount = amount; }
    CRGBPalette16 getCurrentPalette()             { return CRGBPalette16(currentColor); }
    CRGBPalette16 getTargetPalette()              { return CRGBPalette16(targetColor); }

    /**
     * reset(startColor)
     * Snap this jar instantly to startColor (defaults to black), brightness 0.
     * Call once per jar from RestartPerformance().
     */
    void reset(CRGB startColor = CRGB::Black) {
      currentColor      = startColor;
      targetColor       = startColor;
      currentBrightness = 0;
      lastColorStep      = 0;
      CRGB displayColor  = startColor;
      displayColor.nscale8_video(0); // brightness 0 -> black regardless of startColor
      for (uint16_t i = 0; i < numLeds; i++)
        leds[i] = displayColor;
      FastLED.show();
    }
};

// ---------- Effect instances (independent per jar) ----------
// `leds` is defined in glass_jar_4jars.cpp. Each jar is a contiguous
// MATRIX_ROWS*MATRIX_COLS-LED block within the shared leds[] array, so
// &leds[jar * MATRIX_ROWS * MATRIX_COLS] points straight at that jar's
// own LEDs.
extern CRGB leds[NUM_LEDS];

FadeToPaletteAnimationPerJar fadeJar[NUM_JARS] = {
  FadeToPaletteAnimationPerJar(&leds[0 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, BRIGHTNESS),
  FadeToPaletteAnimationPerJar(&leds[1 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, BRIGHTNESS),
  FadeToPaletteAnimationPerJar(&leds[2 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, BRIGHTNESS),
  FadeToPaletteAnimationPerJar(&leds[3 * MATRIX_ROWS * MATRIX_COLS], MATRIX_ROWS * MATRIX_COLS, BRIGHTNESS)
};
