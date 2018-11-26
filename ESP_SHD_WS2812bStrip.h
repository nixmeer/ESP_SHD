#ifndef ESP_SHD_LEDSTRIP
#define ESP_SHD_LEDSTRIP

#define FASTLED_ESP8266_RAW_PIN_ORDER

#include "ESP_SmartHomeDevice.h"
#include "PubSubClient.h"
#include "FastLED.h"

#define MAX_NUM_OF_LEDS 144
#define MAX_NUM_OF_SECTIONS 5

#define DATA_PIN 4

enum ignitionDirection { IGNITION_FORWARD, IGNITION_BACKWARD, IGNITION_BOTH };

class ShdWs2812bStrip : public ESP_SmartHomeDevice {
public:
  static void initStrip(int _numberOfLeds);

  ShdWs2812bStrip(uint16_t _firstLed, uint16_t _lastLed, uint16_t ignitionPoint, ignitionFunction _ignitionDirection, uint8_t _hopsPerShow, uint8_t _flankLength, const char * _stripName);
private:
  // static variables and functions for the entire strip:
  static void show();
  static CRGB leds[MAX_NUM_OF_LEDS];
  static uint16_t numberOfLeds;
  static uint16_t millisLastStripUpdate;
  static uint16_t millisStripUpdateInterval;
  static uint8_t numberOfSections;
  static ShdWs2812bStrip * sections[MAX_NUM_OF_SECTIONS];
  static bool correctlyInitialized;

  // variables for each section
  uint8_t firstLed, lastLed;
  uint16_t setPoint[3];
  uint16_t shownValue[3];
  int16_t delta[3];
  uint8_t flankLength;
  uint8_t hopsPerShow;
  uint16_t updateInterval;
  bool directionInverted;
  bool igniting;
  uint16_t ignitionCounter;
  uint16_t ignitionPoint;
  char[50] subTopic;
  char[50] pubTopic;


  // functions for each section:
  void setNewColor();
  void timer5msHandler();
  void updateCRGBs();

  void (*ignitionFunction)();
  void igniteBoth();
  void igniteSingleForward();
  void igniteSingleBackward();
  void fillLedWithNewColor(uint16_t _led, uint16_t _neighbor);
};

#endif
