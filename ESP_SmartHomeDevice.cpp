#include "ESP_SmartHomeDevice.h"

ESP_SmartHomeDevice* ESP_SmartHomeDevice::shds[MAX_SHDS];
PubSubClient ESP_SmartHomeDevice::mqttClient;
WiFiClient ESP_SmartHomeDevice::wifiClient;
int ESP_SmartHomeDevice::numberOfShds = 0;
char* ESP_SmartHomeDevice::title;
char* ESP_SmartHomeDevice::location;
os_timer_t ESP_SmartHomeDevice::loopTimer;

ESP_SmartHomeDevice::ESP_SmartHomeDevice(){
  if (numberOfShds < MAX_SHDS-1) {
    shds[numberOfShds] = this;
    numberOfShds++;
  }
  Serial.println();
  Serial.println("SHD: New device registered. ");
}

void ESP_SmartHomeDevice::init(char* _address, uint16_t _port, char* _location, char* _title){

  numberOfShds = 0;

  location = _location;
  title = _title;

  IPAddress serverIp(192, 168, 178, 31);
  mqttClient.setClient(wifiClient);
  mqttClient.setServer("spieglein", 1883);
  mqttClient.setCallback(ESP_SmartHomeDevice::mqttCallback);

  os_timer_setfn(&ESP_SmartHomeDevice::loopTimer, &ESP_SmartHomeDevice::loop, NULL);
  os_timer_arm(&ESP_SmartHomeDevice::loopTimer, 1000, true);
}

void ESP_SmartHomeDevice::mqttCallback(char* _topic, byte* _payload, unsigned int _length){
  for (uint8_t i = 0; i < numberOfShds; i++) {
    if(shds[i]->handleMqttRequest(_topic, _payload, _length)){
       break;
    }
  }
}

void ESP_SmartHomeDevice::loop(void *pArg){
  // (re-)connect mqtt client:
  if (!mqttClient.connected()) {
    Serial.print("MQTT not connected. ");
    String clientId = location;
    clientId += "/";
    clientId += title;
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Now successfully connected. ");
    } else {
      Serial.println("Reconnecting failed. ");
    }
  } else { // run mqtt client loop:
    mqttClient.loop();
  }
}
