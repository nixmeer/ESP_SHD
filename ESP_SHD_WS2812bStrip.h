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

  ShdWs2812bStrip(uint16_t _firstLed, uint16_t _lastLed, uint16_t ignitionPoint, ignitionDirection _ignitionDirection, uint8_t _hopsPerShow, uint8_t _flankLength, const char * _stripName);
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
  static uint16_t counter5ms;

  // variables for each section
  uint8_t sectionNumber;
  uint8_t firstLed, lastLed;
  uint16_t setPoint[3];
  uint16_t shownValue[3], savedValue[3];
  int16_t delta[3];
  uint8_t flankLength;
  uint8_t hopsPerShow;
  uint16_t updateInterval;
  bool directionInverted;
  bool igniting;
  uint16_t ignitionCounter;
  uint16_t ignitionPoint;
  char subTopicColor[50], pubTopicColor[50], subTopicState[50], pubTopicState[50], pubTopicBrightness[50], payloadBuffer[50];


  // functions for each section:
  bool handleMqttRequest(char* _topic, byte* _payload, unsigned int _length);
  void setNewColor(uint8_t _newRed, uint8_t _newGreen, uint8_t _newBlue);
  void timer5msHandler();
  void updateCRGBs();

  void (ShdWs2812bStrip::*ignitionFunction)();
  void igniteBoth();
  void igniteSingleForward();
  void igniteSingleBackward();
  bool fillLedWithNewColor(uint16_t _ledIndex);
  void clearPayloadBuffer();
};

#endif
