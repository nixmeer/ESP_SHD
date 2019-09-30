#ifndef ESP_SHD_SPRINKLER_h
#define ESP_SHD_SPRINKLER_h

#include "ESP_SmartHomeDevice.h"
#include "PubSubClient.h"

#define DEBUG 0

class ShdSprinkler : public ESP_SmartHomeDevice {
public:
  ShdSprinkler(uint8_t _pin, bool _lowActive);
  void toggle();
private:
  bool handleMqttRequest(char* _topic, byte* _payload, uint16_t _length);
  void republish();
  void subscribe();
  void timer5msHandler();

  static uint8_t sprinklerCount;
  uint8_t sprinklerNumber;

  // topics named from homebridge's perspective:
  char subTopicSetActive[50];
  char pubTopicGetActive[50];
  char pubTopicGetInUse[50];
  char subTopicSetDuration[50];
  char pubTopicGetDuration[50];
  char pubTopicGetRemainingDuration[60];

  bool targetStatus;
  bool currentStatus;

  uint32_t durationTicks;
  uint32_t tickCounter;
  uint32_t publishTickCounter;

  uint8_t pin;
  bool lowActive;
  void setOutput(bool _target);

  char buffer[10];
};

#endif
