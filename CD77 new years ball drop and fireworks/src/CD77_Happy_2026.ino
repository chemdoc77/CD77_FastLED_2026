// CD77_Happy_2026.cpp by Chemdoc77 
// LED Matrix Animation: Ball Drop and Fireworks for New Year 2026
// Made with a lot of help from Caude.ai

/* License

The code and specifically the animations that come with it are licenced under a Creative Commons Attribution License CC BY-NC-SA 4.0

For non-commercial purposes you're welcome to use the code and animations in any way you like.

In case you would like to use my code or animations in your commercial project or product please contact me and we'll find a fair licensing solution we're both happy with.
*/

#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN     12
#define NUM_LEDS    256
#define BRIGHTNESS  50 
#define MATRIX_WIDTH  16
#define MATRIX_HEIGHT 16
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// 5x3 digits
const bool digits[10][5][3] = {
  {{1,1,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1}}, // 0
  {{0,1,0},{1,1,0},{0,1,0},{0,1,0},{1,1,1}}, // 1
  {{1,1,1},{0,0,1},{1,1,1},{1,0,0},{1,1,1}}, // 2
  {{1,1,1},{0,0,1},{1,1,1},{0,0,1},{1,1,1}}, // 3
  {{1,0,1},{1,0,1},{1,1,1},{0,0,1},{0,0,1}}, // 4
  {{1,1,1},{1,0,0},{1,1,1},{0,0,1},{1,1,1}}, // 5
  {{1,1,1},{1,0,0},{1,1,1},{1,0,1},{1,1,1}}, // 6
  {{1,1,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1}}, // 7
  {{1,1,1},{1,0,1},{1,1,1},{1,0,1},{1,1,1}}, // 8
  {{1,1,1},{1,0,1},{1,1,1},{0,0,1},{1,1,1}}  // 9
};

// 5x3 letters
const bool letters[8][5][3] = {
  {{1,0,1},{1,0,1},{1,1,1},{1,0,1},{1,0,1}}, // H
  {{0,1,0},{1,0,1},{1,1,1},{1,0,1},{1,0,1}}, // A
  {{1,1,0},{1,0,1},{1,1,0},{1,0,0},{1,0,0}}, // P
  {{1,0,1},{1,0,1},{0,1,0},{0,1,0},{0,1,0}}, // Y
  {{1,0,1},{1,1,1},{1,0,1},{1,0,1},{1,0,1}}, // N
  {{1,1,1},{1,0,0},{1,1,0},{1,0,0},{1,1,1}}, // E
  {{1,0,1},{1,0,1},{1,0,1},{1,1,1},{1,0,1}}, // W
  {{1,1,0},{1,0,1},{1,1,0},{1,0,1},{1,0,1}}  // R
};

// XY function for serpentine column-major matrix
uint16_t XY(uint8_t x, uint8_t y) {
  uint16_t i;
  if (x & 0x01) {
    i = (x * MATRIX_HEIGHT) + (MATRIX_HEIGHT - 1 - y);
  } else {
    i = (x * MATRIX_HEIGHT) + y;
  }
  return i;
}

void drawDigit(int digit, int x, int y, CRGB color) {
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 3; col++) {
      if (digits[digit][row][col]) {
        if (x + col < MATRIX_WIDTH && y + row < MATRIX_HEIGHT) {
          leds[XY(x + col, y + row)] = color;
        }
      }
    }
  }
}

void drawLetter(int letterIdx, int x, int y, CRGB color) {
  for (int row = 0; row < 5; row++) {
    for (int col = 0; col < 3; col++) {
      if (letters[letterIdx][row][col]) {
        if (x + col < MATRIX_WIDTH && y + row < MATRIX_HEIGHT) {
          leds[XY(x + col, y + row)] = color;
        }
      }
    }
  }
}

// ============ FIREWORKS PATTERN CLASS ============
class FireworksPattern {
private:
  struct Rocket {
    float x, y;
    float vx, vy;
    CRGB color;
    bool active;
    bool exploded;
  };

  struct Particle {
    float x, y;
    float vx, vy;
    CRGB color;
    uint8_t life;
    bool active;
  };

  static const uint8_t MAX_ROCKETS = 3;
  static const uint8_t MAX_PARTICLES = 30;
  
  Rocket rockets[MAX_ROCKETS];
  Particle particles[MAX_PARTICLES];
  
  uint8_t launchChance;
  unsigned long lastUpdate;
  uint16_t updateInterval;
  unsigned long startTime;
  unsigned long duration;
  bool isActive;

  void launchRocket() {
    for (int i = 0; i < MAX_ROCKETS; i++) {
      if (!rockets[i].active) {
        rockets[i].x = random(4, MATRIX_WIDTH - 4);  // Keep away from edges
        rockets[i].y = MATRIX_HEIGHT - 1;  // Start from bottom
        
        // More vertical angles for realistic upward trajectory
        float angle = random(-100, -81) * PI / 180.0;  // Mostly straight up
        float speed = random(90, 130) / 100.0;  // Good speed variation
        
        rockets[i].vx = cos(angle) * speed;
        rockets[i].vy = sin(angle) * speed;  // Negative Y goes up
        rockets[i].color = CHSV(random(0, 256), 255, 255);
        rockets[i].active = true;
        rockets[i].exploded = false;
        break;
      }
    }
  }

  void explodeRocket(int idx) {
    int particlesCreated = 0;
    // Create more particles for a fuller burst
    int numParticles = random(12, 18);  // Variable number of particles
    float angleStep = 360.0 / numParticles;
    
    for (int i = 0; i < MAX_PARTICLES && particlesCreated < numParticles; i++) {
      if (!particles[i].active) {
        particles[i].x = rockets[idx].x;
        particles[i].y = rockets[idx].y;
        
        // Use evenly spaced angles with slight randomization
        float angle = (particlesCreated * angleStep + random(-8, 8)) * PI / 180.0;
        float speed = random(55, 75) / 100.0;  // Consistent speed for roundness
        
        particles[i].vx = cos(angle) * speed;
        particles[i].vy = sin(angle) * speed;
        particles[i].color = rockets[idx].color;
        particles[i].life = random(220, 255);  // Longer life for better visibility
        particles[i].active = true;
        particlesCreated++;
      }
    }
    rockets[idx].active = false;
  }

  void updateRockets() {
    for (int i = 0; i < MAX_ROCKETS; i++) {
      if (rockets[i].active && !rockets[i].exploded) {
        rockets[i].x += rockets[i].vx;
        rockets[i].y += rockets[i].vy;
        rockets[i].vy += 0.05;  // Gravity
        
        // Explode when rocket starts falling (vy becomes positive) or reaches top area
        // This ensures all rockets explode before leaving the screen
        if (rockets[i].vy >= -0.1 || rockets[i].y <= 3) {
          explodeRocket(i);
          rockets[i].exploded = true;
        }
        
        int ix = (int)rockets[i].x;
        int iy = (int)rockets[i].y;
        if (ix >= 0 && ix < MATRIX_WIDTH && iy >= 0 && iy < MATRIX_HEIGHT) {
          leds[XY(ix, iy)] = rockets[i].color;
        }
      }
    }
  }

  void updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
      if (particles[i].active) {
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        particles[i].vy += 0.08;  // Gravity pulls down
        particles[i].vx *= 0.95;
        particles[i].vy *= 0.98;
        
        particles[i].life -= 8;
        if (particles[i].life < 10) {
          particles[i].active = false;
          continue;
        }
        
        int ix = (int)particles[i].x;
        int iy = (int)particles[i].y;
        if (ix >= 0 && ix < MATRIX_WIDTH && iy >= 0 && iy < MATRIX_HEIGHT) {
          CRGB color = particles[i].color;
          color.fadeToBlackBy(255 - particles[i].life);
          leds[XY(ix, iy)] = color;
        }
      }
    }
  }

public:
  FireworksPattern(uint8_t launch_chance = 4, uint16_t interval = 30) {
    launchChance = launch_chance;
    updateInterval = interval;
    lastUpdate = 0;
    isActive = false;
    duration = 10000; // 10 seconds default
    
    for (int i = 0; i < MAX_ROCKETS; i++) {
      rockets[i].active = false;
    }
    for (int i = 0; i < MAX_PARTICLES; i++) {
      particles[i].active = false;
    }
  }

  void Start(unsigned long fireworksDuration = 10000) {
    isActive = true;
    startTime = millis();
    duration = fireworksDuration;
    lastUpdate = millis();
  }

  bool Update() {
    if (!isActive) return false;
    
    unsigned long currentMillis = millis();
    
    // Check if duration has expired
    if (currentMillis - startTime >= duration) {
      isActive = false;
      FastLED.clear();
      FastLED.show();
      return false; // Finished
    }
    
    // Only update at the specified interval
    if ((currentMillis - lastUpdate) >= updateInterval) {
      lastUpdate = currentMillis;
      
      fadeToBlackBy(leds, NUM_LEDS, 60);
      
      if (random(100) < launchChance) {
        launchRocket();
      }
      
      updateRockets();
      updateParticles();
      
      FastLED.show();
    }
    
    return true; // Still running
  }

  bool IsActive() {
    return isActive;
  }
};

// ============ BALL DROP ANIMATION CLASS ============
class BallDropAnimation {
  enum State {
    DROPPING,
    SHOWING_MESSAGE,
    CLEARING,
    IDLE
  };
  
  State currentState;
  unsigned long previousMillis;
  unsigned long dropStartTime;
  unsigned long dropDuration;
  unsigned long messageDisplayTime;
  unsigned long clearDelay;
  int centerX;
  
public:
  BallDropAnimation(unsigned long dropTime = 10000, unsigned long msgTime = 5000, unsigned long clearTime = 500) {
    dropDuration = dropTime;
    messageDisplayTime = msgTime;
    clearDelay = clearTime;
    currentState = IDLE;
    previousMillis = 0;
    dropStartTime = 0;
    centerX = 8;
  }
  
  void Start() {
    currentState = DROPPING;
    dropStartTime = millis();
    previousMillis = millis();
  }
  
  bool Update() {
    unsigned long currentMillis = millis();
    
    switch(currentState) {
      case DROPPING:
        updateDropping(currentMillis);
        break;
        
      case SHOWING_MESSAGE:
        updateMessage(currentMillis);
        break;
        
      case CLEARING:
        updateClearing(currentMillis);
        break;
        
      case IDLE:
        return false; // Animation complete
    }
    return true; // Still running
  }
  
  bool IsIdle() {
    return currentState == IDLE;
  }
  
private:
  void updateDropping(unsigned long currentMillis) {
    unsigned long elapsed = currentMillis - dropStartTime;
    
    if (elapsed >= dropDuration) {
      currentState = SHOWING_MESSAGE;
      previousMillis = currentMillis;
      showMessage();
    } else {
      int countdown = 10 - (elapsed / 1000);
      if (countdown < 0) countdown = 0;
      
      if (countdown >= 10) {
        drawDigit(1, 0, 0, CRGB::White);
        drawDigit(0, 4, 0, CRGB::White);
      } else {
        drawDigit(countdown, 0, 0, CRGB::White);
      }
      
      float progress = (float)elapsed / dropDuration;
      int ballY = 2 + (int)(progress * 11);
      
      drawBall(centerX, ballY);
    }
  }
  
  void updateMessage(unsigned long currentMillis) {
    if (currentMillis - previousMillis >= messageDisplayTime) {
      currentState = CLEARING;
      previousMillis = currentMillis;
      FastLED.clear();
      FastLED.show();
    } else {
      showMessage();
    }
  }
  
  void updateClearing(unsigned long currentMillis) {
    if (currentMillis - previousMillis >= clearDelay) {
      currentState = IDLE;
    }
  }
  
  void drawBall(int centerX, int centerY) {
    CRGB ballColor = CRGB::White;
    
    for (int dy = -2; dy <= 2; dy++) {
      for (int dx = -2; dx <= 2; dx++) {
        if (dx * dx + dy * dy <= 5) {
          int x = centerX + dx;
          int y = centerY + dy;
          if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
            if (random(100) < 30) {
              leds[XY(x, y)] = CRGB::Blue;
            } else {
              if (dx * dx + dy * dy <= 2) {
                leds[XY(x, y)] = ballColor;
              } else {
                leds[XY(x, y)] = ballColor;
                leds[XY(x, y)].fadeToBlackBy(100);
              }
            }
          }
        }
      }
    }
  }
  
  void showMessage() {
    FastLED.clear();
    CRGB dimRed = CRGB(80, 0, 0);
    
    drawLetter(0, 1, 4, dimRed);
    drawLetter(1, 4, 4, dimRed);
    drawLetter(2, 7, 4, dimRed);
    drawLetter(2, 10, 4, dimRed);
    drawLetter(3, 13, 4, dimRed);
    
    drawDigit(2, 2, 10, dimRed);
    drawDigit(0, 5, 10, dimRed);
    drawDigit(2, 8, 10, dimRed);
    drawDigit(6, 11, 10, dimRed);
    
    FastLED.show();
  }
};

// ============ ANIMATION CONTROLLER ============
enum AnimationState {
  BALL_DROP_PHASE,
  FIREWORKS_PHASE,
  WAITING
};

AnimationState animState = BALL_DROP_PHASE;
BallDropAnimation ballDrop;
FireworksPattern fireworks;
unsigned long waitStartTime = 0;
const unsigned long WAIT_DURATION = 1000; // 1 second between cycles

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  randomSeed(analogRead(0));
  
  ballDrop.Start();
}

void loop() {
  static unsigned long lastUpdateTime = 0;
  unsigned long currentMillis = millis();
  
  // Update at ~20 FPS without using delay()
  if (currentMillis - lastUpdateTime >= 50) {
    lastUpdateTime = currentMillis;
    
    switch(animState) {
      case BALL_DROP_PHASE:
        FastLED.clear();
        if (!ballDrop.Update()) {
          // Ball drop finished, start fireworks
          animState = FIREWORKS_PHASE;
          fireworks.Start(20000); // 20 seconds of fireworks
        }
        FastLED.show();
        break;
        
      case FIREWORKS_PHASE:
        if (!fireworks.Update()) {
          // Fireworks finished, wait then restart
          animState = WAITING;
          waitStartTime = currentMillis;
        }
        break;
        
      case WAITING:
        if (currentMillis - waitStartTime >= WAIT_DURATION) {
          // Restart the sequence
          animState = BALL_DROP_PHASE;
          ballDrop.Start();
        }
        break;
    }
  }
}