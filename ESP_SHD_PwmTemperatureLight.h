#include "ESP_SmartHomeDevice.h"

#define DEBUG_LEVEL 5

#define MQTT_TOPIC_SIZE 70
#define MQTT_PAYLOAD_BUFFER_SIZE 10
#define T_WARMEST 500
#define T_COOLEST 140

enum temperaturePwmMode {
  MODE_WARM_PWM_COLD_PWM = 0,
  MODE_BRIGHTNESS_PWM_TEMP_PWM = 1
};

class ShdPwmTemperatureLight : public ESP_SmartHomeDevice {
public:
  ShdPwmTemperatureLight(temperaturePwmMode _mode, uint8_t _pin1, uint8_t _pin2, bool _lowActive1, bool _lowActive2, uint8_t _millisUpdateInterval, uint16_t _flankLength);
private:
  temperaturePwmMode mode;
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  void timer5msHandler();
  void republish();

  void setBrightness(uint8_t _percentage);
  void setTemperature(uint16_t _temperature);
  void setStatus(bool _status);

  void updateVariables();

  uint8_t updateCounter, updateInterval;
  uint16_t flankLength;
  uint8_t setPointBrightness, lastSetPointBrightnessGreaterZero;
  uint16_t setPointTemperature;

  uint8_t coldPin, coldPwmNumber, warmPin, warmPwmNumber;
  float setPointCold, currentCold, setPointWarm, currentWarm;
  float deltaCold, deltaWarm;

  uint8_t brightnessPin, brightnessPwmNumber, tempPin, tempPwmNumber;
  float setPointBrightnessPwm, currentBrightness, setPointTempPwm, currentTemp;
  float deltaBrightness, deltaTemp;

  char pubTopicState[MQTT_TOPIC_SIZE], pubTopicBrightness[MQTT_TOPIC_SIZE], pubTopicTemperature[MQTT_TOPIC_SIZE];
  char subTopicState[MQTT_TOPIC_SIZE], subTopicBrightness[MQTT_TOPIC_SIZE], subTopicTemperature[MQTT_TOPIC_SIZE];
  uint8_t pwmTemperatureNumber;
  static uint8_t pwmTemperatureCount;
};
