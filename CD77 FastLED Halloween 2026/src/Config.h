#pragma once
/*
  Config.h - shared settings for Halloween2026
  Board : Lolin D32 (ESP32)
  LEDs  : NEOPIXEL (WS2812B) on GPIO 12, 256 LEDs, FastLED brightness 75
*/
#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN              12
#define CHIPSET              NEOPIXEL
#define NUM_LEDS             256
#define BRIGHTNESS           220    // FastLED global brightness (0-255)
#define FRAMES_PER_SECOND    60      // default frame rate for Fire

#define MAX_VOLTS            5
#define MAX_MILLIAMPS        10500
#define POWER_INDICATOR_PIN  13

// The LED array is defined once in main.cpp
extern CRGB leds[NUM_LEDS];
