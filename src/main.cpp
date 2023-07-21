#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN     15
#define NUM_LEDS    120
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

CRGB primaryColor = CRGB::OrangeRed;
CRGB secondaryColor = CRGB::OrangeRed;

long colorChangeInterval = 5000;
long lastColorChange;
int colorCombos = 5;
int activeColorCombo = 0;

#define UPDATES_PER_SECOND 120

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

int WrapLedNumber(int);
int TranslateLedNumber(int);
void SetColumn(int dot, CRGB color);
void RotateColors();

void setup() {
    Serial.begin(9600);
    delay( 500 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    lastColorChange = millis();
}

void loop() {
  for(int dot = 0; dot < 30; dot++) 
  {
    SetColumn(dot, primaryColor);
    SetColumn(dot + 1, primaryColor);
    SetColumn(dot + 2, primaryColor);
    SetColumn(dot + 3, primaryColor);
    SetColumn(dot + 4, primaryColor);

    SetColumn(dot + 16, secondaryColor);
    SetColumn(dot + 17, secondaryColor);
    SetColumn(dot + 18, secondaryColor);
    SetColumn(dot + 19, secondaryColor);
    SetColumn(dot + 20, secondaryColor);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);

    SetColumn(dot, CRGB::Black);
    // SetColumn(dot + 1, CRGB::Black);
    // SetColumn(dot + 2, CRGB::Black);

    SetColumn(dot + 16, CRGB::Black);
    // SetColumn(dot + 17, CRGB::Black);
    // SetColumn(dot + 18, CRGB::Black);
    FastLED.show();
  }
}

void RotateColors()
{
  if(millis() - lastColorChange > colorChangeInterval)
  {
    switch (activeColorCombo)
    {
    case 0:
      primaryColor = CRGB::OrangeRed;
      secondaryColor = CRGB::OrangeRed;
      break;
    case 1:
      primaryColor = CRGB::OrangeRed;
      secondaryColor = CRGB::Green;
      break;
    case 2:
      primaryColor = CRGB::OrangeRed;
      secondaryColor = CRGB::Purple;
      break;
    case 3:
      primaryColor = CRGB::Red;
      secondaryColor = CRGB::Blue;
      break;
    case 4:
      primaryColor = CRGB::Green;
      secondaryColor = CRGB::Purple;
      break;    
    }
    activeColorCombo++;
    activeColorCombo = activeColorCombo > colorCombos ? 0 : activeColorCombo;
    lastColorChange = millis();
    Serial.print("Color changed to set: ");
    Serial.println(activeColorCombo);
  }
}

void SetColumn(int dot, CRGB color)
{
  leds[WrapLedNumber(TranslateLedNumber(dot))] = color;
  leds[WrapLedNumber(TranslateLedNumber(dot + 30))] = color;
  leds[WrapLedNumber(TranslateLedNumber(dot + 60))] = color;
  leds[WrapLedNumber(TranslateLedNumber(dot + 90))] = color;
  // FastLED.show();
}

int WrapLedNumber(int input)
{
  if(input >= NUM_LEDS){
    return input - NUM_LEDS;
  } 
  else{
    return input;
  }
}

int TranslateLedNumber(int input)
{
  auto output = input;
  // We have four rows of LEDs, they switch direction for rows 2 and 4
  //  0..29, 30..59, 60..89, 90..119
  if((input >= 30) & (input <= 59))
  {
    output = 59 - (input - 30);
  }

  if((input >= 90) & (input <= 119))
  {
    output = 119 - (input - 90);
  }

  return output;
}