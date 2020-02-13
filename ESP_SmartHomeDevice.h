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
#define MAX_SHDS 10
#endif

#define DEBUG 0

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
    virtual void republish() = 0;
    static bool mqttPublish(char* _topic, const char* _payload);
    static bool mqttConnected();
    static void mqttSubscribe(ESP_SmartHomeDevice* _subscriber, char* _topic);
    static ESP_SmartHomeDevice* shds[MAX_SHDS];
    static char* name;
    static uint8_t registerPwm(uint8_t _pin);

private:
    static PubSubClient mqttClient;
    static uint8_t numberOfShds;
    static WiFiClient wifiClient;
    static uint32_t lastConnectionAttempt;
    static uint32_t last5msTimer;
    static uint32_t last1msTimer;
    static char * mqttServerAddress;
    static uint16_t port;
    static bool useMdns;
    static void* lastSubscription;
    static uint32_t lastMqttLoop;
    static void reconnect();
    static bool connectWifi();
    static void reconnectMqtt();
    static void reconnectMqtt(IPAddress _mqttServerAddress, uint16_t _port);
    static void reconnectMqtt(const char* _mqttServerAddress, uint16_t _port);
    static bool resubscribe();
    static bool firstRun;
    static uint8_t numberOfPwmPins;
    static uint32_t pwmDutyInit[MAX_PWM_CHANNELS];
    static uint32_t ioInfo[MAX_PWM_CHANNELS][3];
};

struct mqttSubscription {
    mqttSubscription* next;
    ESP_SmartHomeDevice* subscriber;
    char* topic;
};

#endif
