#include "ESP_SHD_TMP36.h"

#define DEBUG 0

ShdTmp36Sensor::ShdTmp36Sensor(){
  timerCounter = 0;

  snprintf (pubTopic, 50, "%s/Temperature", name);

  // debug output:
  #if DEBUG > 0
  Serial.print("TMP: New temperature sensor registered. It publishes to ");
  Serial.println(pubTopic);
  Serial.print();
  #endif
}

bool ShdTmp36Sensor::handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length){
  return false;
}

void ShdTmp36Sensor::timer5msHandler(){
  timerCounter++;
  if (timerCounter > 6000) {
    publishTemperature();
    timerCounter = 0;
  }
}

void ShdTmp36Sensor::publishTemperature() {

  int adcValue = analogRead(A0);
  float temperature = 0.07326 * adcValue - 39.9767;

  for (size_t i = 0; i < 10; i++) {
    message[i] = 0;
  }

  dtostrf(temperature, 3, 1, message);

  if (!mqttConnected()) {
    timerCounter = 5999; // set close to 6000, so next time timer5ms() is called, it's trying again
    return;
  }

  if (mqttPublish(pubTopic, message)) {
    #if DEBUG > 1
    Serial.print("Temperature published to ");
    Serial.print(pubTopic);
    Serial.print(" at ");
    Serial.print(millis());
    Serial.print(" ms: ");
    Serial.println(message);
    #endif
  }
}

void ShdTmp36Sensor::republish() {
  publishTemperature();
}
