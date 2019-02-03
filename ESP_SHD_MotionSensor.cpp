#include "ESP_SHD_MotionSensor.h"
#include <FunctionalInterrupt.h>

#define DEBUG 0

ShdMotionSensor::ShdMotionSensor(uint8_t _pin){

  // prepare pin and attach interrupt:
  pin = _pin;
  pinMode(pin, INPUT);
  attachInterrupt(pin, std::bind(&ShdMotionSensor::pinChange, this), CHANGE);

  // initialize variables
  motionDeteced = false;
  motionSensorStatus = false;
  snprintf (pubTopic, 50, "%s/Motion", name);

  // debug output:
  Serial.print("New motion sensor registered. It publishes to ");
  Serial.println(pubTopic);
  Serial.println();
}

bool ShdMotionSensor::handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length){
  return false;
}

void ShdMotionSensor::timer5msHandler(){

  // test, if mqtt client is connected
  if (!mqttClient.connected()) {
    return;
  }


  if (motionSensorStatus && !motionDeteced) {
                                                                  #if DEBUG > 2
                                                                  Serial.print("Motion detected. ");
                                                                  #endif
    if (mqttClient.publish(pubTopic, "true")) {
                                                                  #if DEBUG > 2
                                                                  Serial.print("Published via MQTT. ");
                                                                  #endif
    } else {
                                                                  #if DEBUG > 2
                                                                  Serial.print("Could not publish via MQTT. ");
                                                                  #endif
    }
                                                                  #if DEBUG > 2
                                                                  Serial.println();
                                                                  #endif
  } else if (!motionSensorStatus && motionDeteced) {
                                                                  #if DEBUG > 2
                                                                  Serial.print("No motion detected. ");
                                                                  #endif
    if (mqttClient.publish(pubTopic, "false")) {
                                                                  #if DEBUG > 2
                                                                  Serial.print("Published via MQTT. ");
                                                                  #endif
    } else {
                                                                  #if DEBUG > 2
                                                                  Serial.print("Could not publish via MQTT. ");
                                                                  #endif
    }
                                                                  #if DEBUG > 2
                                                                  Serial.println();
                                                                  #endif
  }
  motionDeteced = motionSensorStatus;
  return;
}

void ShdMotionSensor::pinChange(){
  motionSensorStatus = digitalRead(pin);
}


void ShdMotionSensor::resubpub() {
  if (motionSensorStatus) {
    mqttClient.publish(pubTopic, "true");
  } else {
    mqttClient.publish(pubTopic, "false");
  }
}
