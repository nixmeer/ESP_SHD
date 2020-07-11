#ifndef ESP_SMART_HOME_DEVICE_H
#define ESP_SMART_HOME_DEVICE_H

#define TRY_RECONNECT_AFTER_MILLISECONDS 10000

#define MAX_PWM_CHANNELS 10

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

struct pwm;
struct mqttSubscription;

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
    static bool mqttSubscribe(ESP_SmartHomeDevice* _subscriber, char* _topic);
    static ESP_SmartHomeDevice* shds[MAX_SHDS];
    static char* name;
    static int8_t registerPwmPin(ESP_SmartHomeDevice* _owner, uint8_t _pin, bool _lowActive);
    static bool setPwmPermill(ESP_SmartHomeDevice* _owner, uint8_t _pwmNumber, uint16_t _value);

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
    static mqttSubscription* lastSubscription;
    static uint32_t lastMqttLoop;
    static void reconnect();
    static bool connectWifi();
    static void reconnectMqtt();
    static void reconnectMqtt(IPAddress _mqttServerAddress, uint16_t _port);
    static void reconnectMqtt(const char* _mqttServerAddress, uint16_t _port);
    static bool resubscribe();

    static pwm* lastPwm;
    static uint32_t pwmDutyInit[MAX_PWM_CHANNELS];
    static uint32_t ioInfo[MAX_PWM_CHANNELS][3];
    static uint16_t gammaCorrection[1001];
    static uint8_t numberOfPwms;
    static bool firstRun;
    static void firstRunFunction();
    static int8_t addIoInfo(uint8_t _pin);
    static bool setPwmDuty(ESP_SmartHomeDevice* _owner, int8_t _pwmNumber, uint8_t _value);
};

struct mqttSubscription {
    mqttSubscription* next;
    ESP_SmartHomeDevice* subscriber;
    char* topic;
};

struct pwm {
public:
  pwm* next;
  ESP_SmartHomeDevice* owner;
  uint8_t pin, pwmNumber;
  bool lowActive;
};

#endif
