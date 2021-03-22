#include "ESP_SHD_PwmSingleColorLight.h"

ShdPwmSingleColorLight::ShdPwmSingleColorLight(uint8_t _pin, bool _lowActive, uint8_t _millisUpdateInterval, uint16_t _flankLength) 
  : ShdPwmSingleColorLight(_pin, _lowActive, _millisUpdateInterval, _flankLength, 0) {

  };

ShdPwmSingleColorLight::ShdPwmSingleColorLight(uint8_t _pin, bool _lowActive, uint8_t _millisUpdateInterval, uint16_t _flankLength, uint16_t _setPointAtStart){
  bool success = true;
  pin = _pin;
  lowActive = _lowActive;
  millisUpdateInterval = _millisUpdateInterval;
  delta = 100/(_flankLength / millisUpdateInterval);
  flankOver = true;
  setPoint = _setPointAtStart;
  millisLastUpdate = millis();
  lastBrightnessGreaterZero = 100;
  currentBrightness = setPoint;

  int8_t _pwmNumber = registerPwmPin(this, pin, lowActive);
  if (_pwmNumber != -1) {
    pwmNumber = _pwmNumber;
  } else {
    success = false;
  }


  snprintf(pubTopicBrightness, 60, "%s/PwmLight/%d/getBrightness", name, pwmNumber);
  snprintf(pubTopicState, 60, "%s/PwmLight/%d/getStatus", name, pwmNumber);
  snprintf(subTopicBrightness, 60, "%s/PwmLight/%d/setBrightness", name, pwmNumber);
  snprintf(subTopicState, 60, "%s/PwmLight/%d/setStatus", name, pwmNumber);

  if (!mqttSubscribe(this, subTopicBrightness)) {
    success = false;
  }
  if (!mqttSubscribe(this, subTopicState)) {
    success = false;
  }


  // debug output:
  if (success) {
    Serial.print("PWM: New pwm light ");
    Serial.print(pwmNumber);
    Serial.print(" registered. It subscribed to ");
    Serial.print(subTopicBrightness);
    Serial.print(" and ");
    Serial.print(subTopicState);
    Serial.println();
  } else {
    Serial.print("PWM: Something went wrong.");
  }
}

void ShdPwmSingleColorLight::timer5msHandler() {

  // return, if this light is not changing its brightness
  if (flankOver) {
    return;
  }

  uint32_t currentMillis = millis();
  if(currentMillis - millisLastUpdate >= millisUpdateInterval){

    #if DEBUG > 3
    Serial.print("SHD: PwmLight: Updating no. ");
    Serial.print(pwmNumber);
    Serial.print(" at ");
    Serial.print(currentMillis);
    #endif

    // Increase millisLastUpdate to keep interval:
    millisLastUpdate += millisUpdateInterval;
    if(currentMillis - millisLastUpdate >= millisUpdateInterval) {
      millisLastUpdate = millis();
    }

    if (abs((int8_t)((int8_t)setPoint - (int8_t)currentBrightness)) <= delta) {
      currentBrightness = setPoint;
      flankOver = true;
    } else if (((int8_t)(setPoint - currentBrightness)) > 0) {
      currentBrightness += delta;
      if (currentBrightness > 100) {
        currentBrightness = 100;
        flankOver = true;
      }
    } else if(((int8_t)(setPoint - currentBrightness)) < 0) {
      currentBrightness -= delta;
      if (currentBrightness < 0) {
        currentBrightness = 0;
        flankOver = true;
      }
    }

    #if DEBUG > 3
    Serial.print(". Percentage: ");
    Serial.println(currentBrightness);
    #endif

    setPwmPermill(this, pwmNumber, currentBrightness*10);
  }
}

bool ShdPwmSingleColorLight::handleMqttRequest(char *_topic, unsigned char *_payload, uint16_t _length){
  if (strcmp(_topic, subTopicState) == 0) {
    if (_payload[0] == 0x30) {
      #if DEBUG > 3
      Serial.println("SHD: PwmLight: OFF");
      #endif
      setBrightness(0);
    } else if (_payload[0] == 0x31) {
      #if DEBUG > 3
      Serial.println("SHD: PwmLight: ON");
      #endif
      setBrightness(lastBrightnessGreaterZero);
    }
  } else if (strcmp(_topic, subTopicBrightness) == 0) {
    #if DEBUG > 3
    Serial.print("SHD: PwmLight: Brightness set to ");
    Serial.print(atoi((char*)_payload));
    Serial.println(" %");
    #endif
    setBrightness(atoi((char*)_payload));
  } else {
    return false;
  }
}

void ShdPwmSingleColorLight::setBrightness(uint8_t _percentage){
  if (_percentage < 0 || _percentage > 100) {
    return;
  }
  if (_percentage > 0) {
    // save every percentage greater 0 for restoring after turning the light off and on
    lastBrightnessGreaterZero = _percentage;
    mqttPublish(pubTopicState, "1");
  } else {
    mqttPublish(pubTopicState, "0");
  }

  // save new set point
  setPoint = _percentage;

  // publish new brightness
  char payload[5];
  snprintf (payload, 5, "%d", setPoint);
  mqttPublish(pubTopicBrightness, payload);

  // start fading process:
  flankOver = false;
}

void ShdPwmSingleColorLight::republish(){

  if (currentBrightness != 0) {
    mqttPublish(pubTopicState, "1");
  } else {
    mqttPublish(pubTopicState, "0");
  }

  char payload[5];
  snprintf (payload, 5, "%d", setPoint);
  mqttPublish(pubTopicBrightness, payload);

}
