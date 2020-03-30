#ifndef ESP_SHD_DHT22
#define ESP_SHD_DHT22

#include "ESP_SmartHomeDevice.h"
#include "DHT.h"

#define DHTTYPE DHT22

#define TOPIC_BUFFER_LENGTH 50
#define MESSAGE_BUFFER_LENGTH 10

class ShdDht22Sensor : public ESP_SmartHomeDevice {
public:
  ShdDht22Sensor(uint8_t _pin, uint32_t _intervalS, uint8_t _dhtType);
private:
  bool handleMqttRequest(char* _topic, unsigned char* _payload, unsigned int _length);
  void timer5msHandler();
  void republish();
  void clearMessage();

  DHT * dht;
  uint8_t pin;

  uint32_t counter5ms;
  const uint32_t interval5msCount;

  char pubTopicHumidity[50], pubTopicTemperature[50];
  char message[10];
}

#endif
