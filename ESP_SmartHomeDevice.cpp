#include "ESP_SmartHomeDevice.h"
#define DEBUG 0

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
void* ESP_SmartHomeDevice::lastSubscription = NULL;
bool ESP_SmartHomeDevice::firstRun = true;
uint8_t ESP_SmartHomeDevice::numberOfPwmPins = 0;
uint32_t ESP_SmartHomeDevice::pwmDutyInit[MAX_PWM_CHANNELS];
uint32_t ESP_SmartHomeDevice::ioInfo[MAX_PWM_CHANNELS][3];

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
    Serial.print("SHD: connectWifi() was called. free cont stack: ");
    Serial.print(ESP.getFreeContStack());
    Serial.print(", free heap stack: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println();
#endif
    if (WiFi.status() != WL_CONNECTED) {
        WiFi.mode(WIFI_STA);
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

void ESP_SmartHomeDevice::mqttSubscribe(ESP_SmartHomeDevice *_subscriber, char *_topic) {
    mqttSubscription* tmp = new mqttSubscription;
    tmp->next = (mqttSubscription*)lastSubscription;
    tmp->subscriber = _subscriber;
    tmp->topic = _topic;
    // mqttClient.subscribe(tmp->topic, 0);
    // Serial.print("MQTT: subscribed to");
    // Serial.println(tmp->topic);

    lastSubscription = (void*)tmp;
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
