#pragma once
#include <FastLED.h>

/*
 * CD77_slowchangepalette.h
 *
 * Non-blocking smooth palette transition class.
 * Based on Mark Kriegsman's PaletteCrossfade.ino
 *
 * Class name: FadeToPaletteAnimation
 * Instance:   FadetoPaletteAnimation   (declared in CD77_pussywillows_both.ino)
 *
 * ── USAGE IN Performance() ───────────────────────────────────────────────────
 *   // Single-line call — sets palette, animates, calls FastLED.show() internally:
 *   BETWEEN(START(0,0,0), TO(0,0,10)) { FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red)); }
 *   BETWEEN(START(0,0,10),TO(0,0,20)) { FadetoPaletteAnimation.Update(OceanColors_p); }
 *
 * ── KEY METHODS ───────────────────────────────────────────────────────────────
 *   FadetoPaletteAnimation.Update(palette)           set target + animate + FastLED.show()
 *   FadetoPaletteAnimation.Update()                  animate toward current target + FastLED.show()
 *   FadetoPaletteAnimation.setTargetPalette(palette) set target without animating
 *   FadetoPaletteAnimation.setMaxChanges(1-48)       blend speed (default 48 = fast)
 *   FadetoPaletteAnimation.setUpdateInterval(ms)     refresh rate ms (default 100 = 10 FPS)
 *   FadetoPaletteAnimation.setMotionSpeed(speed)     colour index speed (default 2)
 *   FadetoPaletteAnimation.setBrightness(0-255)      brightness
 *   FadetoPaletteAnimation.reset()                   snap to black; call from RestartPerformance()
 */

class FadeToPaletteAnimation {
  private:
    CRGB*         leds;
    CRGBPalette16 currentPalette;
    CRGBPalette16 targetPalette;
    uint16_t      numLeds;
    uint8_t       brightness;
    unsigned long lastUpdate;
    uint16_t      updateInterval;
    uint8_t       maxChanges;
    uint8_t       startIndex;
    uint8_t       motionSpeed;

    void FillLEDsFromPaletteColors() {
      CRGB color = ColorFromPalette(currentPalette, 1, 255, LINEARBLEND);
      for (uint16_t i = 0; i < numLeds; i++)
        leds[i] = color;
    }

  public:
    FadeToPaletteAnimation(CRGB* ledArray, uint16_t numLeds, uint8_t brightness = 150) {
      this->leds           = ledArray;
      this->numLeds        = numLeds;
      this->brightness     = brightness;
      this->currentPalette = CRGBPalette16(CRGB::Black);
      this->targetPalette  = CRGBPalette16(CRGB::Red);
      this->lastUpdate     = 0;
      this->updateInterval = 100;
      this->maxChanges     = 48;
      this->startIndex     = 0;
      this->motionSpeed    = 2;
    }

    /**
     * Update(palette)
     * Set the target palette, run one animation frame, call FastLED.show().
     * This is the preferred single-line call for use in Performance().
     */
    void Update(CRGBPalette16 palette) {
      targetPalette = palette;
      Update();
    }

    /**
     * Update()
     * Run one animation frame toward the current target palette.
     * Calls FastLED.show() internally.
     */
    void Update() {
      unsigned long currentMillis = millis();
      if (currentMillis - lastUpdate >= updateInterval) {
        lastUpdate = currentMillis;
        nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
        startIndex = startIndex + motionSpeed;
        FillLEDsFromPaletteColors();
        FastLED.show();
      }
    }

    void setTargetPalette(CRGBPalette16 palette) { targetPalette  = palette; }
    void setMaxChanges(uint8_t changes)           { maxChanges     = changes; }
    void setUpdateInterval(uint16_t interval)     { updateInterval = interval; }
    void setMotionSpeed(uint8_t speed)            { motionSpeed    = speed; }
    void setBrightness(uint8_t newBrightness)     { brightness     = newBrightness; }
    CRGBPalette16 getCurrentPalette()             { return currentPalette; }
    CRGBPalette16 getTargetPalette()              { return targetPalette; }

    /**
     * reset()
     * Snap instantly to black. Call from RestartPerformance().
     */
    void reset() {
      currentPalette = CRGBPalette16(CRGB::Black);
      targetPalette  = CRGBPalette16(CRGB::Black);
      for (uint16_t i = 0; i < numLeds; i++)
        leds[i] = CRGB::Black;
      FastLED.show();
    }
};
