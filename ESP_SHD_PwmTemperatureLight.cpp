#include "ESP_SHD_PwmTemperatureLight.h"

uint8_t ShdPwmTemperatureLight::pwmTemperatureCount = 0;

ShdPwmTemperatureLight::ShdPwmTemperatureLight(uint8_t _coldPin, uint8_t _warmPin, bool _lowActive, uint8_t _millisUpdateInterval, uint16_t _flankLength) {
  pwmTemperatureNumber = ++pwmTemperatureCount;

  coldPin = _coldPin;
  warmPin = _warmPin;
  lowActive = _lowActive;

  updateInterval = _millisUpdateInterval/5;
  if (updateInterval < 1) {
    updateInterval = 1;
  }
  updateCounter = updateInterval;

  flankLength = _flankLength/(5*updateInterval);

  currentCold = 0;
  currentWarm = 0;
  lastSetPointBrightnessGreaterZero = 100;
  setPointBrightness = 0;
  setPointTemperature = (T_WARMEST + T_COOLEST)/2;
  updateVariables();

  // register PWMs:
  int8_t _coldPwmNumber = registerPwmPin(this, coldPin, lowActive);
  int8_t _warmPwmNumber = registerPwmPin(this, warmPin, lowActive);
  if (_coldPwmNumber == -1) {
    Serial.println("TMPL: registering coldPwmNumber failed.");
  } else {
    coldPwmNumber = _coldPwmNumber;
  }
  if (_warmPwmNumber == -1) {
    Serial.println("TMPL: registering warmPwmNumber failed.");
  } else {
    warmPwmNumber = _warmPwmNumber;
  }

  snprintf(pubTopicState, MQTT_TOPIC_SIZE, "%s/PwmTempLight/%d/getState", name, pwmTemperatureNumber);
  snprintf(subTopicState, MQTT_TOPIC_SIZE, "%s/PwmTempLight/%d/setState", name, pwmTemperatureNumber);
  snprintf(pubTopicBrightness, MQTT_TOPIC_SIZE, "%s/PwmTempLight/%d/getBrightness", name, pwmTemperatureNumber);
  snprintf(subTopicBrightness, MQTT_TOPIC_SIZE, "%s/PwmTempLight/%d/setBrightness", name, pwmTemperatureNumber);
  snprintf(pubTopicTemperature, MQTT_TOPIC_SIZE, "%s/PwmTempLight/%d/getTemperature", name, pwmTemperatureNumber);
  snprintf(subTopicTemperature, MQTT_TOPIC_SIZE, "%s/PwmTempLight/%d/setTemperature", name, pwmTemperatureNumber);

  mqttSubscribe(this, subTopicState);
  mqttSubscribe(this, subTopicBrightness);
  mqttSubscribe(this, subTopicTemperature);
}

void ShdPwmTemperatureLight::timer5msHandler() {
  // check, if setPoints are reached:
  if (setPointCold == currentCold && setPointWarm == currentWarm) {
    updateCounter = updateInterval;
    return;
  }

  // check, if time for update:
  if (++updateCounter < updateInterval) {
    return;
  }
  updateCounter = 0;

  // Update currentCold and currentWarm:
  if (deltaCold > 0) {
    if (setPointCold - currentCold > deltaCold) {
      currentCold += deltaCold;
    } else {
      currentCold = setPointCold;
    }
  } else if (deltaCold < 0) {
    if (setPointCold - currentCold < deltaCold) {
      currentCold += deltaCold;
    } else {
      currentCold = setPointCold;
    }
  }
  if (deltaWarm > 0) {
    if (setPointWarm - currentWarm > deltaWarm) {
      currentWarm += deltaWarm;
    } else {
      currentWarm = setPointWarm;
    }
  } else if (deltaWarm < 0) {
    if (setPointWarm - currentWarm < deltaWarm) {
      currentWarm += deltaWarm;
    } else {
      currentWarm = setPointWarm;
    }
  }


  Serial.print("TMPL: currentCold = ");
  Serial.print((uint16_t)round(currentCold));
  Serial.print(", currentWarm = ");
  Serial.print((uint16_t)round(currentWarm));
  Serial.println();

  // set pwms:
  if (!setPwmPermill(this, coldPwmNumber, (uint16_t)round(currentCold))) {
    Serial.print(", coldPwm failed");
  }
  if (!setPwmPermill(this, warmPwmNumber, (uint16_t)round(currentWarm))) {
    Serial.print(", warmPwm failed");
  }

}

bool ShdPwmTemperatureLight::handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length) {
  if (strcmp(_topic, subTopicState) == 0) {
    if (_payload[0] == 0x31) {
      setStatus(true);
    } else {
      setStatus(false);
    }
  } else if (strcmp(_topic, subTopicBrightness) == 0) {
    setBrightness(atoi((char*)_payload));
  } else if (strcmp(_topic, subTopicTemperature) == 0) {
    setTemperature(atoi((char*)_payload));
  } else {
    return false;
  }
  republish();
  return true;
}

void ShdPwmTemperatureLight::setBrightness(uint8_t _percentage) {
  setPointBrightness = _percentage;
  setPointBrightness = min(setPointBrightness, (uint8_t)100);
  setPointBrightness = max(setPointBrightness, (uint8_t)0);
  updateVariables();
}

void ShdPwmTemperatureLight::setTemperature(uint16_t _temperature) {
  setPointTemperature = _temperature;
  setPointTemperature = min(setPointTemperature, (uint16_t)T_WARMEST);
  setPointTemperature = max(setPointTemperature, (uint16_t)T_COOLEST);
  updateVariables();
}

void ShdPwmTemperatureLight::setStatus(bool _status) {
  if (_status == false && setPointBrightness > 0) {
    setPointBrightness = 0;
    updateVariables();
  } else if (_status == true && setPointBrightness == 0) {
    setPointBrightness = lastSetPointBrightnessGreaterZero;
    updateVariables();
  }
}

void ShdPwmTemperatureLight::republish() {
  // Publish status:
  if (setPointBrightness > 0) {
    mqttPublish(pubTopicState, "1");
  } else {
    mqttPublish(pubTopicState, "0");
  }

  // create payload buffer:
  char payloadBuffer[MQTT_PAYLOAD_BUFFER_SIZE];

  // publish brightness:
  for (uint16_t i = 0; i < MQTT_PAYLOAD_BUFFER_SIZE; i++) {
    payloadBuffer[i] = 0;
  }
  snprintf(payloadBuffer, MQTT_PAYLOAD_BUFFER_SIZE, "%i", setPointBrightness);
  mqttPublish(pubTopicBrightness, payloadBuffer);

  // publish temperature:
  for (uint16_t i = 0; i < MQTT_PAYLOAD_BUFFER_SIZE; i++) {
    payloadBuffer[i] = 0;
  }
  snprintf(payloadBuffer, MQTT_PAYLOAD_BUFFER_SIZE, "%i", setPointTemperature);
  mqttPublish(pubTopicTemperature, payloadBuffer);
}

void ShdPwmTemperatureLight::updateVariables() {

  if (setPointBrightness > 0) {
    lastSetPointBrightnessGreaterZero = setPointBrightness;
  }

  float newSetPointCold = setPointBrightness * 10;
  float newSetPointWarm = setPointBrightness * 10;

  float temperatureOffset = setPointTemperature - ((T_WARMEST + T_COOLEST)/2);
  float temperatureOffsetNormalizedTo1 = temperatureOffset / (((T_WARMEST - T_COOLEST)/2));

  Serial.print("TMPL: setPointTemperature = ");
  Serial.print(setPointTemperature);
  Serial.print(", setPointBrightness = ");
  Serial.print(setPointBrightness);
  Serial.print(", temperatureOffset = ");
  Serial.print(temperatureOffset);
  Serial.print(", temperatureOffsetNormalizedTo1 = ");
  Serial.print(temperatureOffsetNormalizedTo1);
  Serial.println();

  if (temperatureOffset > 0) {
    newSetPointCold = newSetPointCold * (1-temperatureOffsetNormalizedTo1);
  } else if (temperatureOffset < 0) {
    newSetPointWarm = newSetPointWarm * (1+temperatureOffsetNormalizedTo1);
  }

  newSetPointCold = min((float)1000, newSetPointCold);
  newSetPointWarm = min((float)1000, newSetPointWarm);

  deltaCold = (newSetPointCold - setPointCold)/flankLength;
  deltaWarm = (newSetPointWarm - setPointWarm)/flankLength;

  setPointCold = newSetPointCold;
  setPointWarm = newSetPointWarm;
}
