#pragma once
#include <FastLED.h>

/*
 * CD77_slowchangepalette.h
 *
 * Non-blocking SMOOTH colour crossfade class.
 * Interpolates linearly, in time, from wherever the LEDs currently are
 * to a target colour — framerate-independent, so it always completes a
 * full, buttery-smooth fade over `fadeDurationMs`, regardless of how
 * often Update() gets called or how far apart the start/end colours are.
 *
 * (Earlier version used FastLED's nblendPaletteTowardPalette(), which only
 *  nudges each colour channel by 1 unit per call on a fixed 100ms timer —
 *  too slow and not tied to your BETWEEN() window length, so long fades
 *  looked stepwise/incomplete rather than smooth.)
 *
 * Class name: FadeToPaletteAnimation
 * Instance:   FadetoPaletteAnimation   (declared in the .ino/.cpp)
 *
 * ── USAGE IN Performance() ───────────────────────────────────────────────────
 *   // Single-line call — sets target colour, animates, calls FastLED.show() internally:
 *   BETWEEN(0,0,00.000,  0,0,10.000) { FadetoPaletteAnimation.Update(CRGBPalette16(CRGB::Red), 1500, 20); }
 *   BETWEEN(0,0,10.000,  0,0,20.000) { FadetoPaletteAnimation.Update(OceanColors_p, 1500, 20); }
 *
 * ── KEY METHODS ───────────────────────────────────────────────────────────────
 *   FadetoPaletteAnimation.Update(palette)          set target + animate + FastLED.show()
 *   FadetoPaletteAnimation.Update()                 animate toward current target + FastLED.show()
 *   FadetoPaletteAnimation.setTargetPalette(palette) set target, restarting the fade from the current colour
 *   FadetoPaletteAnimation.setFadeDuration(ms)      how long a full crossfade takes (default DEFAULT_FADE_DURATION_MS = 1500ms)
 *   FadetoPaletteAnimation.setUpdateInterval(ms)    frame interval (default DEFAULT_UPDATE_INTERVAL_MS = 20ms = ~50 FPS)
 *   FadetoPaletteAnimation.setBrightness(0-255)     brightness
 *   FadetoPaletteAnimation.reset()                  snap to black (paints + show()); for manual use
 *   FadetoPaletteAnimation.forceInactive()          state-only reset (no paint, no show()); used by ResetAllEffects()
 *                                                   from RestartPerformance()
 *
 *   DEFAULT_FADE_DURATION_MS and DEFAULT_UPDATE_INTERVAL_MS (both public static
 *   constants on the class) are the single source of truth for these defaults —
 *   they set the instance's initial state in the constructor AND are used as
 *   Update()'s own default arguments, so the two can never drift out of sync.
 *   Change them in one place (near the top of the class) if you want different
 *   defaults sketch-wide.
 *
 *   A new palette given mid-fade smoothly re-targets from whatever colour is
 *   currently showing — no pop, no restart-from-black.
 */

class FadeToPaletteAnimation {
  public:
    // Defaults for a full crossfade — defined once here, used both to
    // initialise a new instance and as Update()'s default arguments, so
    // there's a single source of truth instead of the same numbers
    // hardcoded in two separate places.
    static const uint16_t DEFAULT_FADE_DURATION_MS  = 1500;  // 1.5s default full-fade time
    static const uint16_t DEFAULT_UPDATE_INTERVAL_MS = 20;   // ~50 FPS

  private:
    CRGB*         leds;
    uint16_t      numLeds;
    uint8_t       brightness;

    CRGB          currentColor;    // colour actually showing right now
    CRGB          fadeStartColor;  // colour the current fade started from
    CRGB          targetColor;     // colour the current fade is headed to
    unsigned long fadeStart;       // millis() when the current fade began
    uint16_t      fadeDurationMs;  // how long a full crossfade takes
    unsigned long lastUpdate;
    uint16_t      updateInterval;  // frame interval (ms)

    // A palette's "colour" for this class is its slot-0 entry — every
    // CRGBPalette16 passed in here (CRGBPalette16(CRGB::Red) etc.) is a
    // flat, single-colour palette, so slot 0 is exactly that colour.
    static CRGB PaletteToColor(const CRGBPalette16& palette) {
      return ColorFromPalette(palette, 0, 255, LINEARBLEND);
    }

    void RetargetTo(CRGB newTarget) {
      if (newTarget == targetColor) return;   // already heading there
      fadeStartColor = currentColor;          // smoothly continue from here
      targetColor    = newTarget;
      fadeStart       = millis();
    }

  public:
    FadeToPaletteAnimation(CRGB* ledArray, uint16_t numLeds, uint8_t brightness = 150) {
      this->leds           = ledArray;
      this->numLeds        = numLeds;
      this->brightness     = brightness;
      this->currentColor   = CRGB::Black;
      this->fadeStartColor = CRGB::Black;
      this->targetColor    = CRGB::Black;
      this->fadeStart       = 0;
      this->fadeDurationMs  = DEFAULT_FADE_DURATION_MS;
      this->lastUpdate      = 0;
      this->updateInterval  = DEFAULT_UPDATE_INTERVAL_MS;
    }

    /**
     * Update(palette, fadeDurationMs, updateInterval)
     * Set the target colour (if it's new), run one animation frame,
     * call FastLED.show(). Preferred single-line call for Performance().
     *   fadeDurationMs – how long a full crossfade takes (default DEFAULT_FADE_DURATION_MS)
     *   updateInterval – frame interval in ms (default DEFAULT_UPDATE_INTERVAL_MS)
     */
    void Update(CRGBPalette16 palette, uint16_t fadeDurationMs = DEFAULT_FADE_DURATION_MS, uint16_t updateInterval = DEFAULT_UPDATE_INTERVAL_MS) {
      this->fadeDurationMs = fadeDurationMs;
      this->updateInterval = updateInterval;
      RetargetTo(PaletteToColor(palette));
      Update();
    }

    /**
     * Update()
     * Run one animation frame toward the current target colour.
     * Calls FastLED.show() internally. Framerate-independent: however
     * often this is called, the fade always completes in fadeDurationMs.
     */
    void Update() {
      unsigned long now = millis();
      if (now - lastUpdate < updateInterval) return;
      lastUpdate = now;

      unsigned long elapsed = now - fadeStart;
      uint8_t frac = (elapsed >= fadeDurationMs)
                       ? 255
                       : (uint8_t)((elapsed * 255UL) / fadeDurationMs);

      currentColor = blend(fadeStartColor, targetColor, frac);

      for (uint16_t i = 0; i < numLeds; i++)
        leds[i] = currentColor;
      FastLED.show();
    }

    void setTargetPalette(CRGBPalette16 palette)  { RetargetTo(PaletteToColor(palette)); }
    void setFadeDuration(uint16_t ms)             { fadeDurationMs = ms; }
    void setUpdateInterval(uint16_t interval)     { updateInterval = interval; }
    void setBrightness(uint8_t newBrightness)     { brightness     = newBrightness; }
    CRGB getCurrentColor()                        { return currentColor; }
    CRGB getTargetColor()                         { return targetColor; }

    /**
     * reset()
     * Snap instantly to black. Call from RestartPerformance().
     */
    void reset() {
      currentColor   = CRGB::Black;
      fadeStartColor = CRGB::Black;
      targetColor    = CRGB::Black;
      fadeStart       = millis();
      for (uint16_t i = 0; i < numLeds; i++)
        leds[i] = CRGB::Black;
      FastLED.show();
    }

    /**
     * forceInactive()
     * State-only reset: puts the fade back to "black, nothing in progress"
     * WITHOUT touching the LED buffer or calling FastLED.show(). Use this
     * (instead of reset()) from RestartPerformance()/ResetAllEffects(),
     * where other resets and the next effect's first paint follow
     * immediately — stacking extra show() calls there can make the next
     * real paint fail to latch on this hardware. Same idea as the
     * forceInactive() / ForceInactiveAll() functions in LED_Effects.h.
     */
    void forceInactive() {
      currentColor   = CRGB::Black;
      fadeStartColor = CRGB::Black;
      targetColor    = CRGB::Black;
      fadeStart       = millis();
      lastUpdate      = 0;
      fadeDurationMs  = DEFAULT_FADE_DURATION_MS;
      updateInterval  = DEFAULT_UPDATE_INTERVAL_MS;
    }
};
