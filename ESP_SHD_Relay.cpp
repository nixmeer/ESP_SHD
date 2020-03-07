#include "ESP_SHD_Relay.h"
uint8_t ShdRelay::relayCount = 0;


ShdRelay::ShdRelay(uint8_t _pin, uint32_t _millisBetweenToggle, bool _lowActive, bool _valueAtBeginning) : pin(_pin), lowActive(_lowActive){

  relayCount++;
  relayNumber = relayCount;

  if (_millisBetweenToggle/5 > 0) {
    minCyclesBetweenToggles = _millisBetweenToggle/5;
  } else {
    minCyclesBetweenToggles = 1;
  }

  setPoint = _valueAtBeginning;

  pinMode(pin, OUTPUT);
  setOuput(setPoint);
  cycleCounterSinceLastToggle = 0;

  snprintf(subTopicSetStatus, 50, "%s/Relay/%d/setStatus", name, relayNumber);
  snprintf(pubTopicGetStatus, 50, "%s/Relay/%d/getStatus", name, relayNumber);

  mqttSubscribe(this, subTopicSetStatus);
  #if DEBUG >= 1
  Serial.print("RELAY: No. ");
  Serial.print(relayCount);
  Serial.print(" subscribed to ");
  Serial.println(subTopicSetStatus);
  #endif
}

void ShdRelay::timer5msHandler(){
  // increase cycleCounterSinceLastToggle:
  cycleCounterSinceLastToggle++;

  // if overflow, reset to minCyclesBetweenToggles:
  if (cycleCounterSinceLastToggle == 0xFFFF) {
    cycleCounterSinceLastToggle = minCyclesBetweenToggles+1;
  }

  if (setPoint != currentStatus) {
    if (cycleCounterSinceLastToggle > minCyclesBetweenToggles) {
      setOuput(setPoint);
    }
  }
}

bool ShdRelay::handleMqttRequest(char *_topic, unsigned char *_payload, uint16_t _length){
  if (strcmp(_topic, subTopicSetStatus) == 0) {
    if (_payload[0] == 0x31) {
      setPoint = true;
    } else {
      setPoint = false;
    }
  } else {
    return false;
  }
  republish();
  return true;
}

void ShdRelay::republish(){
  if (setPoint == true) {
    mqttPublish(pubTopicGetStatus, "1");
  } else {
    mqttPublish(pubTopicGetStatus, "0");
  }
}

void ShdRelay::setOuput(bool _setPoint) {
  if (!lowActive) {
    digitalWrite(pin, _setPoint);
  } else {
    digitalWrite(pin, !_setPoint);
  }
  currentStatus = _setPoint;
  cycleCounterSinceLastToggle = 0;
}
