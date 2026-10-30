#ifndef LED_EFFECTS_H
#define LED_EFFECTS_H

// ============================================================
//  LED_Effects.h
//  State-machine LED effects for WS2812b strips
//  Target MCU : Lolin D32  (ESP32)
//  LED library : FastLED
//
//  All timing uses millis() — zero blocking delays.
//  All effect parameters including the instance number are
//  passed directly in each Update() call in loop().
//
//  Usage in loop():
//    SolidFill::Update    (leds, 1, {CRGB::Red,   0,  24, 500, true}); // repaints every 500ms, always red, no blink
//    ColorWipeFill::Update(leds, 1, {CRGB::Blue,  24, 48,  20, true});
//    ColorWipeDot::Update (leds, 1, {CRGB::Green, 48, 72,  20, false});
//    Blink::Update        (leds, 1, {CRGB::Yellow, 0, 128, 300, true}); // blinks on/off every 300ms (forward is ignored — any value)
//
//  SolidFill::ForceInactive(instance), ColorWipeFill::ForceInactive(instance),
//  ColorWipeDot::ForceInactive(instance), and Blink::ForceInactive(instance)
//  each reset just that instance's internal "already started/active" state
//  (no LED paint, no FastLED.show()) so the next Update() call treats it as
//  a fresh start. Each class also has a ForceInactiveAll() — e.g.
//  ColorWipeFill::ForceInactiveAll() — that does the same thing for EVERY
//  instance (1..its _COUNT) in one call; that is what RestartPerformance()
//  uses, so a new instance number used in a future Performance() function
//  is automatically covered with no extra line needed there. Prefer these
//  over each effect's Reset() when calling them back-to-back with other
//  resets that already call show() — e.g. from RestartPerformance() —
//  since stacking several show() calls in a row can cause the very next
//  real paint to fail to latch on some hardware.
//
//  The second argument is the instance number (1-based).
//  The last field in LEDParams is forward: true = fromLed to toLed,
//                                         false = toLed to fromLed.
//  To allow more instances, increase the count defines below.
//
//  No variable name begins with '_'.
// ============================================================

#include <FastLED.h>

// ============================================================
//  LEDParams
//  Holds all parameters for one Update() call.
// ============================================================
struct LEDParams
{
    CRGB     color;      // effect color
    uint16_t fromLed;    // first LED index (inclusive)
    uint16_t toLed;      // last  LED index (exclusive)
    uint32_t waitMs;     // ms between steps or toggles (0 = always on for SolidFill)
    bool     forward;    // true = animate fromLed to toLed, false = toLed to fromLed
};

// ============================================================
//  Instance counts — increase these if you need more.
// ============================================================
#define SOLID_FILL_COUNT      4
#define COLOR_WIPE_FILL_COUNT 4
#define COLOR_WIPE_DOT_COUNT  4
#define BLINK_COUNT           4

// A Blink instance that has not been called for this long (ms) is treated as
// stopped: its window ended. The next time Blink::Update() is called, any
// stopped instances are cleared, and the instances that start next begin
// together on a fresh shared clock (ON first). A normal loop() pass is a few
// ms, so 100 only triggers when a Blink really was skipped (e.g. after a
// blackout window). Raise it only if a single loop() pass can take longer.
#define BLINK_RESTART_GAP_MS  100

// Same idea for ColorWipeFill and ColorWipeDot. They share ONE clock, so wipes
// and dots that are started together also stay in step with each other.
#define WIPE_RESTART_GAP_MS   100


// ============================================================
//  SolidFill
//  Fills LEDs [p.fromLed .. p.toLed-1] with p.color and leaves them that
//  color — never toggles to black, never blinks. p.waitMs > 0 makes it
//  periodically repaint the SAME color every p.waitMs ms (a redundant
//  "keep-alive" re-show, not a toggle) as a safety net against an
//  occasional show() call failing to latch on this hardware. p.waitMs = 0
//  means paint once and never repaint.
//
//  Call in loop():
//    SolidFill::Update(leds, 0, {CRGB::Red, 0, 24, 500});   // repaints every 500ms, always red, no blink
// ============================================================
class SolidFillState
{
public:
    SolidFillState()
    {
        active     = false;
        lastUpdate = 0;
    }

    // Paints p.color across [fromLed, toLed) and keeps it that color —
    // never toggles to black, never blinks. On the first call (or any call
    // whose color/range differs from what's already showing) it paints and
    // calls FastLED.show() immediately. After that, if p.waitMs > 0, it
    // periodically REPAINTS THE SAME COLOR every p.waitMs ms — not a
    // toggle, just a redundant re-affirming show() call. This exists as a
    // self-healing safety net: on this hardware a single show() call can
    // occasionally fail to latch, and with no periodic repaint at all, a
    // failed first paint would leave the LEDs wrong for the rest of the
    // window with nothing to correct it. A nonzero waitMs means any single
    // failed latch gets caught and corrected within one waitMs interval,
    // same as how the old blink version's periodic repaints masked latch
    // failures — but without the visual on/off toggling that caused. Pass
    // waitMs=0 for "paint once, no periodic repaint" if you're confident a
    // single show() call is reliable enough for your case.
    void run(CRGB* ledsPtr, LEDParams p)
    {
        unsigned long now = millis();

        bool changed = !active ||
                       p.color   != lastParams.color   ||
                       p.fromLed != lastParams.fromLed ||
                       p.toLed   != lastParams.toLed;

        if (changed)
        {
            active     = true;
            lastParams = p;
            lastUpdate = now;
            fill_solid(ledsPtr + p.fromLed, p.toLed - p.fromLed, p.color);
            FastLED.show();
            return;
        }

        if (p.waitMs > 0 && now - lastUpdate >= p.waitMs)
        {
            lastUpdate = now;
            fill_solid(ledsPtr + p.fromLed, p.toLed - p.fromLed, p.color);
            FastLED.show();
        }
    }

    void reset(CRGB* ledsPtr, LEDParams p)
    {
        active = false;
        fill_solid(ledsPtr + p.fromLed, p.toLed - p.fromLed, CRGB::Black);
        FastLED.show();
    }

    // State-only reset: forces back to "not yet activated" so the next
    // run() call treats it as fresh, WITHOUT touching the LED buffer or
    // calling FastLED.show(). Use this (instead of reset()) when calling it
    // back-to-back with other resets that already call show() — e.g. from
    // RestartPerformance() — so this doesn't stack one more show() call
    // immediately before the effect's own next real paint, which on this
    // hardware can cause that paint to fail to latch.
    void forceInactive()
    {
        active = false;
    }

private:
    bool          active;
    LEDParams     lastParams;
    unsigned long lastUpdate;
};

SolidFillState solidFillInstances[SOLID_FILL_COUNT];

class SolidFill
{
public:
    static void Update(CRGB* ledsPtr, uint8_t instance, LEDParams p)
    {
        if (instance >= 1 && instance <= SOLID_FILL_COUNT)
            solidFillInstances[instance - 1].run(ledsPtr, p);
    }

    static void Reset(CRGB* ledsPtr, uint8_t instance, LEDParams p)
    {
        if (instance >= 1 && instance <= SOLID_FILL_COUNT)
            solidFillInstances[instance - 1].reset(ledsPtr, p);
    }

    // State-only reset — see SolidFillState::forceInactive() above.
    static void ForceInactive(uint8_t instance)
    {
        if (instance >= 1 && instance <= SOLID_FILL_COUNT)
            solidFillInstances[instance - 1].forceInactive();
    }

    // Same as ForceInactive(), but for EVERY instance (1..SOLID_FILL_COUNT)
    // at once. Call this one line from RestartPerformance() instead of a
    // separate ForceInactive(n) per instance number — it stays correct even
    // if you add more SolidFill instances to a Performance() later.
    static void ForceInactiveAll()
    {
        for (uint8_t i = 1; i <= SOLID_FILL_COUNT; i++)
            solidFillInstances[i - 1].forceInactive();
    }
};


// ============================================================
//  Blink
//  Blinks LEDs [p.fromLed .. p.toLed-1] ON/OFF with p.color, toggling
//  every p.waitMs milliseconds — ON first, then alternating. Non-blocking
//  state machine; call every loop(). p.forward is UNUSED — it's part of
//  the shared LEDParams struct (meaningful for ColorWipeFill/ColorWipeDot's
//  direction of travel) but Blink has no direction to animate, so pass any
//  value.
//
//  Call in loop():
//    Blink::Update(leds, 1, {CRGB::Red, 0, 128, 500, true});   // blinks the whole strip red, 500ms per phase
//
//  SYNCHRONIZED BLINKING: every Blink instance shares ONE start time
//  (blinkSyncStart) and toggles on a fixed grid of waitMs steps from it, so
//  several instances with the same waitMs blink in lockstep and never drift
//  apart, even though each one's Update() call comes one after another
//  (each Update() ends in a ~8 ms FastLED.show(), which used to push every
//  later instance a little further out of step).
//
//  Nothing extra is needed in a Performance() function: just call the
//  Blink::Update() lines inside the BETWEEN window. An instance that was not
//  called for BLINK_RESTART_GAP_MS or more (its earlier window ended) counts
//  as stopped, so when Blinks are used again they ALL start together, ON
//  first, on a fresh shared clock — a leftover instance never carries on
//  with its old phase. An instance that starts while others are already
//  blinking joins their grid in the matching phase. Blinks used in
//  back-to-back windows with no gap simply carry on.
// ============================================================

// Shared Blink clock: the time the current group of Blink instances started.
// Valid while at least one Blink instance is active; cleared when none is.
unsigned long blinkSyncStart = 0;
bool          blinkSyncValid = false;

class BlinkState
{
public:
    BlinkState()
    {
        ledState   = true;
        lastUpdate = 0;
        lastCall   = 0;
        active     = false;
    }

    void run(CRGB* ledsPtr, LEDParams p)
    {
        unsigned long now = millis();
        lastCall = now;

        if (!active)
        {
            // First instance of a group sets the shared start time; later
            // ones join it. Phase comes from the shared clock, so a
            // freshly restarted group is all ON first, and a late joiner
            // lands in the same phase as the instances already running.
            if (!blinkSyncValid)
            {
                blinkSyncStart = now;
                blinkSyncValid = true;
            }
            unsigned long steps = (p.waitMs > 0) ? (now - blinkSyncStart) / p.waitMs : 0;

            active     = true;
            ledState   = (steps % 2 == 0);
            lastUpdate = blinkSyncStart + steps * p.waitMs;
            fill_solid(ledsPtr + p.fromLed, p.toLed - p.fromLed,
                       ledState ? p.color : CRGB::Black);
            FastLED.show();
            return;
        }

        if (now - lastUpdate >= p.waitMs)
        {
            // Advance along the shared grid (not "now"), so every instance
            // toggles at the same pass and none drifts. If we have fallen
            // more than a step behind (e.g. waitMs was changed), resync to
            // now instead of toggling several times in a burst.
            lastUpdate += p.waitMs;
            if (now - lastUpdate >= p.waitMs) lastUpdate = now;

            ledState = !ledState;
            fill_solid(ledsPtr + p.fromLed, p.toLed - p.fromLed,
                       ledState ? p.color : CRGB::Black);
            FastLED.show();
        }
    }

    void reset(CRGB* ledsPtr, LEDParams p)
    {
        active   = false;
        ledState = true;
        fill_solid(ledsPtr + p.fromLed, p.toLed - p.fromLed, CRGB::Black);
        FastLED.show();
    }

    // State-only reset: forces back to "not yet activated" so the next
    // run() call treats it as fresh, WITHOUT touching the LED buffer or
    // calling FastLED.show(). Prefer this over reset() when calling it
    // back-to-back with other resets that already call show() — e.g. from
    // RestartPerformance() — same reasoning as SolidFillState::forceInactive().
    void forceInactive()
    {
        active   = false;
        ledState = true;
    }

    bool isActive() const
    {
        return active;
    }

    // Active, but not called for BLINK_RESTART_GAP_MS or more: its window ended.
    bool isStale(unsigned long now) const
    {
        return active && (now - lastCall) >= BLINK_RESTART_GAP_MS;
    }

private:
    bool          ledState;
    unsigned long lastUpdate;
    unsigned long lastCall;
    bool          active;
};

BlinkState blinkInstances[BLINK_COUNT];

class Blink
{
public:
    static void Update(CRGB* ledsPtr, uint8_t instance, LEDParams p)
    {
        if (instance >= 1 && instance <= BLINK_COUNT)
        {
            // Clear every instance whose window ended (not called lately) BEFORE
            // running this one, so a new group starts together on a fresh clock
            // no matter which instance is called first.
            ReleaseStale(millis());
            blinkInstances[instance - 1].run(ledsPtr, p);
        }
    }

    static void Reset(CRGB* ledsPtr, uint8_t instance, LEDParams p)
    {
        if (instance >= 1 && instance <= BLINK_COUNT)
        {
            blinkInstances[instance - 1].reset(ledsPtr, p);
            ReleaseSyncIfIdle();
        }
    }

    // State-only reset — see BlinkState::forceInactive() above.
    static void ForceInactive(uint8_t instance)
    {
        if (instance >= 1 && instance <= BLINK_COUNT)
        {
            blinkInstances[instance - 1].forceInactive();
            ReleaseSyncIfIdle();
        }
    }

    // Same as ForceInactive(), but for EVERY instance (1..BLINK_COUNT) at
    // once. See SolidFill::ForceInactiveAll() above. Call it when a BETWEEN
    // window that uses several Blink instances begins, so they all start
    // together on a fresh shared clock (ON first).
    static void ForceInactiveAll()
    {
        for (uint8_t i = 1; i <= BLINK_COUNT; i++)
            blinkInstances[i - 1].forceInactive();
        ReleaseSyncIfIdle();
    }

private:
    static void ReleaseStale(unsigned long now)
    {
        for (uint8_t i = 0; i < BLINK_COUNT; i++)
            if (blinkInstances[i].isStale(now))
                blinkInstances[i].forceInactive();
        ReleaseSyncIfIdle();
    }

    // Drops the shared start time once no instance is blinking, so the next
    // group starts a new one. Resetting just ONE instance while others keep
    // running leaves the shared clock alone: it rejoins in step with them.
    static void ReleaseSyncIfIdle()
    {
        for (uint8_t i = 0; i < BLINK_COUNT; i++)
            if (blinkInstances[i].isActive()) return;
        blinkSyncValid = false;
    }
};


// ============================================================
//  ColorWipeFill
//  Wipes p.color across LEDs [p.fromLed .. p.toLed-1] one LED
//  at a time, leaving each LED lit.  Clears and restarts when
//  done.  p.waitMs controls ms between each LED step.
//
//  SYNCHRONIZED WIPES: ColorWipeFill and ColorWipeDot instances share ONE
//  start time (wipeSyncStart) and step on a fixed grid of waitMs ticks from
//  it, so several wipes/dots with the same waitMs advance in lockstep and
//  never drift apart. Nothing extra is needed in a Performance() function:
//  just call the Update() lines inside the BETWEEN window. An instance not
//  called for WIPE_RESTART_GAP_MS or more counts as stopped, so when wipes
//  are used again they ALL start together at their first LED on a fresh
//  clock. Wipes over ranges of different lengths still finish and loop at
//  different times. An instance added while others are running starts at
//  its first LED but steps on the same grid.
//
//  Call in loop():
//    ColorWipeFill::Update(leds, 0, {CRGB::Blue, 24, 48, 20});
//
//  NOTE on restarting a performance: this effect only resets its
//  currentIndex back to fromLed when the CALL'S OWN PARAMETERS change
//  (isNewSegment, below). If a Performance() function calls
//  ColorWipeFill::Update() with the exact same {color, fromLed, toLed,
//  waitMs, forward} on every single cycle (very common — most
//  Performance() functions repeat the same choreography every loop),
//  isNewSegment never fires on cycle 2+, so the wipe silently continues
//  from wherever currentIndex was left instead of restarting at the first
//  LED. Call ColorWipeFill::ForceInactive(instance) from
//  RestartPerformance() to force a clean restart regardless of whether the
//  parameters changed — same pattern as SolidFill/Blink below.
// ============================================================

// Shared wipe clock for ColorWipeFill AND ColorWipeDot: the time the current
// group of wipes/dots started. Valid while at least one instance is active;
// cleared when none is.
unsigned long wipeSyncStart = 0;
bool          wipeSyncValid = false;

// Latest tick (start + k*waitMs) of the shared grid that is <= now. Every
// instance schedules its steps on this grid instead of on its own "now", so
// all of them step on the same ticks. waitMs 0 = step on every pass.
unsigned long WipeGridFloor(unsigned long now, uint32_t waitMs)
{
    return (waitMs > 0) ? wipeSyncStart + ((now - wipeSyncStart) / waitMs) * waitMs : now;
}

// Moves a step timer forward by exactly ONE tick of the grid, so no step is
// ever skipped and instances stay in step with each other. Only when it is
// 2+ ticks behind (a stall, e.g. waitMs changed) does it jump to the current
// tick instead of racing through all the missed steps.
void WipeAdvance(unsigned long& lastUpdate, unsigned long now, uint32_t waitMs)
{
    lastUpdate += waitMs;
    if (now - lastUpdate >= 2UL * waitMs)
        lastUpdate = WipeGridFloor(now, waitMs);
}

// Defined after both instance arrays exist (below ColorWipeDotState).
void WipeReleaseStale(unsigned long now);
void WipeReleaseSyncIfIdle();

class ColorWipeFillState
{
public:
    ColorWipeFillState()
    {
        currentIndex = 0;
        lastUpdate   = 0;
        lastCall     = 0;
        started      = false;
    }

    void run(CRGB* ledsPtr, LEDParams p)
    {
        unsigned long now = millis();
        lastCall = now;

        // Auto-reset: if the color, range, or direction changed since the
        // last call (or this is the very first call, or forceInactive()
        // was called), treat it as a brand new wipe segment — clear out
        // whatever this instance last drew and restart at fromLed. This
        // replaces having to call ColorWipeFill::Reset() by hand between
        // different-colored wipes that share the same instance number.
        bool isNewSegment = !started ||
                             p.color   != lastParams.color   ||
                             p.fromLed != lastParams.fromLed ||
                             p.toLed   != lastParams.toLed   ||
                             p.forward != lastParams.forward;

        if (isNewSegment)
        {
            if (started)
            {
                fill_solid(ledsPtr + lastParams.fromLed, lastParams.toLed - lastParams.fromLed, CRGB::Black);
                FastLED.show();
            }
            if (!wipeSyncValid)
            {
                wipeSyncStart = now;
                wipeSyncValid = true;
            }
            started      = true;
            currentIndex = p.forward ? p.fromLed : p.toLed - 1;
            lastUpdate   = WipeGridFloor(now, p.waitMs);
            lastParams   = p;
        }

        if (now - lastUpdate < p.waitMs) return;

        WipeAdvance(lastUpdate, now, p.waitMs);
        ledsPtr[currentIndex] = p.color;
        FastLED.show();

        if (p.forward)
        {
            currentIndex++;
            if (currentIndex >= p.toLed)
            {
                fill_solid(ledsPtr + p.fromLed, p.toLed - p.fromLed, CRGB::Black);
                FastLED.show();
                currentIndex = p.fromLed;
            }
        }
        else
        {
            if (currentIndex == p.fromLed)
            {
                fill_solid(ledsPtr + p.fromLed, p.toLed - p.fromLed, CRGB::Black);
                FastLED.show();
                currentIndex = p.toLed - 1;
            }
            else
            {
                currentIndex--;
            }
        }
    }

    void reset(CRGB* ledsPtr, LEDParams p)
    {
        started      = false;
        currentIndex = p.forward ? p.fromLed : p.toLed - 1;
        fill_solid(ledsPtr + p.fromLed, p.toLed - p.fromLed, CRGB::Black);
        FastLED.show();
    }

    // State-only reset: forces started back to false so the very next
    // run() call is treated as isNewSegment — restarting currentIndex at
    // fromLed/toLed-1 — WITHOUT touching the LED buffer or calling
    // FastLED.show(). Use this (instead of reset()) when calling it
    // back-to-back with other resets that already call show() — e.g. from
    // RestartPerformance() — same reasoning as
    // SolidFillState::forceInactive() / BlinkState::forceInactive(). This
    // is the fix for "doesn't start at the first LED on the second and
    // later rounds": without it, a Performance() function that calls
    // Update() with identical parameters every cycle never re-triggers
    // isNewSegment on its own.
    void forceInactive()
    {
        started = false;
    }

    bool isActive() const
    {
        return started;
    }

    // Active, but not called for WIPE_RESTART_GAP_MS or more: its window ended.
    bool isStale(unsigned long now) const
    {
        return started && (now - lastCall) >= WIPE_RESTART_GAP_MS;
    }

private:
    unsigned long lastCall;
    uint16_t      currentIndex;
    unsigned long lastUpdate;
    bool          started;
    LEDParams     lastParams;  // params from the previous call, for change-detection
};

ColorWipeFillState colorWipeFillInstances[COLOR_WIPE_FILL_COUNT];

class ColorWipeFill
{
public:
    static void Update(CRGB* ledsPtr, uint8_t instance, LEDParams p)
    {
        if (instance >= 1 && instance <= COLOR_WIPE_FILL_COUNT)
        {
            // Clear every wipe/dot whose window ended BEFORE running this one,
            // so a new group starts together on a fresh clock.
            WipeReleaseStale(millis());
            colorWipeFillInstances[instance - 1].run(ledsPtr, p);
        }
    }

    static void Reset(CRGB* ledsPtr, uint8_t instance, LEDParams p)
    {
        if (instance >= 1 && instance <= COLOR_WIPE_FILL_COUNT)
        {
            colorWipeFillInstances[instance - 1].reset(ledsPtr, p);
            WipeReleaseSyncIfIdle();
        }
    }

    // State-only reset — see ColorWipeFillState::forceInactive() above.
    static void ForceInactive(uint8_t instance)
    {
        if (instance >= 1 && instance <= COLOR_WIPE_FILL_COUNT)
        {
            colorWipeFillInstances[instance - 1].forceInactive();
            WipeReleaseSyncIfIdle();
        }
    }

    // Same as ForceInactive(), but for EVERY instance
    // (1..COLOR_WIPE_FILL_COUNT) at once. See SolidFill::ForceInactiveAll().
    static void ForceInactiveAll()
    {
        for (uint8_t i = 1; i <= COLOR_WIPE_FILL_COUNT; i++)
            colorWipeFillInstances[i - 1].forceInactive();
        WipeReleaseSyncIfIdle();
    }
};


// ============================================================
//  ColorWipeDot
//  Moves a single lit dot across LEDs [p.fromLed .. p.toLed-1],
//  turning each LED off after the dot passes.  Restarts when
//  done.  p.waitMs controls ms the dot dwells on each LED.
//
//  Call in loop():
//    ColorWipeDot::Update(leds, 0, {CRGB::Green, 48, 72, 20});
//
//  NOTE: this effect has the identical "won't restart at the first LED"
//  issue ColorWipeFill had (see its comment above) if it's ever called
//  with unchanged parameters on every cycle of a repeating Performance().
//  It has no param-change detection at all — started only ever goes true
//  once per instance, for the life of the sketch — so call
//  ColorWipeDot::ForceInactive(instance) from RestartPerformance() for any
//  instance you use in a Performance() function.
// ============================================================
class ColorWipeDotState
{
public:
    ColorWipeDotState()
    {
        currentIndex = 0;
        lastUpdate   = 0;
        lastCall     = 0;
        started      = false;
    }

    void run(CRGB* ledsPtr, LEDParams p)
    {
        unsigned long now = millis();
        lastCall = now;

        if (!started)
        {
            if (!wipeSyncValid)
            {
                wipeSyncStart = now;
                wipeSyncValid = true;
            }
            started      = true;
            currentIndex = p.forward ? p.fromLed : p.toLed - 1;
            lastUpdate   = WipeGridFloor(now, p.waitMs);
        }

        if (now - lastUpdate < p.waitMs) return;

        WipeAdvance(lastUpdate, now, p.waitMs);

        if (p.forward)
        {
            // Turn off the previous LED
            if (currentIndex > p.fromLed)
                ledsPtr[currentIndex - 1] = CRGB::Black;

            ledsPtr[currentIndex] = p.color;
            FastLED.show();
            currentIndex++;

            if (currentIndex >= p.toLed)
            {
                ledsPtr[currentIndex - 1] = CRGB::Black;
                FastLED.show();
                currentIndex = p.fromLed;
            }
        }
        else
        {
            // Turn off the previous LED
            if (currentIndex < p.toLed - 1)
                ledsPtr[currentIndex + 1] = CRGB::Black;

            ledsPtr[currentIndex] = p.color;
            FastLED.show();

            if (currentIndex == p.fromLed)
            {
                ledsPtr[currentIndex] = CRGB::Black;
                FastLED.show();
                currentIndex = p.toLed - 1;
            }
            else
            {
                currentIndex--;
            }
        }
    }

    void reset(CRGB* ledsPtr, LEDParams p)
    {
        started      = false;
        currentIndex = p.forward ? p.fromLed : p.toLed - 1;
        fill_solid(ledsPtr + p.fromLed, p.toLed - p.fromLed, CRGB::Black);
        FastLED.show();
    }

    // State-only reset — see ColorWipeFillState::forceInactive() above for
    // the full rationale. Forces started back to false, WITHOUT touching
    // the LED buffer or calling FastLED.show().
    void forceInactive()
    {
        started = false;
    }

    bool isActive() const
    {
        return started;
    }

    // Active, but not called for WIPE_RESTART_GAP_MS or more: its window ended.
    bool isStale(unsigned long now) const
    {
        return started && (now - lastCall) >= WIPE_RESTART_GAP_MS;
    }

private:
    unsigned long lastCall;
    uint16_t      currentIndex;
    unsigned long lastUpdate;
    bool          started;
};

ColorWipeDotState colorWipeDotInstances[COLOR_WIPE_DOT_COUNT];

// Clears every wipe/dot instance whose window ended (not called lately), then
// drops the shared clock if nothing is left running. Called at the top of every
// ColorWipeFill/ColorWipeDot Update().
void WipeReleaseStale(unsigned long now)
{
    for (uint8_t i = 0; i < COLOR_WIPE_FILL_COUNT; i++)
        if (colorWipeFillInstances[i].isStale(now))
            colorWipeFillInstances[i].forceInactive();
    for (uint8_t i = 0; i < COLOR_WIPE_DOT_COUNT; i++)
        if (colorWipeDotInstances[i].isStale(now))
            colorWipeDotInstances[i].forceInactive();
    WipeReleaseSyncIfIdle();
}

// Drops the shared wipe clock once no wipe/dot instance is running, so the
// next group starts a new one. Resetting just ONE instance while others keep
// running leaves the clock alone: it rejoins on the same grid.
void WipeReleaseSyncIfIdle()
{
    for (uint8_t i = 0; i < COLOR_WIPE_FILL_COUNT; i++)
        if (colorWipeFillInstances[i].isActive()) return;
    for (uint8_t i = 0; i < COLOR_WIPE_DOT_COUNT; i++)
        if (colorWipeDotInstances[i].isActive()) return;
    wipeSyncValid = false;
}

class ColorWipeDot
{
public:
    static void Update(CRGB* ledsPtr, uint8_t instance, LEDParams p)
    {
        if (instance >= 1 && instance <= COLOR_WIPE_DOT_COUNT)
        {
            WipeReleaseStale(millis());
            colorWipeDotInstances[instance - 1].run(ledsPtr, p);
        }
    }

    static void Reset(CRGB* ledsPtr, uint8_t instance, LEDParams p)
    {
        if (instance >= 1 && instance <= COLOR_WIPE_DOT_COUNT)
        {
            colorWipeDotInstances[instance - 1].reset(ledsPtr, p);
            WipeReleaseSyncIfIdle();
        }
    }

    // State-only reset — see ColorWipeDotState::forceInactive() above.
    static void ForceInactive(uint8_t instance)
    {
        if (instance >= 1 && instance <= COLOR_WIPE_DOT_COUNT)
        {
            colorWipeDotInstances[instance - 1].forceInactive();
            WipeReleaseSyncIfIdle();
        }
    }

    // Same as ForceInactive(), but for EVERY instance
    // (1..COLOR_WIPE_DOT_COUNT) at once. See SolidFill::ForceInactiveAll().
    static void ForceInactiveAll()
    {
        for (uint8_t i = 1; i <= COLOR_WIPE_DOT_COUNT; i++)
            colorWipeDotInstances[i - 1].forceInactive();
        WipeReleaseSyncIfIdle();
    }
};

#endif // LED_EFFECTS_H
