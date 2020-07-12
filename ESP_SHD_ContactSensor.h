#include "ESP_SmartHomeDevice.h"

#define MQTT_TOPIC_SIZE 70
#define MQTT_PAYLOAD_BUFFER_SIZE 10

enum contactSensorType {
  CS_NORMAL = 0,
  CS_LSC_DOOR_SENROR = 1
};

enum csStatus {
  OPENED, CLOSED, ERROR
}

class ShdContactSensor : public ESP_SmartHomeDevice {
public:
  ShdContactSensor(contactSensorType _type, bool _sleepAfterPublish);
  ShdContactSensor(contactSensorType _type, uint8_t _pin, bool _lowAcive, bool _sleepAfterPublish, uint16_t _intervalMillis);
private:
  static uint8_t contactSensorCount;

  void timer5msHandler();
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  void republish();

  doorSensorType csType;
  char pubTopicState[MQTT_TOPIC_SIZE];
  csStatus lastStatus;
  bool sleepAfterPublish;

  uint8_t pin;
  bool lowActive;
  uint16_t updateInterval, updateCounter;

  csStatus getStatusNormal();
  csStatus getStatusLsc();
}
