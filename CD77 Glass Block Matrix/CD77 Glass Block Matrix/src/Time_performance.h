#ifndef Time_performance_h
#define Time_performance_h

/*
 
 Based on the Time Performance sketch by Mark Kriegsman of FastLED at:
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


void Performance()
{
 FROM(0,0,00.000) { threeDots.Update(); FastLED.show();}
 FROM(0,0,15.000) { paletteAnimation.setTargetPalette(CRGBPalette16(CRGB::Red)); paletteAnimation.Update(); FastLED.show(); }
 FROM(0,0,25.000) { paletteAnimation.setTargetPalette(CRGBPalette16(CRGB::Navy)); paletteAnimation.Update(); FastLED.show(); }
 FROM(0,0,35.000) { paletteAnimation.setTargetPalette(CRGBPalette16(CRGB::DarkGreen)); paletteAnimation.Update(); FastLED.show(); }
 FROM(0,0,45.000) { paletteAnimation.setTargetPalette(CRGBPalette16(CRGB::Yellow)); paletteAnimation.Update(); FastLED.show(); }
 FROM(0,0,55.000) {speed = 20; currentPalette = IceColors_p; inwardPalette();  FastLED.show();}
 FROM(0,1,05.000) {speed = 40; currentPalette = gb6_p; clockwisePalette();  FastLED.show();}
 FROM(0,1,15.000) {speed = 50; currentPalette = gb9_p;outwardPalette();  FastLED.show();}
 AT(0,1,25.000)   {RestartPerformance(); }
   
}
#endif


