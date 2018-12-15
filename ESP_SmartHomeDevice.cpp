#include "ESP_SmartHomeDevice.h"

ESP_SmartHomeDevice* ESP_SmartHomeDevice::shds[MAX_SHDS];
PubSubClient ESP_SmartHomeDevice::mqttClient;
WiFiClient ESP_SmartHomeDevice::wifiClient;
int ESP_SmartHomeDevice::numberOfShds = 0;
int ESP_SmartHomeDevice::lastConnectionAttempt = 0;
int ESP_SmartHomeDevice::last5msTimer = 0;
char* ESP_SmartHomeDevice::name;
os_timer_t ESP_SmartHomeDevice::loopTimer;


ESP_SmartHomeDevice::ESP_SmartHomeDevice(){
  if (numberOfShds < MAX_SHDS-1) {
    shds[numberOfShds] = this;
    numberOfShds++;
  }
  Serial.println();
  Serial.println("SHD: New device registered. ");
}

void ESP_SmartHomeDevice::init(const char* _mqttServerAddress, uint16_t _port, char* _name){ // TODO: Char * als const?

  numberOfShds = 0;

  name = _name;

  mqttClient.setClient(wifiClient);
  mqttClient.setServer(_mqttServerAddress, _port);
  mqttClient.setCallback(ESP_SmartHomeDevice::mqttCallback);

  lastConnectionAttempt = millis();
  if (mqttClient.connect(name)) {
    Serial.println("Now successfully connected to MQTT server. ");
  } else {
    Serial.print("Connecting to MQTT server failed.");
  }

  // os_timer_setfn(&ESP_SmartHomeDevice::loopTimer, &ESP_SmartHomeDevice::loop, NULL);
  // os_timer_arm(&ESP_SmartHomeDevice::loopTimer, 5, true);
}

void ESP_SmartHomeDevice::init(char* _name){

  if (!MDNS.begin(_name)) {
    Serial.println("Trying to connect to mDNS.");
    while (!MDNS.begin(_name)) {
      Serial.print(".");
      delay(200);
    }
  }

  int n = MDNS.queryService("mqtt", "tcp");
  if(n == 1){
    ESP_SmartHomeDevice::init(MDNS.hostname(0).c_str(), MDNS.port(0), _name);
  } else if (n == 0) {
    Serial.println("0 mqtt broker found. Resetting this device now.");
    ESP.reset();
  } else {
    Serial.print(n);
    Serial.println(" mqtt broker found, didn't know which one to choose. Resetting this device now.");
    ESP.reset();
  }

}

void ESP_SmartHomeDevice::mqttCallback(char* _topic, unsigned char* _payload, unsigned int _length){
  #if DEBUG >= 1
  Serial.print("MQTT: callback topic: ");
  Serial.print(_topic);
  Serial.println(".");
  #endif
  for (uint8_t i = 0; i < numberOfShds; i++) {
    if(shds[i]->handleMqttRequest(_topic, _payload, _length)){
       break;
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

  // TODO: (re-)connecting doesn't work here, even though it's the same code as in init():
  if (!mqttClient.loop()) {
    reconnect();
  }

  if(micros() - last5msTimer > 5000){
    while (micros() - last5msTimer > 5000) {
      last5msTimer += 5000;
    }
    for (size_t i = 0; i < numberOfShds; i++) {
      shds[i]->timer5msHandler();
    }
  }

}

void ESP_SmartHomeDevice::reconnect(){
  int currentMillis = millis();
  if (currentMillis - lastConnectionAttempt > TRY_RECONNECT_AFTER_MILLISECONDS) {
    mqttClient.setServer("192.168.178.37", 1883);
    if (mqttClient.connect(name)) {
      Serial.println("Now successfully connected to MQTT server. ");
      for (size_t i = 0; i < numberOfShds; i++) {
        shds[i]->resubscribe();
      }
    }
    else {
      Serial.print("Connecting to MQTT server failed. Last connection attempt ");
      Serial.print(currentMillis - lastConnectionAttempt);
      Serial.println(" milliseconds ago.");
    }
    while (currentMillis - lastConnectionAttempt > TRY_RECONNECT_AFTER_MILLISECONDS) {
      lastConnectionAttempt += TRY_RECONNECT_AFTER_MILLISECONDS;
    }
  }
}
