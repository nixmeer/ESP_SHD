#include "ESP_SmartHomeDevice.h"

#define DEBUG 4

class ShdPwmSingleColorLight : public ESP_SmartHomeDevice {
public:
  ShdPwmSingleColorLight(uint8_t _pin, bool _lowActive, uint8_t _millisUpdateInterval, uint16_t _flankLength);
private:
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  void timer5msHandler();
  void republish();
  void setBrightness(uint8_t _percentage);
  bool lowActive;
  uint8_t pin, pwmNumber;
  uint8_t setPoint;
  uint8_t delta;
  int16_t currentBrightness;
  uint8_t lastBrightnessGreaterZero;
  uint32_t millisLastUpdate;
  uint32_t millisUpdateInterval;
  bool flankOver;
  char pubTopicBrightness[70], pubTopicState[70], subTopicState[70], subTopicBrightness[70];
};
