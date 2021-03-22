#include "ESP_SmartHomeDevice.h"
#include <SoftwareSerial.h>

#define DEBUG 5

#define MQTT_TOPIC_SIZE 70
#define MQTT_PAYLOAD_BUFFER_SIZE 10

enum contactSensorType {
  CS_NORMAL = 0,
  CS_LSC_DOOR_SENROR = 1
};

enum csStatus {
  CS_OPENED, CS_CLOSED, CS_ERROR
};

class ShdContactSensor : public ESP_SmartHomeDevice {
public:
  ShdContactSensor(contactSensorType _type, bool _sleepAfterPublish, bool _getBatteryStatus);
  ShdContactSensor(contactSensorType _type, uint8_t _pin, bool _lowAcive, bool _sleepAfterPublish, uint16_t _intervalMillis, bool _getBatteryStatus);
private:
  static uint8_t contactSensorCount;

  void timer5msHandler();
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  void republish();

  char payloadBuffer[50];

  contactSensorType csType;
  char pubTopicState[MQTT_TOPIC_SIZE];
  csStatus lastStatus;
  bool sleepAfterPublish;
  uint8_t csNumber;
  bool getBatteryStatus;
  char pubTopicBattery[MQTT_TOPIC_SIZE];

  uint8_t pin;
  bool lowActive;
  uint16_t updateInterval, updateCounter;

  void getStatus();
  csStatus getStatusNormal();
  csStatus getStatusLsc();

  void lscCommunication(byte _cmd, byte _len, byte _value, uint8_t* _reply, uint8_t _maxReplySize, uint8_t* _replySize);

  SoftwareSerial swSerial; // REMOVE!
};
