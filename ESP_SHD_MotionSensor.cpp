#include "ESP_SHD_MotionSensor.h"
#include <FunctionalInterrupt.h>

ShdMotionSensor::ShdMotionSensor(uint8_t _pin){
  motionDeteced = false;
  pin = _pin;
  pinMode(pin, INPUT);
  attachInterrupt(pin, std::bind(&ShdMotionSensor::pinChange,this), CHANGE);
  Serial.println("New motion sensor registered.");
  Serial.println();
}

bool ShdMotionSensor::handleMqttRequest(char* _topic, byte* _payload, unsigned int _length){
  return false;
}

void ShdMotionSensor::pinChange(){
  bool motionSensorStatus = digitalRead(pin);
  if (motionSensorStatus && !motionDeteced) {
    Serial.print("Motion detected. ");
    if (mqttClient.publish("test1", "true")) {
      Serial.print("Published via MQTT. ");
    } else {
      Serial.print("Could not publish via MQTT. ");
    }
    Serial.println();
  } else if (!motionSensorStatus && motionDeteced) {
    Serial.print("No motion detected.");
    if (mqttClient.publish("test1", "false")) {
      Serial.print("Published via MQTT. ");
    } else {
      Serial.print("Could not publish via MQTT. ");
    }
    Serial.println();
  }
  motionDeteced = motionSensorStatus;
}
