#ifndef ESP_SHD_LEDSTRIP
#define ESP_SHD_LEDSTRIP

#define FASTLED_ESP8266_RAW_PIN_ORDER

#include "ESP_SmartHomeDevice.h"
#include "PubSubClient.h"
#include "FastLED.h"

#define MAX_NUM_OF_LEDS 144
#define MAX_NUM_OF_SECTIONS 5

#define DATA_PIN 4

enum ignitionDirection { IGNITION_SINGLE_FORWARD, IGNITION_SINGLE_BACKWARD, IGNITION_BOTH_FORWARD, IGNITION_BOTH_BACKWARD };

class ShdWs2812bStrip : public ESP_SmartHomeDevice {

public:
  static void initStrip(uint16_t _numberOfLeds, uint16_t _updateInterval);

  ShdWs2812bStrip(uint16_t _firstLed, uint16_t _lastLed, uint16_t _ignitionPoint, ignitionDirection _ignitionDirection, uint8_t _hopsPerShow, uint8_t _flankLength);

private:
  // static functions and variables for the entire strip:
  static void show();
  static CRGB leds[MAX_NUM_OF_LEDS];
  static uint16_t numberOfLeds;
  static uint16_t millisLastStripUpdate;
  static uint16_t millisStripUpdateInterval;
  static uint8_t numberOfSections;
  static ShdWs2812bStrip * sections[MAX_NUM_OF_SECTIONS];
  static bool correctlyInitialized;
  static uint16_t counter5ms;
  static uint8_t gammaCorrection[256];

  // variables for each section
  uint8_t sectionNumber;
  uint16_t sectionLength;
  uint8_t firstLed, lastLed;
  uint16_t setPoint[3], shownValue[3], savedValue[3];
  int16_t delta[3];
  uint8_t flankLength;
  uint8_t hopsPerShow;
  uint16_t updateInterval;
  bool directionInverted;
  bool directionTmpInverted;
  bool igniting;
  uint16_t ignitionCounter;
  uint16_t ignitionPoint;
  char subTopicColor[50], pubTopicColor[50], subTopicState[50], pubTopicState[50], pubTopicBrightness[50], payloadBuffer[50];
  ignitionDirection direction;

  // functions for each section:
  bool handleMqttRequest(char* _topic, byte* _payload, uint16_t _length);
  void setNewColor(uint8_t _newRed, uint8_t _newGreen, uint8_t _newBlue);
  void timer5msHandler();
  void updateCRGBs();
  //void (ShdWs2812bStrip::*ignitionFunction)();
  void callIgnitionFunction();
  void igniteBothDir();
  void igniteSingleDir();
  bool fillLedWithNewColor(uint16_t _ledIndex);
  bool fillLedWithNewColor(uint16_t _ledIndex1, uint16_t _ledIndex2);
  void clearPayloadBuffer();
  void resubscribe();
  void changeStatus();
};

#endif
