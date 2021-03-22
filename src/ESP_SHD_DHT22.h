#ifndef ESP_SHD_DHT22
#define ESP_SHD_DHT22

#include "ESP_SmartHomeDevice.h"
#include "DHT.h"

#define DHTTYPE DHT22

#define TOPIC_BUFFER_LENGTH 100
#define MESSAGE_BUFFER_LENGTH 10

class ShdDht22Sensor : public ESP_SmartHomeDevice {
public:
  ShdDht22Sensor(uint8_t _pin, uint32_t _intervalS, uint8_t _dhtType);
private:
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  void timer5msHandler();
  void republish();
  void clearMessage();

  DHT * dht;
  uint8_t pin;

  uint32_t counter5ms;
  const uint32_t interval5msCount;

  char pubTopicHumidity[TOPIC_BUFFER_LENGTH], pubTopicTemperature[TOPIC_BUFFER_LENGTH];
  char message[MESSAGE_BUFFER_LENGTH];
};

#endif
