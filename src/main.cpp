#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN     15
#define NUM_LEDS    56
#define BRIGHTNESS  255
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

#define UPDATES_PER_SECOND 4

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

int WrapLedNumber(int, int);

void setup() {
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
}

void loop() {

    for(int dot = 0; dot < NUM_LEDS; dot++) {
        leds[dot] = CRGB::Orange;
    }
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);

    
    for(int dot = 0; dot < NUM_LEDS; dot++) {
        leds[dot] = CRGB::Black;
    }
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}

int WrapLedNumber(int input, int offset)
{
  if(input + offset >= NUM_LEDS){
    return input + offset - NUM_LEDS;
  } 
  else{
    return input;
  }
}