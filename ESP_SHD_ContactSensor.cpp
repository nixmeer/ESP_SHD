#include "ESP_SHD_ContactSensor.h"
uint8_t ESP_SHD_ContactSensor::contactSensorCount = 0;

ShdContactSensor:ShdContactSensor(contactSensorType _type, bool _sleepAfterPublish)
  : ShdContactSensor(_type, 0, false, _sleepAfterPublish) { }

ShdContactSensor:ShdContactSensor(contactSensorType _type, uint8_t _pin, bool _lowAcive, bool _sleepAfterPublish, uint16_t _intervalMillis) {
  csType = _type;
  pin = _pin;
  lowActive = _lowAcive;
  sleepAfterPublish = _sleepAfterPublish;
  updateInterval = _intervalMillis/5;
  if (updateInterval < 1) {
    updateInterval = 1;
  }
  updateCounter = updateInterval;

  switch (csType) {
    case CS_NORMAL:
      getStatusNormal();
      break;
    case CS_LSC_DOOR_SENROR:
      getStatusLsc();
      break;
  }
}

void ShdContactSensor::timer5msHandler() {
  return;
}

bool ShdContactSensor::handleMqttRequest(char *_topic, unsigned char *_payload, int _length) {
  return false;
}

void ShdContactSensor::republish() {
  switch (lastStatus) {
    case OPENED:
    case CLOSED:
    case ERROR:
  }
}
