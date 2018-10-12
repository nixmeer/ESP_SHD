#ifndef ESP_SMART_HOME_DEVICE_H
#define ESP_SMART_HOME_DEVICE_H

#include "PubSubClient.h"
#include <ESP8266WiFi.h>
#include "WiFiClient.h"

#ifndef MAX_SHDS
#define MAX_SHDS 5
#endif


class ESP_SmartHomeDevice {
public:
  ESP_SmartHomeDevice();
  static void init(char* _mqttServerAddress, uint16_t _port, char* _location, char* title);
  static void mqttCallback(char* _topic, byte* _payload, unsigned int _length);
  virtual bool handleMqttRequest(char* _topic, byte* _payload, unsigned int _length) = 0;
protected:
  static ESP_SmartHomeDevice* shds[MAX_SHDS];
  static int numberOfShds;
  static PubSubClient mqttClient;
  static WiFiClient wifiClient;
  static char* location;
  static char* title;
};

#endif
