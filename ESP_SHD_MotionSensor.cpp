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
  Serial.print("Pin changed: ");
  bool motionSensorStatus = digitalRead(pin);
  Serial.println(motionSensorStatus);
  Serial.println();
  // if (motionSensorStatus && !motionDeteced) {
  //   mqttClient.publish("test1", "true");
  // } else if (!motionSensorStatus && motionDeteced) {
  //   mqttClient.publish("test1", "false");
  // }
  motionDeteced = motionSensorStatus;
}
