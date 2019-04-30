#include "ESP_SHD_DS18B20.h"

#define DEBUG 0

ShdDs18b20Sensor::ShdDs18b20Sensor(uint8_t _pin) : oneWire(_pin), DS18B20(&oneWire){
  timerCounter = 0;

  snprintf (pubTopic, 50, "%s/Temperature", name);

  // debug output:
  #if DEBUG > 0
  Serial.print("TMP: New temperature sensor registered. It publishes to ");
  Serial.println(pubTopic);
  Serial.print();
  #endif
}

bool ShdDs18b20Sensor::handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length){
  return false;
}

void ShdDs18b20Sensor::timer5msHandler(){
  timerCounter++;
  if (timerCounter > 6000) {
    publishTemperature();
  }
  if (timerCounter > 60000) {
    timerCounter = 6001;
  }
}

void ShdDs18b20Sensor::publishTemperature() {

  // Temperaturerfassung
  DS18B20.requestTemperatures();
  float temperature = DS18B20.getTempCByIndex(0);

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

  // reset timer, if publishing worked out:
  timerCounter = 0;
}

void ShdDs18b20Sensor::resubpub() {
  publishTemperature();
}
