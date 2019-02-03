#ifndef ESP_SHD_MOTION_SENSOR
#define ESP_SHD_MOTION_SENSOR

#include "PubSubClient.h"
#include "ESP_SmartHomeDevice.h"

class ShdMotionSensor : public ESP_SmartHomeDevice {
public:
  ShdMotionSensor(uint8_t _pin);
private:
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  uint8_t pin;
  bool motionDeteced, motionSensorStatus;
  void pinChange();
  char pubTopic[50];
  void timer5msHandler();
  void resubpub();
};

#endif
