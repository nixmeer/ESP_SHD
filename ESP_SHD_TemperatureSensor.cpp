#include "ESP_SHD_TemperatureSensor.h"
#include <FunctionalInterrupt.h>

#define DEBUG 0

ShdTemperatureSensor::ShdTemperatureSensor(){
  timerCounter = 0;

  snprintf (pubTopic, 50, "%s/Temperatur", name);

  // os_timer_setfn(&this->timer, std::bind(&ShdTemperatureSensor::publishTemperature,this), NULL);
  // os_timer_arm(&this->timer, PUBLISH_TIME_IN_MS, true);

  // debug output:
  Serial.print("New temperature sensor registered. It publishes to ");
  Serial.print(pubTopic);
  Serial.println();
}

bool ShdTemperatureSensor::handleMqttRequest(char* _topic, unsigned char* _payload, unsigned int _length){
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

void ShdTemperatureSensor::resubscribe() {
  return;
}
