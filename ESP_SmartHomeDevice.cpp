#include "ESP_SmartHomeDevice.h"

ESP_SmartHomeDevice* ESP_SmartHomeDevice::shds[MAX_SHDS];
PubSubClient ESP_SmartHomeDevice::mqttClient;
WiFiClient ESP_SmartHomeDevice::wifiClient;
int ESP_SmartHomeDevice::numberOfShds = 0;
char* ESP_SmartHomeDevice::title;
char* ESP_SmartHomeDevice::location;

ESP_SmartHomeDevice::ESP_SmartHomeDevice(){
  if (numberOfShds < MAX_SHDS-1) {
    shds[numberOfShds] = this;
    numberOfShds++;
  }
  Serial.println();
  Serial.println("SHD: New device registered.");
}

void ESP_SmartHomeDevice::init(char* _address, uint16_t _port, char* _location, char* _title){

  numberOfShds = 0;

  location = _location;
  title = _title;

  mqttClient.setClient(wifiClient);
  mqttClient.setServer(_address, _port);
  mqttClient.setCallback(ESP_SmartHomeDevice::mqttCallback);
}

void ESP_SmartHomeDevice::mqttCallback(char* _topic, byte* _payload, unsigned int _length){
  for (uint8_t i = 0; i < numberOfShds; i++) {
    if(shds[i]->handleMqttRequest(_topic, _payload, _length)){
       break;
    }
  }
}
