#ifndef Time_performance
#define Time_performance

/* This code is based on the Time Performance sketch by Mark Kriegsman of FastLED at:
   https://gist.github.com/kriegsman/a916be18d32ec675fea8
*/

#define TC(HOURS,MINUTES,SECONDS) \
  ((uint32_t)(((uint32_t)((HOURS)*(uint32_t)(3600000))) + \
  ((uint32_t)((MINUTES)*(uint32_t)(60000))) + \
  ((uint32_t)((SECONDS)*(uint32_t)(1000)))))


#define AT(HOURS,MINUTES,SECONDS) if( atTC(TC(HOURS,MINUTES,SECONDS)) )
#define FROM(HOURS,MINUTES,SECONDS) if( fromTC(TC(HOURS,MINUTES,SECONDS)) )

static bool atTC( uint32_t tc)
{
  bool maybe = false;
  if( gTimeCode >= tc) {
    if( gLastTimeCodeDoneAt < tc) {
      maybe = true;
      gLastTimeCodeDoneAt = tc;
    }
  }
  return maybe;
}

static bool fromTC( uint32_t tc)
{
  bool maybe = false;
  if( gTimeCode >= tc) {
    if( gLastTimeCodeDoneFrom <= tc) {
      maybe = true;
      gLastTimeCodeDoneFrom = tc;
    }
  }
  return maybe;
} 

void RestartPerformance()
{
  gLastTimeCodeDoneAt = 0;
  gLastTimeCodeDoneFrom = 0;
  gTimeCodeBase = millis();
}

//note: add  random16_add_entropy( random(0,65535)); and FastLED.delay(1000 / FRAMES_PER_SECOND); for Fire2012 entries




void Performance()
{
  FROM(0,0,0.000)  {speed = 30; currentPalette = HO1_p; northPalette();  FastLED.show();}
  FROM(0,0,8.000)  {currentPalette = HO1_p; southPalette();  FastLED.show();}
 // FROM(0,0,16.000)   {FastLED.clear();FastLED.show();}
  FROM(0,0,16.100) {unsigned long currentMillis = millis();
  
  // Check if it's time to switch patterns
  if (currentMillis - patternStartTime >= PATTERN_DURATION) {
    patternStartTime = currentMillis;
    
    // Toggle between patterns
    if (currentPattern == 0) {
      // Switch to Blue/White
      marquee.SetColors(CRGB::Blue, CRGB::White);
      currentPattern = 1;
      Serial.println("Pattern: Blue/White");
    } else {
      // Switch to Red/White
      marquee.SetColors(CRGB::Red, CRGB::White);
      currentPattern = 0;
      Serial.println("Pattern: Red/White");
    }
  }
  
  // Update the marquee effect
  marquee.Update();
}
  FROM(0,0,36.000)   {FastLED.clear();FastLED.show();} 
  FROM(0,0,36.100)  { speed = 40; currentPalette = PartyColors_p; northPalette(); FastLED.show();}
  FROM(0,0,44.000)  {speed = 40; currentPalette = PartyColors_p; southPalette();  FastLED.show();}
  FROM(0,0,52.000)  {speed = 40; currentPalette = PartyColors_p; inwardPalette();  FastLED.show();}
  FROM(0,1,00.000)  {currentPalette = PartyColors_p; outwardPalette();  FastLED.show();}
  FROM(0,1,08.000)   {FastLED.clear();FastLED.show();}
  AT(0,1,08.002)     {RestartPerformance(); }
  
}







  



#endif
