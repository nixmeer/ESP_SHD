#include "ESP_SHD_TemperatureSensor.h"

#define DEBUG 0

ShdTemperatureSensor::ShdTemperatureSensor(){
  timerCounter = 0;

  snprintf (pubTopic, 50, "%s/Temperature", name);

  // debug output:
  #if DEBUG > 0
  Serial.print("TMP: New temperature sensor registered. It publishes to ");
  Serial.println(pubTopic);
  Serial.print();
  #endif
}

bool ShdTemperatureSensor::handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length){
  return false;
}

void ShdTemperatureSensor::timer5msHandler(){
  timerCounter++;
  if (timerCounter > 6000) {
    publishTemperature();
    timerCounter = 0;
  }
}

void ShdTemperatureSensor::publishTemperature() {

  int adcValue = analogRead(A0);
  float temperature = 0.07326 * adcValue - 39.9767;

  for (size_t i = 0; i < 10; i++) {
    message[i] = 0;
  }

  dtostrf(temperature, 3, 1, message);

  if (!mqttClient.connected()) {
    timerCounter = 5999; // set close to 6000, so next time timer5ms() is called, it's trying again
    return;
  }

  if (mqttClient.publish(pubTopic, message)) {
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

void ShdTemperatureSensor::resubpub() {
  publishTemperature();
}
