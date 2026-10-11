#include <Arduino.h>

//New Product Test Sketch by Chemdoc77
//used to test new RGB LED strips and Matrix that I purchase.

// 6/22/19 - updated to allow more than 255 LEDS.

#include <FastLED.h>

#define DATA_PIN     12
#define LED_TYPE     NEOPIXEL 


#define NUM_LEDS 96

CRGB leds[NUM_LEDS];
int brightness = 70;
void cd77_colorwipe_dot(CRGB color,uint16_t from,  uint16_t to,  uint16_t wait);
void cd77_colorwipe(CRGB color,  uint16_t to, uint16_t wait);


void setup() { 
  delay(1000);
  FastLED.addLeds<LED_TYPE, DATA_PIN>(leds, NUM_LEDS);         
  FastLED.setDither(false);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 8000);
   set_max_power_indicator_LED(13);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
 
}
void loop() {  

     fill_solid( leds, NUM_LEDS, CRGB::Red);
     FastLED.delay(500);
      
     fill_solid( leds, NUM_LEDS, CRGB::Black);
     FastLED.delay(500);
     fill_solid( leds, NUM_LEDS, CRGB::Blue);
     FastLED.delay(500);
      
    fill_solid( leds, NUM_LEDS, CRGB::Black);
    FastLED.delay(500);
  
    fill_solid( leds, NUM_LEDS, CRGB::Green);
    FastLED.delay(500);
      
    fill_solid( leds, NUM_LEDS, CRGB::Black);
    FastLED.delay(500); 
       
    fill_solid( leds, NUM_LEDS, CRGB::Yellow);
    FastLED.delay(500);
      
    fill_solid( leds, NUM_LEDS, CRGB::Black);
/*
    cd77_colorwipe_dot(CRGB::Red, 0, NUM_LEDS, 5);
    cd77_colorwipe_dot(CRGB::Blue, 0, NUM_LEDS, 5);
    cd77_colorwipe_dot(CRGB::Green, 0, NUM_LEDS, 5);
*/
 cd77_colorwipe(CRGB::Red, NUM_LEDS, 400) ;
  cd77_colorwipe(CRGB::Blue, NUM_LEDS, 15) ;
  cd77_colorwipe(CRGB::Green, NUM_LEDS, 15) ;
}
//==================== Functions ===============================
void cd77_colorwipe(CRGB color,  uint16_t to, uint16_t wait) {
  for (uint16_t i = 0; i <to; i++) {
    leds[i] = color;
    FastLED.delay(wait);
  }
}

void cd77_colorwipe_dot(CRGB color,uint16_t from,  uint16_t to,  uint16_t wait) {
  for (uint16_t i = from; i <to; i++) {
    leds[i] = color;
    FastLED.delay(wait);
    leds[i] = CRGB::Black;
    FastLED.show();
    
    
  }
}

