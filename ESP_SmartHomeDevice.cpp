#include "ESP_SmartHomeDevice.h"

ESP_SmartHomeDevice* ESP_SmartHomeDevice::shds[MAX_SHDS];
PubSubClient ESP_SmartHomeDevice::mqttClient;
WiFiClient ESP_SmartHomeDevice::wifiClient;
uint8_t ESP_SmartHomeDevice::numberOfShds = 0;
uint32_t ESP_SmartHomeDevice::lastConnectionAttempt = 0;
uint32_t ESP_SmartHomeDevice::last5msTimer = 0;
char* ESP_SmartHomeDevice::name;
os_timer_t ESP_SmartHomeDevice::loopTimer;
bool ESP_SmartHomeDevice::useMdns = false;
char* ESP_SmartHomeDevice::mqttServerAddress;
uint16_t ESP_SmartHomeDevice::port;


ESP_SmartHomeDevice::ESP_SmartHomeDevice(){
  if (numberOfShds < MAX_SHDS-1) {
    shds[numberOfShds] = this;
    numberOfShds++;
  }
  Serial.println("SHD: New device registered. ");
}

void ESP_SmartHomeDevice::init(char* _mqttServerAddress, uint16_t _port, char* _name){
  numberOfShds = 0;
  name = _name;
  useMdns = false;

  mqttServerAddress = _mqttServerAddress;
  port = _port;

  connectWifi();
  Serial.println("------------------------------------------");
  Serial.println();
}

void ESP_SmartHomeDevice::init(char *_name){
  numberOfShds = 0;
  name = _name;
  useMdns = true;

  connectWifi();
  Serial.println("------------------------------------------");
  Serial.println();
}

void ESP_SmartHomeDevice::connectWifi(){
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    WiFiManager wifiManager;
    wifiManager.autoConnect(name);
    WiFi.hostname(name);
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
  if (n != 1) {
    Serial.print("SHD: ");
    Serial.print(n);
    Serial.println(" mqtt services found.");
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
  uint32_t currentMicros = micros();
  if(currentMicros - last5msTimer > 5000){
    while (currentMicros - last5msTimer > 5000) {
      last5msTimer += 5000;
    }

    if (!mqttClient.loop()) {
      #if DEBUG > 6
      Serial.println("MQTT: loop() returned false.");
      #endif
      reconnect();
      return;
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
    connectWifi();

    // reconnect mqtt client:
    if (useMdns) {
      reconnectMqtt();
    } else {
      reconnectMqtt(mqttServerAddress, port);
    }

    // if reconnecting has been successfull, resubscribe to all topics and publish current states:
    if (mqttClient.connected()) {
      for (size_t i = 0; i < numberOfShds; i++) {
        shds[i]->resubpub();
      }
      Serial.println("MQTT: Successfully (re-)subscribed.");
    }  else {
      Serial.print(" Last connection attempt ");
      Serial.print(currentMillis - lastConnectionAttempt);
      Serial.println(" milliseconds ago.");
    }

    // update lastConnectionAttempt
    while (lastConnectionAttempt < (currentMillis - TRY_RECONNECT_AFTER_MILLISECONDS)) {
      lastConnectionAttempt += TRY_RECONNECT_AFTER_MILLISECONDS;
    }
  }
}
