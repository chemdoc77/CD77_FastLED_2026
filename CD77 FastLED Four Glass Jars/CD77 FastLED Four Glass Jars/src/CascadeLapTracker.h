#pragma once
#include <FastLED.h>

/*
 * CascadeLapTracker.h
 *
 * Small helper class used by the "build one jar, hold, then start the
 * next" cascade effects in Performance2() (Time_performance.h) --
 * currently the squareFillMultiJar and rowGradientFillJar cascades.
 *
 * WHY THIS EXISTS:
 * Gating "should jar N+1 start yet?" purely on jar N's isComplete() is
 * NOT safe across laps. isComplete() reflects "has this jar EVER
 * finished" -- on lap 2+, jar 1/2/3 are still marked complete=true from
 * the END of lap 1, even before their Update() has been called even
 * once in the new lap. That stale "true" would leak through and unlock
 * jar 2 in the very first frame of the new lap, running it at the same
 * time as jar 0's fresh rebuild -- exactly the bug this class fixes.
 *
 * CascadeLapTracker keeps its own "stage" counter (0..N) that only
 * advances forward WITHIN THE CURRENT LAP, so a jar's completion only
 * ever unlocks the next jar once, at the right point in the lap. All of
 * this state lives inside the class instance (matching the rest of this
 * sketch's design), rather than as loose static variables floating in
 * Performance2()'s body.
 *
 * TWO WAYS TO RESET A CASCADE AT THE START OF ITS LAP/WINDOW:
 *
 *   1. checkNewLap(gPerformanceRunID) -- for cascades whose window
 *      STARTS AT t=0.000. AT(0,0,00.000) can never fire (a real edge
 *      case in the atTC() framework: it requires
 *      gLastTimeCodeDoneAt < tc, but that variable starts at/resets to
 *      0, so for tc=0 the check is always "0 < 0", never true). Calling
 *      checkNewLap() every frame inside the cascade's own BETWEEN block
 *      works around this by watching gPerformanceRunID itself change
 *      instead -- it only changes once per lap (inside
 *      RestartPerformance()), so this fires exactly once per lap,
 *      including the very first one, and resets the stage back to 0.
 *
 *   2. reset() -- for cascades whose window starts at any t > 0.000,
 *      where a normal AT(0,0,xx.000) block already fires correctly
 *      once per lap. Call reset() unconditionally inside that AT block.
 *
 * Usage (always call these methods from inside a BETWEEN/FROM/AT block
 * -- never as loose top-level code in Performance()/Performance2()):
 *
 *   // Cascade whose window starts at t=0.000:
 *   BETWEEN(START(0,0,00.000), TO(0,0,08.000)) {
 *     if (squareCascadeTracker.checkNewLap(gPerformanceRunID)) {
 *       // one-time per-lap setup, e.g. clear the waiting jars
 *     }
 *     effectJar[0].Update(...);
 *     if (squareCascadeTracker.stage() == 0 && effectJar[0].isComplete()) {
 *       squareCascadeTracker.advance();
 *     }
 *     if (squareCascadeTracker.stage() >= 1) {
 *       effectJar[1].Update(...);
 *       ...
 *     }
 *   }
 *
 *   // Cascade whose window starts at t > 0.000:
 *   AT(0,0,13.000) {
 *     rowGradCascadeTracker.reset();
 *     // one-time per-lap setup, e.g. clear the waiting jars
 *   }
 *   BETWEEN(START(0,0,13.000), TO(0,0,24.000)) {
 *     effectJar[0].Update(...);
 *     if (rowGradCascadeTracker.stage() == 0 && effectJar[0].isComplete()) {
 *       rowGradCascadeTracker.advance();
 *     }
 *     ...
 *   }
 */

class CascadeLapTracker {
public:
  // For cascades whose window starts at t=0.000. Call every frame from
  // inside that cascade's own BETWEEN block. Returns true exactly once
  // per lap (the first call after gPerformanceRunID changes), and
  // resets stage() back to 0 at the same time.
  bool checkNewLap(uint32_t currentRunID) {
    if (currentRunID != _lastSeenRunID) {
      _lastSeenRunID = currentRunID;
      _stage = 0;
      return true;
    }
    return false;
  }

  // For cascades whose window starts at t > 0.000. Call unconditionally
  // from inside a normal AT(0,0,xx.000) block for that window's start
  // time -- AT() fires correctly once per lap for any non-zero tc.
  void reset() {
    _stage = 0;
  }

  // Current cascade stage: 0 = only the first jar is running, 1 = the
  // second jar has been unlocked, 2 = the third, 3 = the fourth, etc.
  uint8_t stage() const { return _stage; }

  // Unlocks the next jar in the cascade. Call this once, guarded by
  // "stage() == N && jarN.isComplete()", so it only ever fires once
  // per jar per lap.
  void advance() { _stage++; }

private:
  uint8_t  _stage         = 0;
  uint32_t _lastSeenRunID = 0; // sentinel; gPerformanceRunID is always >=1 by the time Performance()/Performance2() first runs
};

// ---------- Instances ----------
// Tracks cascade progress for the "build one jar, hold, start the next"
// effects used in Performance2(): squareFillMultiJar and rowGradientFillJar.
CascadeLapTracker squareCascadeTracker;
CascadeLapTracker rowGradCascadeTracker;
