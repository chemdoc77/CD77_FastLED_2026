#pragma once
#include <FastLED.h>
#include "MatrixConfig.h" // for NUM_LEDS, BRIGHTNESS

/*
 * CD77_slowchangepalette.h
 *
 * Non-blocking smooth colour crossfade class.
 * Based on Mark Kriegsman's PaletteCrossfade.ino
 *
 * Class name: FadeToPaletteAnimation
 * Instance:   FadetoPaletteAnimation   (declared in CD77_pussywillows_both.ino)
 *
 * NOTE: internally this blends a single solid CRGB colour, not a moving multi-
 * colour palette gradient. Colours are blended using FastLED's blend(), which
 * interpolates R, G, and B together in lockstep — this avoids the transient
 * off-colours (including flashes of black) that happen if each channel is
 * blended independently/sequentially.
 *
 * BRIGHTNESS: baked directly into each pixel's color via nscale8_video(),
 * same approach as FadeToPaletteAnimationPerJar (CD77_slowchangepalette_PerJar.h)
 * -- this class does NOT call the global FastLED.setBrightness(). It used to,
 * but that caused a double-darkening bug: this class would leave the global
 * brightness scale sitting at whatever it last used, which then got applied a
 * SECOND time on top of fadeJar[]'s own per-jar baked-in brightness the next
 * time THAT class ran later in the same performance timeline -- silently
 * darkening fadeJar[]'s output to roughly targetBrightness² (out of 255)
 * instead of targetBrightness, with fadeJar[] itself unchanged. Set the
 * overall ceiling brightness ONCE in setup() with FastLED.setBrightness(BRIGHTNESS),
 * and leave it alone from then on -- every brightness-capable effect in this
 * sketch (this class included) now scales its own pixels relative to that
 * fixed ceiling instead of touching it.
 *
 * ── USAGE IN Performance() ───────────────────────────────────────────────────
 *   // Single-line call — sets colour, animates, calls FastLED.show() internally:
 *   BETWEEN(START(0,0,0), TO(0,0,10)) { FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red)); }
 *
 *   // Optional tuning params right in the call — each BETWEEN block can use different values:
 *   //   Update(palette, colorBlendAmount, updateIntervalMs, brightnessFadeAmount, colorStepIntervalMs, targetBrightness)
 *   BETWEEN(START(0,0,0), TO(0,0,10)) { FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red), 6, 20, 255, 150, 150); }
 *
 * ── KEY METHODS ───────────────────────────────────────────────────────────────
 *   FadetoPaletteAnimation.Update(palette, colorBlendAmount=24, updateIntervalMs=20,
 *                                 brightnessFadeAmount=8, colorStepIntervalMs=0, targetBrightness=BRIGHTNESS)
 *                                                     set target + tune smoothness/ramps + animate + FastLED.show().
 *                                                     targetBrightness (0-255, optional, last param) is this
 *                                                     call's brightness ceiling -- omit it to keep the
 *                                                     previous/default value (BRIGHTNESS)
 *   FadetoPaletteAnimation.Update()                  animate toward current target + FastLED.show()
 *   FadetoPaletteAnimation.setTargetPalette(palette) set target without animating
 *   FadetoPaletteAnimation.setColorBlendAmount(1-255) how far current colour moves toward target per colour
 *                                                      step (default 24); lower = smoother/slower
 *   FadetoPaletteAnimation.setUpdateInterval(ms)     DISPLAY refresh rate ms (default 20); keep 20-30 for smoothness
 *   FadetoPaletteAnimation.setColorStepInterval(ms)  how often colour actually blends (default 0 = every tick);
 *                                                     raise this (e.g. 150-500) to slow the colour transition
 *   FadetoPaletteAnimation.setBrightness(0-255)      target brightness to ramp toward
 *   FadetoPaletteAnimation.setBrightnessFadeAmount(1-255)  brightness ramp step size (default 8; lower = slower
 *                                                     ramp; 255 = snap to target brightness instantly)
 *   FadetoPaletteAnimation.reset()                   snap to black, brightness 0; call from RestartPerformance()
 */

class FadeToPaletteAnimation {
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
      // Bake brightness into the color directly (nscale8_video), same
      // approach as FadeToPaletteAnimationPerJar, instead of calling the
      // global FastLED.setBrightness(). See "Why this bakes brightness
      // in" note below Update() for why this matters even though only
      // one instance of THIS class ever runs at a time.
      CRGB displayColor = currentColor;
      displayColor.nscale8_video(currentBrightness);
      for (uint16_t i = 0; i < numLeds; i++)
        leds[i] = displayColor;
    }

  public:
    FadeToPaletteAnimation(CRGB* ledArray, uint16_t numLeds, uint8_t brightness = 150) {
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
      this->colorStepInterval    = 0;   // 0 = advance colour every tick (same as updateInterval)
      this->colorBlendAmount     = 24;
    }

    /**
     * Update(palette, colorBlendAmount, updateIntervalMs, brightnessFadeAmount, colorStepIntervalMs, targetBrightness)
     * Set the target colour (taken from the palette), run one animation frame, call FastLED.show().
     * This is the preferred single-line call for use in Performance(). Every parameter after
     * `palette` is optional and independent per call — each BETWEEN block can use its own values.
     *
     *   colorBlendAmount     1-255  how far current colour moves toward target per colour step (default 24)
     *                                try 4-16 for a slow, smooth crossfade — R/G/B always move together,
     *                                so there is no black flash or channel-lag stepping
     *   updateIntervalMs     ms     how often the DISPLAY refreshes / brightness ramps (default 20 = 50 fps)
     *                                keep this LOW (20-30) for smoothness — this is not a slow-down knob
     *   brightnessFadeAmount 1-255  how much brightness moves toward target per tick (default 8)
     *                                try 1-4 for a slow ramp up; 255 snaps to target brightness instantly
     *   colorStepIntervalMs  ms     how often the colour actually blends a step (default 0 = every tick,
     *                                same as updateIntervalMs). Raise this (e.g. 150-500) to slow the
     *                                colour transition further while the display still refreshes smoothly.
     *   targetBrightness     0-255  this call's brightness ceiling (default BRIGHTNESS). Pure red/green/
     *                                blue/yellow don't look equally bright to the eye at the same numeric
     *                                value, so different calls (e.g. different jars/colors in the same
     *                                BETWEEN block) often want different values here to look visually
     *                                matched -- see the fadeJar[] example in CD77_slowchangepalette_PerJar.h
     *
     * Example — smooth, slow colour fade + brightness snapped instantly:
     *   FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red), 6, 20, 255, 150);
     *
     * Example — same, but with a one-off brightness ceiling for this call
     * (trailing optional param, default BRIGHTNESS):
     *   FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red), 6, 20, 255, 150, 150);
     */
    void Update(CRGBPalette16 palette,
                uint8_t  colorBlendAmountParam      = 24,
                uint16_t updateIntervalParam        = 20,
                uint8_t  brightnessFadeAmountParam  = 8,
                uint16_t colorStepIntervalParam     = 0,
                uint8_t  targetBrightnessParam      = BRIGHTNESS) {
      // This class works with a single solid colour rather than a moving multi-stop
      // gradient, so pull one representative colour out of the palette.
      targetColor           = ColorFromPalette(palette, 0, 255, NOBLEND);
      colorBlendAmount      = colorBlendAmountParam;
      updateInterval         = updateIntervalParam;
      brightnessFadeAmount   = brightnessFadeAmountParam;
      colorStepInterval      = colorStepIntervalParam;
      targetBrightness       = targetBrightnessParam;
      Update();
    }

    /**
     * Update()
     * Run one animation frame toward the current target colour.
     * Calls FastLED.show() internally.
     */
    void Update() {
      unsigned long currentMillis = millis();
      if (currentMillis - lastUpdate >= updateInterval) {
        lastUpdate = currentMillis;

        // The colour blend only advances every colorStepInterval ms (0 = every tick,
        // i.e. same rate as the display refresh). This lets the display/brightness
        // stay smooth and fast while the actual colour transition crawls as slowly
        // as you like — raising updateInterval instead would make things choppy.
        uint16_t effectiveColorInterval = (colorStepInterval > 0) ? colorStepInterval : updateInterval;
        if (currentMillis - lastColorStep >= effectiveColorInterval) {
          lastColorStep = currentMillis;
          // blend() moves R, G, and B together in one weighted step — no channel
          // ever "gets ahead" of the others, so no black flash or visible stepping.
          currentColor = blend(currentColor, targetColor, colorBlendAmount);
        }

        // Ramp actual LED brightness toward targetBrightness, one step per tick.
        // Smaller brightnessFadeAmount = slower ramp up (or down); 255 = instant snap.
        if (currentBrightness < targetBrightness) {
          currentBrightness = qadd8(currentBrightness, brightnessFadeAmount);
          if (currentBrightness > targetBrightness) currentBrightness = targetBrightness;
        } else if (currentBrightness > targetBrightness) {
          currentBrightness = qsub8(currentBrightness, brightnessFadeAmount);
          if (currentBrightness < targetBrightness) currentBrightness = targetBrightness;
        }
        // NOTE: no FastLED.setBrightness() call here (removed) -- brightness
        // is now baked directly into the pixel color in
        // FillLEDsFromCurrentColor() instead. Why this matters even though
        // only one instance of THIS class ever runs at a time: this sketch
        // also uses FadeToPaletteAnimationPerJar (fadeJar[]) elsewhere in
        // the same performance timeline, and THAT class was specifically
        // designed to never touch the global FastLED.setBrightness() (see
        // CD77_slowchangepalette_PerJar.h's file header) -- it assumes the
        // global brightness is set once in setup() and left alone. If THIS
        // class kept calling FastLED.setBrightness() here, it would leave
        // the global scale sitting at whatever value this class last used
        // it for, which then gets applied a SECOND time on top of fadeJar[]'s
        // own baked-in per-jar brightness the next time fadeJar[] runs --
        // silently darkening fadeJar[]'s output far more than intended
        // (roughly targetBrightness² instead of targetBrightness, out of
        // 255) without fadeJar[] itself having changed at all.

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
     * Snap instantly to startColor (defaults to black if omitted). Call from RestartPerformance().
     * Brightness still ramps up from 0 as normal — only the starting colour changes.
     */
    void reset(CRGB startColor = CRGB::Black) {
      currentColor       = startColor;
      targetColor        = startColor;
      currentBrightness  = 0;
      lastColorStep       = 0;
      // Snap the strip to black by baking brightness 0 into the pixel
      // color directly (nscale8_video), instead of the old
      // FastLED.setBrightness(0) -- this class no longer touches the
      // global brightness scale at all (see Update()'s comment above
      // for why). RestartPerformance() still explicitly restores
      // FastLED.setBrightness(BRIGHTNESS) once, as a safety-net ceiling
      // for the whole strip, set once and left alone.
      CRGB displayColor = startColor;
      displayColor.nscale8_video(0); // brightness 0 -> black regardless of startColor
      for (uint16_t i = 0; i < numLeds; i++)
        leds[i] = displayColor;
      FastLED.show();
    }
};

// ---------- Effect instance (mirrored across all 4 jars) ----------
// Fades all 4 jars together as one solid color across the whole
// 256-LED strip. `leds` is defined in glass_jar_4jars.cpp.
extern CRGB leds[NUM_LEDS];

FadeToPaletteAnimation FadetoPaletteAnimation(leds, NUM_LEDS, BRIGHTNESS);
