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
#include "WiFiManager.h"

#ifndef MAX_SHDS
#define MAX_SHDS 8
#endif

// #define DEBUG 5

class ESP_SmartHomeDevice {
public:
  ESP_SmartHomeDevice();
  static void init(char* _name);
  static void init(char* _mqttServerAddress, uint16_t _port, char* _name);
  static void mqttCallback(char* _topic, unsigned char* _payload, unsigned int _length);
  static void loop();//void *pArg);
protected:
  virtual bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length) = 0;
  virtual void timer5msHandler() = 0;
  virtual void resubpub() = 0;
  static void reconnect();
  static void connectWifi();
  static void reconnectMqtt();
  static void reconnectMqtt(IPAddress _mqttServerAddress, uint16_t _port);
  static void reconnectMqtt(const char* _mqttServerAddress, uint16_t _port);
  static ESP_SmartHomeDevice* shds[MAX_SHDS];
  static uint8_t numberOfShds;
  static PubSubClient mqttClient;
  static WiFiClient wifiClient;
  static char* name;
  static os_timer_t loopTimer;
  static uint32_t lastConnectionAttempt;
  static uint32_t last5msTimer;
  static char * mqttServerAddress;
  static uint16_t port;
  static bool useMdns;
};

#endif
