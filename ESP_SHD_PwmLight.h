#include "ESP_SmartHomeDevice.h"

#define MAX_PWM_CHANNELS 5

class Shd_PwmLight : public ESP_SmartHomeDevice {
public:
  Shd_PwmLight(uint8_t _pin, bool _lowActive, uint8_t _millisUpdateInterval, uint16_t _flankLength);
private:
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  void timer5msHandler();
  void resubscribe();
  void setBrightness(uint8_t _percentage);
  bool addIoInfo();
  bool lowActive;
  uint8_t pin, pwmNumber;
  uint8_t setPoint;
  uint8_t delta;
  int16_t currentBrightness;
  uint8_t lastBrightnessGreaterZero;
  uint32_t millisLastUpdate;
  uint32_t millisUpdateInterval;
  bool flankOver;
  char pubTopicBrightness[60], pubTopicState[60], subTopicState[60], subTopicBrightness[60];

  static bool firstRun;
  static uint8_t numberOfPwmPins;
  static uint32_t pwmDutyInit[MAX_PWM_CHANNELS];
  static uint32_t ioInfo[MAX_PWM_CHANNELS][3];

  static uint32_t gammaCorrection[101];
};
