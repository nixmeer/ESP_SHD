#include "ESP_SmartHomeDevice.h"
#include "pwm.h"
#define DEBUG 5

ESP_SmartHomeDevice* ESP_SmartHomeDevice::shds[MAX_SHDS];
PubSubClient ESP_SmartHomeDevice::mqttClient;
WiFiClient ESP_SmartHomeDevice::wifiClient;
uint8_t ESP_SmartHomeDevice::numberOfShds = 0;
uint32_t ESP_SmartHomeDevice::lastConnectionAttempt = 0;
uint32_t ESP_SmartHomeDevice::last5msTimer = 0;
uint32_t ESP_SmartHomeDevice::last1msTimer = 0;
uint32_t ESP_SmartHomeDevice::lastMqttLoop = 0;
char* ESP_SmartHomeDevice::name;
bool ESP_SmartHomeDevice::useMdns = false;
char* ESP_SmartHomeDevice::mqttServerAddress;
uint16_t ESP_SmartHomeDevice::port;
mqttSubscription* ESP_SmartHomeDevice::lastSubscription = NULL;
bool ESP_SmartHomeDevice::firstRun = true;
uint32_t ESP_SmartHomeDevice::pwmDutyInit[MAX_PWM_CHANNELS];
uint32_t ESP_SmartHomeDevice::ioInfo[MAX_PWM_CHANNELS][3];
uint16_t ESP_SmartHomeDevice::gammaCorrection[101] = {
  	0, 1, 4, 10, 20, 33, 51, 72,
  	98, 129, 164, 204, 250, 300, 356, 417,
  	484, 557, 635, 719, 809, 905, 1007, 1115,
  	1230, 1351, 1479, 1613, 1753, 1901, 2055, 2216,
  	2384, 2559, 2741, 2929, 3126, 3329, 3539, 3757,
  	3983, 4215, 4456, 4703, 4959, 5222, 5493, 5771,
  	6057, 6352, 6654, 6964, 7282, 7608, 7942, 8285,
  	8635, 8994, 9361, 9736, 10120, 10512, 10913, 11322,
  	11740, 12166, 12600, 13044, 13496, 13957, 14427, 14905,
  	15392, 15888, 16393, 16907, 17430, 17962, 18504, 19054,
  	19613, 20181, 20759, 21346, 21942, 22548, 23162, 23787,
  	24420, 25063, 25715, 26377, 27049, 27730, 28420, 29121,
  	29830, 30550, 31279, 32018, 32767
};
pwm* ESP_SmartHomeDevice::lastPwm = NULL;
uint8_t ESP_SmartHomeDevice::numberOfPwms = 0;

ESP_SmartHomeDevice::ESP_SmartHomeDevice(){
    if (numberOfShds < MAX_SHDS-1) {
        shds[numberOfShds] = this;
        numberOfShds++;
    } else {
        Serial.println("SHD: Too many devices!!");
    }
}

void ESP_SmartHomeDevice::init(char* _mqttServerAddress, uint16_t _port, char* _name){
    numberOfShds = 0;
    name = _name;
    useMdns = false;

    mqttServerAddress = _mqttServerAddress;
    port = _port;

    Serial.println("------------------------------------------");
    Serial.println();
}

void ESP_SmartHomeDevice::init(char *_name){
    numberOfShds = 0;
    name = _name;
    useMdns = true;

    Serial.println("------------------------------------------");
    Serial.println();
}

bool ESP_SmartHomeDevice::connectWifi(){
#if DEBUG > 1
    // Serial.print("SHD: connectWifi() was called. free cont stack: ");
    // Serial.print(ESP.getFreeContStack());
    // Serial.print(", free heap stack: ");
    // Serial.print(ESP.getFreeHeap());
    // Serial.println();
#endif
    if (WiFi.status() != WL_CONNECTED) {
        // WiFi.mode(WIFI_STA);
        WiFiManager wifiManager;
        wifiManager.setConfigPortalTimeout(90);
        wifiManager.autoConnect(name);
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.hostname(name);
            Serial.println("SHD: WiFiManager could establish a connection.");
            return true;
        } else {
            Serial.println("WiFiManager could not establish a connection. Resetting...");
            ESP.reset();
            return false;
        }
    } else {
#if DEBUG > 1
        Serial.println("SHD: WiFi connection was ok.");
#endif
        return true;
    }
}

void ESP_SmartHomeDevice::reconnectMqtt(){

    // start mDNS:
    // if (!MDNS.begin(name)) {
    //   Serial.print("MQTT: Trying to begin to mDNS service .");
    //   uint32_t mdnsMillis = millis();
    //   while (!MDNS.begin(name)) {
    //     Serial.print(".");
    //     delay(200);
    //     if (millis() - mdnsMillis > 5001) {
    //       Serial.println(" Could not begin mDNS service.");
    //       break;
    //     }
    //   }
    // }

    // find mqtt service via mdns:
    uint16_t n = MDNS.queryService("mqtt", "tcp");
    Serial.print("SHD: ");
    Serial.print(n);
    Serial.println(" mqtt services found.");

    if (n != 1) {
        return;
    }

    reconnectMqtt(MDNS.IP(0),  MDNS.port(0));
}

void ESP_SmartHomeDevice::reconnectMqtt(const char* _mqttServerAddress, uint16_t _port){
    // mqttClient.disconnect();

    mqttClient.setClient(wifiClient);
    mqttClient.setServer(_mqttServerAddress, _port);
    mqttClient.setCallback(ESP_SmartHomeDevice::mqttCallback);

    if (mqttClient.connect(name)) {
        Serial.println("MQTT: Now successfully connected to broker. ");
    } else {
        Serial.println("MQTT: 1 mqtt service found via mDNS but connecting to broker failed.");
    }

    // os_timer_setfn(&ESP_SmartHomeDevice::loopTimer, &ESP_SmartHomeDevice::loop, NULL);
    // os_timer_arm(&ESP_SmartHomeDevice::loopTimer, 5, true);
}

void ESP_SmartHomeDevice::reconnectMqtt(IPAddress _mqttServerAddress, uint16_t _port){
    // mqttClient.disconnect();

    mqttClient.setClient(wifiClient);
    mqttClient.setServer(_mqttServerAddress, _port);
    mqttClient.setCallback(ESP_SmartHomeDevice::mqttCallback);

    if (mqttClient.connect(name)) {
        Serial.println("MQTT: Now successfully connected to broker. ");
    } else {
        Serial.println("MQTT: 1 mqtt service found via mDNS but connecting to broker failed.");
    }

    // os_timer_setfn(&ESP_SmartHomeDevice::loopTimer, &ESP_SmartHomeDevice::loop, NULL);
    // os_timer_arm(&ESP_SmartHomeDevice::loopTimer, 5, true);
}

void ESP_SmartHomeDevice::mqttCallback(char* _topic, unsigned char* _payload, unsigned int _length){
#if DEBUG >= 1
    Serial.print("MQTT: callback topic: ");
    Serial.print(_topic);
    Serial.println(".");
#endif
    mqttSubscription* subscribtion = (mqttSubscription*)lastSubscription;
    while (subscribtion != NULL) {
        if (strcmp(_topic, subscribtion->topic) == 0) {
            subscribtion->subscriber->handleMqttRequest(_topic, _payload, _length);
            break;
        } else {
            subscribtion = subscribtion->next;
        }
    }
    // clear payload:
    uint16_t i = 0;
    while (_payload[i] != 0) {
        _payload[i] = 0;
        i++;
    }
}

void ESP_SmartHomeDevice::loop(){//void *pArg){
    uint32_t currentMicros = micros();


    if (firstRun) {
      firstRunFunction();
    }

    // Serial.println("TEST 1.");

    if (currentMicros - last1msTimer > 1000) {
        while (currentMicros - last1msTimer > 1000) {
            last1msTimer += 1000;
        }

        if (!mqttClient.loop()) {
#if DEBUG > 1
            Serial.print("MQTT: state = ");
            Serial.println(mqttClient.state());
            Serial.print("MQTT: loop() returned false. It has not been called since ");
            Serial.print(currentMicros - lastMqttLoop);
            Serial.println(" us.");
#endif
            reconnect();
            return;
        } else {
            lastMqttLoop = currentMicros;
        }

    }

    if(currentMicros - last5msTimer > 5000){
        while (currentMicros - last5msTimer > 5000) {
            last5msTimer += 5000;
        }

        for (size_t i = 0; i < numberOfShds; i++) {
            shds[i]->timer5msHandler();
        }
    }

}

void ESP_SmartHomeDevice::reconnect(){
    uint32_t currentMillis = millis();
    if (currentMillis - lastConnectionAttempt > TRY_RECONNECT_AFTER_MILLISECONDS) {

        // check wifi connection:
        if (connectWifi()) {

            // reconnect mqtt client:
            if (useMdns) {
                reconnectMqtt();
            } else {
                reconnectMqtt(mqttServerAddress, port);
            }

            // if reconnecting has been successfull, resubscribe to all topics and publish current states:
            if (mqttClient.connected()) {
                if (resubscribe()) {
                    Serial.println("MQTT: Successfully (re-)subscribed.");
                }
                for (size_t i = 0; i < numberOfShds; i++) {
                    shds[i]->republish();
                }
            }  else {
                Serial.print("SHD: Last connection attempt ");
                Serial.print(currentMillis - lastConnectionAttempt);
                Serial.println(" milliseconds ago.");
            }
        }

        // update lastConnectionAttempt
        while (lastConnectionAttempt < (currentMillis - TRY_RECONNECT_AFTER_MILLISECONDS)) {
            lastConnectionAttempt += TRY_RECONNECT_AFTER_MILLISECONDS;
        }
    }
}

bool ESP_SmartHomeDevice::mqttPublish(char *_topic, const char *_payload) {
    if (mqttClient.connected()) {
        mqttClient.publish(_topic, _payload);
    }
}

bool ESP_SmartHomeDevice::mqttConnected() {
    return mqttClient.connected();
}

bool ESP_SmartHomeDevice::mqttSubscribe(ESP_SmartHomeDevice *_subscriber, char *_topic) {
    mqttSubscription* tmp = new mqttSubscription;
    tmp->next = (mqttSubscription*)lastSubscription;
    tmp->subscriber = _subscriber;
    tmp->topic = _topic;
    // mqttClient.subscribe(tmp->topic, 0);
    // Serial.print("MQTT: subscribed to");
    // Serial.println(tmp->topic);

    lastSubscription = tmp;
    return true;
}

bool ESP_SmartHomeDevice::resubscribe() {
    if (!mqttClient.connected()) {
        return false;
    }

    mqttSubscription* subscribtion = (mqttSubscription*)lastSubscription;
    while(subscribtion != NULL) {
        mqttClient.subscribe(subscribtion->topic, 0);
        Serial.print("MQTT: subscribed to ");
        Serial.println(subscribtion->topic);
        subscribtion = subscribtion->next;
    }
    return true;
}

int8_t ESP_SmartHomeDevice::registerPwmPin(ESP_SmartHomeDevice* _owner, uint8_t _pin, bool _lowActive) {
  bool success = true;

  // check, if another pwm channel is available:
  if (numberOfPwms >= MAX_PWM_CHANNELS) {
    success = false;
  }

  // check, if requested pwm pin is still available:
  if (success) {
    pwm* localPwm = lastPwm;
    while (localPwm != NULL) {
      if (_pin == localPwm->pin) {
        success = false;
        break;
      }
      localPwm = localPwm->next;
    }
  }


  if (success) {
    pwm* newPwm = new pwm;
    newPwm->lowActive = _lowActive;
    newPwm->owner = _owner;
    newPwm->next = lastPwm;
    newPwm->pin = _pin;
    lastPwm = newPwm;
  }


  int8_t _pwmNumber = addIoInfo(lastPwm->pin);
  if (_pwmNumber != 1) {
    lastPwm->pwmNumber = _pwmNumber;
  } else {
    success = false;
  }

  if (success) {
    pwmDutyInit[numberOfPwms-1] = 0;
    pinMode(_pin, OUTPUT);
  }

  if (success) {
    return lastPwm->pwmNumber;
  } else {
    return -1;
  }
}

bool ESP_SmartHomeDevice::setPwmPercentage(ESP_SmartHomeDevice *_owner, uint8_t _pwmNumber, uint8_t _value) {
  bool success = true;

  pwm* localPwm = lastPwm;
  while (localPwm->pwmNumber != _pwmNumber) {
    if (localPwm->next == NULL) {
      success = false;
      break;
    }
  }

  if (success) {
    if (localPwm->owner != _owner) {
      success = false;
    }
  }

  if (success) {
    if (_value > 100 || _value < 0) {
      success = false;
    }
  }

  if (success) {
    if (!localPwm->lowActive) {
      pwm_set_duty(gammaCorrection[_value], localPwm->pwmNumber);
    } else {
      pwm_set_duty(gammaCorrection[100] - gammaCorrection[_value], localPwm->pwmNumber);
    }
    pwm_start();
  }

  return success;
}

void ESP_SmartHomeDevice::firstRunFunction() {
  firstRun = false;
  pwm_init(gammaCorrection[101], pwmDutyInit, numberOfPwms, ioInfo);
}

int8_t ESP_SmartHomeDevice::addIoInfo(uint8_t _pin){
    bool success = true;
    switch (_pin) {
      case 2:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_GPIO2_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO2;
        ioInfo[numberOfPwms][2] = 2;
        break;

      case 3:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_U0RXD_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO3;
        ioInfo[numberOfPwms][2] = 3;
        break;

      case 4:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_GPIO4_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO4;
        ioInfo[numberOfPwms][2] = 4;
        break;

      case 5:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_GPIO5_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO5;
        ioInfo[numberOfPwms][2] = 5;
        break;

      case 10:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_SD_DATA3_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO10;
        ioInfo[numberOfPwms][2] = 10;
        break;

      case 12:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_MTDI_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO12;
        ioInfo[numberOfPwms][2] = 12;
        break;

      case 13:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_MTCK_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO13;
        ioInfo[numberOfPwms][2] = 13;
        break;

      case 14:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_MTMS_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO14;
        ioInfo[numberOfPwms][2] = 14;
        break;

      default:
        success = false;
        break;
    }

    if (success) {
      numberOfPwms++;
      return numberOfPwms-1;
    } else {
      return -1;
    }
}
