#include "ESP_SHD_MotionSensor.h"
#include <FunctionalInterrupt.h>

ShdMotionSensor::ShdMotionSensor(uint8_t _pin){

  // prepare pin and attach interrupt:
  pin = _pin;
  pinMode(pin, INPUT);
  attachInterrupt(pin, std::bind(&ShdMotionSensor::pinChange, this), CHANGE);

  // initialize variables
  motionDeteced = false;
  snprintf (pubTopic, 50, "%s/Bewegung", name);

  // debug output:
  Serial.print("New motion sensor registered. It publishes to ");
  Serial.print(pubTopic);
  Serial.println();
}

bool ShdMotionSensor::handleMqttRequest(char* _topic, unsigned char* _payload, unsigned int _length){
  return false;
}

void ShdMotionSensor::timer5msHandler(){
  return;
}

void ShdMotionSensor::pinChange(){
  bool motionSensorStatus = digitalRead(pin);
  if (motionSensorStatus && !motionDeteced) {
    Serial.print("Motion detected. ");
    if (mqttClient.publish(pubTopic, "true")) {
      Serial.print("Published via MQTT. ");
    } else {
      Serial.print("Could not publish via MQTT. ");
    }
    Serial.println();
  } else if (!motionSensorStatus && motionDeteced) {
    Serial.print("No motion detected. ");
    if (mqttClient.publish(pubTopic, "false")) {
      Serial.print("Published via MQTT. ");
    } else {
      Serial.print("Could not publish via MQTT. ");
    }
    Serial.println();
  }
  motionDeteced = motionSensorStatus;
}
