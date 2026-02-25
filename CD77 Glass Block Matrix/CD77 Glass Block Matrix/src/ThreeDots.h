#ifndef THREEDOTS_H
#define THREEDOTS_H

// ThreeDots - Non-blocking LED matrix animation
// Based on original code by sutaburosu:
// sdf_circle2
// https://wokwi.com/projects/329474825790685778
// Modified by chemdoc77 and refactored for multitasking

class ThreeDots {
  private:
    struct Ball {
      float x_speed;
      float y_speed;
      float radius;
      float transition;
      CRGB colour;
    };
    
    CRGB* leds;              // Pointer to LED array
    uint16_t numLeds;        // Total number of LEDs
    uint8_t width;           // Matrix width
    uint8_t height;          // Matrix height
    unsigned long lastUpdate; // Last update time
    uint16_t updateInterval;  // Update interval in milliseconds
    bool fpsEnabled;         // Enable FPS counter
    uint8_t fpsFrame;        // FPS frame counter
    
    // Three balls with default settings
    Ball balls[3] = {
      {8.0f, 9.58f, 1.5f, 3.0f, CRGB::Red},    // Red ball
      {7.0f, 7.2f, 1.5f, 3.0f, CRGB::Green},   // Green ball
      {6.0f, 6.5f, 1.5f, 3.0f, CRGB::Blue}     // Blue ball
    };
    
    // XY coordinate mapping for serpentine matrix
    // LED 0 is in upper left, LED 1 is below LED 0
    uint16_t XY(uint8_t x, uint8_t y) {
      if (x >= width) return numLeds;
      if (y >= height) return numLeds;
      if (y & 1)  // Odd rows go right to left
        return (y + 1) * width - 1 - x;
      else        // Even rows go left to right
        return y * width + x;
    }
    
  public:
    // Constructor
    ThreeDots(CRGB* ledArray, uint8_t matrixWidth, uint8_t matrixHeight) {
      leds = ledArray;
      width = matrixWidth;
      height = matrixHeight;
      numLeds = width * height;
      lastUpdate = 0;
      updateInterval = 16;  // ~60 FPS default
      fpsEnabled = false;
      fpsFrame = 0;
    }
    
    // Update the animation (call this in loop)
    void Update() {
      unsigned long currentTime = millis();
      
      // Only update when interval has elapsed
      if ((currentTime - lastUpdate) < updateInterval) {
        return;
      }
      
      lastUpdate = currentTime;
      
      // Clear the display first
      FastLED.clear();
      
      // Process each ball
      for (uint8_t b = 0; b < 3; b++) {
        // Calculate ball position using sine and cosine
        float offset_x = sinf(balls[b].x_speed * currentTime / 8192.0f) * width / 2.0f - width / 2.0f;
        float offset_y = cosf(balls[b].y_speed * currentTime / 8192.0f) * height / 2.0f - height / 2.0f;
        float radius = balls[b].radius;
        float transition = balls[b].transition;
        float max_sum_squares = powf(radius + transition, 2);
        
        // Draw the ball with soft edges
        for (uint8_t y = 0; y < height; y++) {
          for (uint8_t x = 0; x < width; x++) {
            float sum_squares = powf(x + offset_x, 2) + powf(y + offset_y, 2);
            if (sum_squares > max_sum_squares)
              continue;
              
            CRGB faded = balls[b].colour;
            float distance = sqrtf(sum_squares) - radius;
            if (distance >= 0)
              faded %= 255 - 255.0f * distance / transition;
            leds[XY(x, y)] += faded;
          }
        }
      }
      
      // FPS counter (optional)
      if (fpsEnabled) {
        if (!++fpsFrame) {
          Serial.println(FastLED.getFPS());
        }
      }
    }
    
    // Configuration methods
    
    // Set update interval in milliseconds (lower = faster animation)
    void setUpdateInterval(uint16_t interval) {
      updateInterval = interval;
    }
    
    // Enable/disable FPS counter (requires Serial.begin)
    void enableFPS(bool enable) {
      fpsEnabled = enable;
    }
    
    // Set individual ball properties
    void setBall(uint8_t ballIndex, float xSpeed, float ySpeed, float radius, float transition, CRGB color) {
      if (ballIndex < 3) {
        balls[ballIndex].x_speed = xSpeed;
        balls[ballIndex].y_speed = ySpeed;
        balls[ballIndex].radius = radius;
        balls[ballIndex].transition = transition;
        balls[ballIndex].colour = color;
      }
    }
    
    // Set ball color only
    void setBallColor(uint8_t ballIndex, CRGB color) {
      if (ballIndex < 3) {
        balls[ballIndex].colour = color;
      }
    }
    
    // Set ball speed
    void setBallSpeed(uint8_t ballIndex, float xSpeed, float ySpeed) {
      if (ballIndex < 3) {
        balls[ballIndex].x_speed = xSpeed;
        balls[ballIndex].y_speed = ySpeed;
      }
    }
    
    // Set ball radius and transition (softness)
    void setBallSize(uint8_t ballIndex, float radius, float transition) {
      if (ballIndex < 3) {
        balls[ballIndex].radius = radius;
        balls[ballIndex].transition = transition;
      }
    }
    
    // Get ball color
    CRGB getBallColor(uint8_t ballIndex) {
      if (ballIndex < 3) {
        return balls[ballIndex].colour;
      }
      return CRGB::Black;
    }
};

#endif // THREEDOTS_H
