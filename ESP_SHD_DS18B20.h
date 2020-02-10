#ifndef ESP_SHD_DS18B20_SENSOR
#define ESP_SHD_DS18B20_SENSOR

#define PUBLISH_TIME_IN_MS 500

#include "PubSubClient.h"
#include "ESP_SmartHomeDevice.h"
#include <OneWire.h>
#include <DallasTemperature.h>

class ShdDs18b20Sensor : public ESP_SmartHomeDevice {
public:
  ShdDs18b20Sensor(uint8_t _pin);
private:
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  char pubTopic[50];
  char message[10];
  void publishTemperature();
  void timer5msHandler();
  uint16_t timerCounter;
  void republish();
  OneWire oneWire;
  DallasTemperature DS18B20;
};

#endif
