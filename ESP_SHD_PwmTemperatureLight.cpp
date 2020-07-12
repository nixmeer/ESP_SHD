#include "ESP_SHD_PwmTemperatureLight.h"

uint8_t ShdPwmTemperatureLight::pwmTemperatureCount = 0;

ShdPwmTemperatureLight::ShdPwmTemperatureLight(temperaturePwmMode _mode, uint8_t _pin1, uint8_t _pin2, bool _lowActive1, bool _lowActive2, uint8_t _millisUpdateInterval, uint16_t _flankLength) {
  pwmTemperatureNumber = ++pwmTemperatureCount;

  mode = _mode;


  updateInterval = _millisUpdateInterval/5;
  if (updateInterval < 1) {
    updateInterval = 1;
  }
  updateCounter = updateInterval;

  flankLength = _flankLength/(5*updateInterval);

  lastSetPointBrightnessGreaterZero = 100;
  setPointBrightness = 0;
  setPointTemperature = (T_WARMEST + T_COOLEST)/2;

  if (mode == MODE_WARM_PWM_COLD_PWM) {
    coldPin = _pin1;
    warmPin = _pin2;
    currentCold = 0;
    currentWarm = 0;
    updateVariables();

    // register PWMs:
    int8_t _coldPwmNumber = registerPwmPin(this, coldPin, _lowActive1);
    int8_t _warmPwmNumber = registerPwmPin(this, warmPin, _lowActive2);
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
  }

  if (mode == MODE_BRIGHTNESS_PWM_TEMP_PWM) {
    brightnessPin = _pin1;
    tempPin = _pin2;
    currentBrightness = 0;
    currentTemp = (T_WARMEST + T_COOLEST)/2;

    // register PWMs:
    int8_t _brightnessPwmNumber = registerPwmPin(this, brightnessPin, _lowActive1);
    int8_t _tempPwmNumber = registerPwmPin(this, tempPin, _lowActive2);
    if (_brightnessPwmNumber == -1) {
      Serial.println("TMPL: registering brightnessPwmNumber failed.");
    } else {
      brightnessPwmNumber = _brightnessPwmNumber;
    }
    if (_tempPwmNumber == -1) {
      Serial.println("TMPL: registering tempPwmNumber failed.");
    } else {
      tempPwmNumber = _tempPwmNumber;
    }
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
  if (mode == MODE_WARM_PWM_COLD_PWM) {
    if (setPointCold == currentCold && setPointWarm == currentWarm) {
      updateCounter = updateInterval;
      return;
    }
  } else if (mode == MODE_BRIGHTNESS_PWM_TEMP_PWM) {
    if (setPointBrightness == currentBrightness && setPointTemperature == currentTemp) {
      updateCounter = updateInterval;
      return;
    }
  }

  // check, if time for update:
  if (++updateCounter < updateInterval) {
    return;
  }
  updateCounter = 0;

  if (mode == MODE_WARM_PWM_COLD_PWM) {

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

      #if DEBUG_LEVEL > 4
      Serial.print("TMPL: currentCold = ");
      Serial.print((uint16_t)round(currentCold));
      Serial.print(", currentWarm = ");
      Serial.print((uint16_t)round(currentWarm));
      Serial.println();
      #endif

      // set pwms:
      if (!setPwmPermill(this, coldPwmNumber, (uint16_t)round(currentCold))) {
        #if DEBUG_LEVEL > 4
        Serial.print(", coldPwm failed");
        #endif
      }
      if (!setPwmPermill(this, warmPwmNumber, (uint16_t)round(currentWarm))) {
        #if DEBUG_LEVEL > 4
        Serial.print(", warmPwm failed");
        #endif
      }
  }

  if (mode == MODE_BRIGHTNESS_PWM_TEMP_PWM) {
    if (deltaBrightness > 0) {
      if (setPointBrightnessPwm - currentBrightness > deltaBrightness) {
        currentBrightness += deltaBrightness;
      } else {
        currentBrightness = setPointBrightnessPwm;
      }
    } else if (deltaBrightness < 0) {
      if (setPointBrightnessPwm - currentBrightness < deltaBrightness) {
        currentBrightness += deltaBrightness;
      } else {
        currentBrightness = setPointBrightnessPwm;
      }
    }
    if (deltaTemp > 0) {
      if (setPointTempPwm - currentTemp > deltaTemp) {
        currentTemp += deltaTemp;
      } else {
        currentTemp = setPointTempPwm;
      }
    } else if (deltaTemp < 0) {
      if (setPointTempPwm - currentTemp < deltaTemp) {
        currentTemp += deltaTemp;
      } else {
        currentTemp = setPointTempPwm;
      }
    }

    // set pwms:
    if (!setPwmPermill(this, brightnessPwmNumber, (uint16_t)round(currentBrightness))) {
      #if DEBUG_LEVEL > 4
      Serial.print(", brightnessPwm failed");
      #endif
    }
    if (!setPwmPermill(this, tempPwmNumber, (uint16_t)round(currentTemp))) {
      #if DEBUG_LEVEL > 4
      Serial.print(", tempPwm failed");
      #endif
    }
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

  if (mode == MODE_WARM_PWM_COLD_PWM) {

    float newSetPointCold = setPointBrightness * 10;
    float newSetPointWarm = setPointBrightness * 10;

    float temperatureOffset = setPointTemperature - ((T_WARMEST + T_COOLEST)/2);
    float temperatureOffsetNormalizedTo1 = temperatureOffset / (((T_WARMEST - T_COOLEST)/2));

    #if DEBUG_LEVEL > 3
    Serial.print("TMPL: setPointTemperature = ");
    Serial.print(setPointTemperature);
    Serial.print(", setPointBrightness = ");
    Serial.print(setPointBrightness);
    Serial.print(", temperatureOffset = ");
    Serial.print(temperatureOffset);
    Serial.print(", temperatureOffsetNormalizedTo1 = ");
    Serial.print(temperatureOffsetNormalizedTo1);
    Serial.println();
    #endif

    if (temperatureOffset > 0) {
      newSetPointCold = newSetPointCold * (1-temperatureOffsetNormalizedTo1);
    } else if (temperatureOffset < 0) {
      newSetPointWarm = newSetPointWarm * (1+temperatureOffsetNormalizedTo1);
    }

    newSetPointCold = min((float)1000, newSetPointCold);
    newSetPointWarm = min((float)1000, newSetPointWarm);

    deltaCold = (newSetPointCold - currentCold)/flankLength;
    deltaWarm = (newSetPointWarm - currentWarm)/flankLength;

    setPointCold = newSetPointCold;
    setPointWarm = newSetPointWarm;
  }

  if (mode == MODE_BRIGHTNESS_PWM_TEMP_PWM) {
    float newSetPointBrightnessPwm = setPointBrightness * 10;
    float newSetPointTempPwm = (setPointTemperature - 140) * 1000 / (T_WARMEST - T_COOLEST);

    newSetPointBrightnessPwm = min((float)1000, newSetPointBrightnessPwm);
    newSetPointTempPwm = min((float)1000, newSetPointTempPwm);

    deltaBrightness = (newSetPointBrightnessPwm - currentBrightness)/flankLength;
    deltaTemp = (newSetPointTempPwm - currentTemp)/flankLength;

    setPointBrightnessPwm = newSetPointBrightnessPwm;
    setPointTempPwm = newSetPointTempPwm;

    #if DEBUG_LEVEL > 3
    Serial.print("TMPL: setPointBrightness = ");
    Serial.print(setPointBrightness);
    Serial.print(", setPointBrightnessPwm = ");
    Serial.print(setPointBrightnessPwm);
    Serial.print(", setPointTemperature = ");
    Serial.print(setPointTemperature);
    Serial.print(", setPointTempPwm = ");
    Serial.print(setPointTempPwm);
    Serial.println();
    #endif
  }

}
