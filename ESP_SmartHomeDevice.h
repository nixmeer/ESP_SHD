#ifndef ESP_SMART_HOME_DEVICE_H
#define ESP_SMART_HOME_DEVICE_H

#define TRY_RECONNECT_AFTER_MILLISECONDS 10000

extern "C" {
  #include "user_interface.h"
}
#include "PubSubClient.h"
#include <ESP8266WiFi.h>
#include "WiFiClient.h"
#include <ESP8266mDNS.h>

#ifndef MAX_SHDS
#define MAX_SHDS 5
#endif


class ESP_SmartHomeDevice {
public:
  ESP_SmartHomeDevice();
  static void init(char* _name);
  static void init(const char* _mqttServerAddress, uint16_t _port, char* _name);
  static void mqttCallback(char* _topic, unsigned char* _payload, unsigned int _length);
  static void loop();//void *pArg);
protected:
  virtual bool handleMqttRequest(char* _topic, unsigned char* _payload, unsigned int _length) = 0;
  virtual void timer5msHandler() = 0;
  static ESP_SmartHomeDevice* shds[MAX_SHDS];
  static int numberOfShds;
  static PubSubClient mqttClient;
  static WiFiClient wifiClient;
  static char* name;
  static os_timer_t loopTimer;
  static int lastConnectionAttempt;
  static int last5msTimer;
  static void reconnect();
};

#endif
