#ifndef CD77_SLOWCHANGEPALETTE_H
#define CD77_SLOWCHANGEPALETTE_H

#include <FastLED.h>

/*
 * CD77_slowchangepalette.h
 * 
 * Non-blocking multitasking class for smooth palette transitions
 * Based on Mark Kriegsman's PaletteCrossfade.ino
 * 
 * This class smoothly blends from currentPalette to targetPalette
 * and animates the LEDs with a moving color pattern.
 * 
 * 
 * Put this to change palettes paletteAnimation.setTargetPalette(CRGBPalette16(CRGB::Red));
 * put this in ino file:
 * 
 * After: // LED array
CRGB leds[NUM_LEDS];

PUT THIS in ino file:

// Create the palette animation object (palettes are handled internally)
CD77_slowchangepalette paletteAnimation(leds, NUM_LEDS, BRIGHTNESS);
 */

// Function declarations
void FillLEDsFromPaletteColors(uint8_t colorIndex);
void CD77_slowchangepalette_update(uint8_t maxChanges, uint16_t wait);

class CD77_slowchangepalette {
  private:
    CRGB* leds;                      // Pointer to LED array
    CRGBPalette16 currentPalette;    // Current palette (stored in class)
    CRGBPalette16 targetPalette;     // Target palette (stored in class)
    uint16_t numLeds;                // Number of LEDs
    uint8_t brightness;              // LED brightness
    unsigned long lastUpdate;        // Last update time (millis)
    uint16_t updateInterval;         // Update interval in milliseconds
    uint8_t maxChanges;              // Palette blend speed (1-48)
    uint8_t startIndex;              // Color index for animation
    uint8_t motionSpeed;             // How fast to increment color index
    
    /**
     * FillLEDsFromPaletteColors
     * Helper function to fill all LEDs with colors from the current palette
     */
    void FillLEDsFromPaletteColors() {
      CRGB color = ColorFromPalette(currentPalette, 1, brightness, LINEARBLEND);
      for(uint16_t i = 0; i < numLeds; i++) {
        leds[i] = color;
      }
    }
    
  public:
    /**
     * Constructor
     * 
     * @param ledArray - Pointer to CRGB LED array
     * @param numLeds - Number of LEDs in the array
     * @param brightness - LED brightness (0-255, default: 150)
     */
    CD77_slowchangepalette(CRGB* ledArray, uint16_t numLeds, uint8_t brightness = 150) {
      this->leds = ledArray;
      this->numLeds = numLeds;
      this->brightness = brightness;
      
      // Initialize palettes (start with black, fade to red)
      this->currentPalette = CRGBPalette16(CRGB::Black);
      this->targetPalette = CRGBPalette16(CRGB::Red);
      
      // Animation settings with good defaults
      this->lastUpdate = 0;
      this->updateInterval = 100;  // 10 FPS
      this->maxChanges = 48;       // Fast blend
      this->startIndex = 0;
      this->motionSpeed = 2;       // Moderate motion speed
    }
    
    /**
     * Update
     * 
     * Non-blocking update function - call this repeatedly in loop()
     * Handles palette blending and LED animation
     */
    void Update() {
      unsigned long currentMillis = millis();
      
      // Only update animation at the specified interval
      if (currentMillis - lastUpdate >= updateInterval) {
        lastUpdate = currentMillis;
        
        // Crossfade current palette slowly toward the target palette
        nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
        
        // Animate the color index to create movement
        startIndex = startIndex + motionSpeed;
        
        // Fill LEDs with colors from the palette
        FillLEDsFromPaletteColors();
      }
    }
    
    /**
     * setMaxChanges
     * Set the palette blend speed (1-48)
     * Higher values = faster transitions
     */
    void setMaxChanges(uint8_t changes) {
      maxChanges = changes;
    }
    
    /**
     * setUpdateInterval
     * Set the animation update rate in milliseconds
     * Lower values = smoother animation
     */
    void setUpdateInterval(uint16_t interval) {
      updateInterval = interval;
    }
    
    /**
     * setMotionSpeed
     * Set how fast the color index increments
     * Higher values = faster animation movement
     */
    void setMotionSpeed(uint8_t speed) {
      motionSpeed = speed;
    }
    
    /**
     * setBrightness
     * Change the LED brightness (0-255)
     */
    void setBrightness(uint8_t newBrightness) {
      brightness = newBrightness;
    }
    
    /**
     * setTargetPalette
     * Manually set the target palette
     * Use this to change palettes from your sketch (e.g., in ChangePalettePeriodically)
     * 
     * @param palette - The new target palette to transition to
     */
    void setTargetPalette(CRGBPalette16 palette) {
      targetPalette = palette;
    }
    
    /**
     * getCurrentPalette
     * Get the current palette
     * 
     * @return The current palette
     */
    CRGBPalette16 getCurrentPalette() {
      return currentPalette;
    }
    
    /**
     * getTargetPalette
     * Get the target palette
     * 
     * @return The target palette
     */
    CRGBPalette16 getTargetPalette() {
      return targetPalette;
    }
};

#endif // CD77_SLOWCHANGEPALETTE_H
