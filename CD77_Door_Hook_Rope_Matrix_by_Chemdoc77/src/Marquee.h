#ifndef MARQUEE_H
#define MARQUEE_H

//=======================================================



// Marquee Class - encapsulates the marquee effect using non-blocking timing
class Marquee {
  // Class Member Variables
  CRGB* leds;          // Pointer to LED array
  int numLeds;         // Total number of LEDs
  int color1Leds;      // Number of LEDs for color1
  int color2Leds;      // Number of LEDs for color2
  CRGB color1;         // First color
  CRGB color2;         // Second color
  int updateInterval;  // Milliseconds per LED movement
  unsigned long previousMillis; // Last update time
  int offset;          // Current position offset

public:
  // Constructor - initializes the marquee
  Marquee(CRGB* ledArray, int total, int c1Leds, int c2Leds, 
          CRGB col1, CRGB col2, int speed) {
    leds = ledArray;
    numLeds = total;
    color1Leds = c1Leds;
    color2Leds = c2Leds;
    color1 = col1;
    color2 = col2;
    updateInterval = speed;
    previousMillis = 0;
    offset = 0;
  }

  // Update function - call this from loop()
  void Update() {
    unsigned long currentMillis = millis();
    
    // Check if it's time to update
    if (currentMillis - previousMillis >= updateInterval) {
      previousMillis = currentMillis;
      
      // Move the pattern
      offset--;
      
      // Wrap around when reaching the beginning
      if (offset < 0) {
        offset = (color1Leds + color2Leds) - 1;
      }
      
      // Draw the pattern
      DrawPattern();
    }
  }

  // Draw the marquee pattern on the LED strip
  void DrawPattern() {
    int patternLength = color1Leds + color2Leds;
    
    for (int i = 0; i < numLeds; i++) {
      // Calculate position in pattern with offset
      int pos = (i + offset) % patternLength;
      
      // Assign color based on position in pattern
      if (pos < color1Leds) {
        leds[i] = color1;
      } else {
        leds[i] = color2;
      }
    }
    
    FastLED.show();
  }

  // Change colors on the fly
  void SetColors(CRGB col1, CRGB col2) {
    color1 = col1;
    color2 = col2;
  }

  // Change speed on the fly
  void SetSpeed(int speed) {
    updateInterval = speed;
  }

  // Change pattern sizes on the fly
  void SetPattern(int c1Leds, int c2Leds) {
    color1Leds = c1Leds;
    color2Leds = c2Leds;
    // Reset offset to prevent issues
    offset = 0;
  }
};

// Global objects

Marquee marquee(leds, NUM_LEDS, 10, 10, CRGB::Red, CRGB::White, 35);  // changed from 35 by CD77

// Pattern switching variables
#define PATTERN_DURATION 10000  // Duration in milliseconds (z seconds * 1000)
unsigned long patternStartTime = 0;
int currentPattern = 0;  // 0 = Red/White, 1 = Blue/White

#endif