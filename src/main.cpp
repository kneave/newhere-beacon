#include <Arduino.h>
#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>
#include <newhere-espnow-lib.h>

Joystick_Msg joy_msg;
Power_Msg power_msg;
Position_Msg position_msg;

uint8_t remote_mac[] = {0x7C, 0xDF, 0xA1, 0x15, 0x30, 0x86};
#define ESP_CHANNEL 0

#define LED_PIN     GPIO_NUM_35
#define NUM_LEDS    8 * 32
#define BRIGHTNESS  5
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
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);

long lastJoyMsg = -1;
long lastPosMsg = -1;

volatile bool joyDataReady = false;
volatile bool posDataReady = false;
volatile bool powerDataReady = false;

bool MSG_DEBUG = false;

// Serial link to the Teensy 4.1 (UART1: TX=GPIO5, RX=GPIO16 on the QT Py)
#define TEENSY_BAUD 115200

SerialFrameReceiver teensyRx;

void pollTeensySerial() {
  while (Serial1.available()) {
    uint16_t len = teensyRx.feed(Serial1.read());
    if (len == 0) {
      continue;
    }

    if (len == sizeof(Power_Msg)) {
      memcpy(&power_msg, teensyRx.payload(), sizeof(Power_Msg));
      powerDataReady = true;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500); // power-up safety delay

  Serial1.begin(TEENSY_BAUD, SERIAL_8N1, RX, TX);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  lastColorChange = millis();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}


String inputString = "";

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
    SetColumn(dot + 16, CRGB::Black);
    FastLED.show();
  }

  if (joyDataReady) {
    joyDataReady = false;
    if (MSG_DEBUG) {
      Serial.print("Bytes received: ");
      Serial.println(sizeof(Joystick_Msg));
      Serial.print("Timestamp: ");
      Serial.println(joy_msg.timestamp);
      Serial.print("C: ");
      Serial.println(joy_msg.c);
      Serial.print("Z: ");
      Serial.println(joy_msg.z);
      Serial.print("X: ");
      Serial.println(joy_msg.x);
      Serial.print("Y: ");
      Serial.println(joy_msg.y);
      Serial.println();

      long joyInt = millis() - lastJoyMsg;
      lastJoyMsg = millis();
      if (joyInt > 0) {
        Serial.print(1000 / joyInt);
        Serial.println("Hz, Joy");
      }
    }
  }

  if (posDataReady) {
    posDataReady = false;
    if (MSG_DEBUG) {
      Serial.print("Unix Time: ");
      Serial.println(position_msg.unixtime);
      Serial.print("Lat: ");
      Serial.println(String(position_msg.lat, 7));
      Serial.print("Lon: ");
      Serial.println(String(position_msg.lon, 7));
      Serial.print("Alt: ");
      Serial.println(position_msg.alt);
      Serial.print("Acc: ");
      Serial.println(position_msg.acc);
      Serial.print("RTK: ");
      Serial.println(position_msg.rtk);
      Serial.println();
    }
  }

  if (powerDataReady) {
    powerDataReady = false;
    if (MSG_DEBUG) {
      Serial.print("Voltage: ");
      Serial.println(power_msg.voltage);
      Serial.print("Current: ");
      Serial.println(power_msg.current);
      Serial.println();
    }
  }

  pollTeensySerial();

  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      // Process the complete string here
      inputString.trim();
      Serial.println("Command received: " + inputString);
          
      if(inputString == "MAC")
      {    
        Serial.print("Board MAC Address: ");
        Serial.println(WiFi.macAddress());
      }
      else if (inputString == "MSG_DEBUG")
      {
        MSG_DEBUG = !MSG_DEBUG;
        Serial.print("MSG_DEBUG toggled to: ");
        Serial.println(MSG_DEBUG ? "ON" : "OFF");
      }
    
      inputString = ""; // Clear for next command
    } else {
      inputString += inChar;
    }
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

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (len == sizeof(Joystick_Msg)) {
    memcpy(&joy_msg, incomingData, sizeof(joy_msg));
    joyDataReady = true;
    sendSerialFrame(Serial1, &joy_msg, sizeof(joy_msg));  // forward to Teensy
  } else if (len == sizeof(Position_Msg)) {
    memcpy(&position_msg, incomingData, sizeof(position_msg));
    posDataReady = true;
    sendSerialFrame(Serial1, &position_msg, sizeof(position_msg));  // forward to Teensy
  }
}
