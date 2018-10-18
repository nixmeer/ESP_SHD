#include "ESP_SmartHomeDevice.h"

ESP_SmartHomeDevice* ESP_SmartHomeDevice::shds[MAX_SHDS];
PubSubClient ESP_SmartHomeDevice::mqttClient;
WiFiClient ESP_SmartHomeDevice::wifiClient;
int ESP_SmartHomeDevice::numberOfShds = 0;
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

void ESP_SmartHomeDevice::init(char* _address, uint16_t _port, char* _name){

  numberOfShds = 0;

  name = _name;

  mqttClient.setClient(wifiClient);
  mqttClient.setServer(_address, _port);
  mqttClient.setCallback(ESP_SmartHomeDevice::mqttCallback);

  if (mqttClient.connect(name)) {
    Serial.println("Now successfully connected to MQTT server. ");
  } else {
    Serial.println("Connecting to MQTT server failed. ");
  }

  os_timer_setfn(&ESP_SmartHomeDevice::loopTimer, &ESP_SmartHomeDevice::loop, NULL);
  os_timer_arm(&ESP_SmartHomeDevice::loopTimer, 500, true);
}

void ESP_SmartHomeDevice::mqttCallback(char* _topic, byte* _payload, unsigned int _length){
  for (uint8_t i = 0; i < numberOfShds; i++) {
    if(shds[i]->handleMqttRequest(_topic, _payload, _length)){
       break;
    }
  }
}

void ESP_SmartHomeDevice::loop(void *pArg){

  // TODO: (re-)connecting doesn't work here, even though it's the same code as in init():
  if (!mqttClient.loop()) {
    if (mqttClient.connect(name)) {
      Serial.println("Now successfully connected to MQTT server. ");
    } else {
      Serial.println("Connecting to MQTT server failed. ");
    }
    Test;
  }
}
