#include "ESP_SmartHomeDevice.h"

#define DEBUG_LEVEL 5

#define MQTT_TOPIC_SIZE 70
#define MQTT_PAYLOAD_BUFFER_SIZE 10
#define T_WARMEST 500
#define T_COOLEST 140

class ShdPwmTemperatureLight : public ESP_SmartHomeDevice {
public:
  ShdPwmTemperatureLight(uint8_t _coldPin, uint8_t _warmPin, bool _lowActive, uint8_t _millisUpdateInterval, uint16_t _flankLength);
private:
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  void timer5msHandler();
  void republish();

  void setBrightness(uint8_t _percentage);
  void setTemperature(uint16_t _temperature);
  void setStatus(bool _status);

  void updateVariables();

  bool lowActive;
  uint8_t coldPin, coldPwmNumber;
  uint8_t warmPin, warmPwmNumber;
  uint8_t updateCounter, updateInterval;
  uint16_t flankLength;
  float deltaCold, deltaWarm;
  float setPointCold, currentCold, setPointWarm, currentWarm;
  uint8_t setPointBrightness, lastSetPointBrightnessGreaterZero;
  uint16_t setPointTemperature;
  char pubTopicState[MQTT_TOPIC_SIZE], pubTopicBrightness[MQTT_TOPIC_SIZE], pubTopicTemperature[MQTT_TOPIC_SIZE];
  char subTopicState[MQTT_TOPIC_SIZE], subTopicBrightness[MQTT_TOPIC_SIZE], subTopicTemperature[MQTT_TOPIC_SIZE];
  uint8_t pwmTemperatureNumber;
  static uint8_t pwmTemperatureCount;
};
