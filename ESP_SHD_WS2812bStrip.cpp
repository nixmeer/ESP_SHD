#include "ESP_SHD_WS2812bStrip.h"
#include "FastLED.h"
#include <FunctionalInterrupt.h>

#define DEBUG 2

uint16_t ShdWs2812bStrip::millisLastStripUpdate;
uint16_t ShdWs2812bStrip::millisStripUpdateInterval;
CRGB ShdWs2812bStrip::leds[MAX_NUM_OF_LEDS];
ShdWs2812bStrip * ShdWs2812bStrip::sections[MAX_NUM_OF_SECTIONS];
uint16_t ShdWs2812bStrip::numberOfLeds = 0;
uint8_t ShdWs2812bStrip::numberOfSections = 0;
bool ShdWs2812bStrip::correctlyInitialized = false;

uint8_t ShdWs2812bStrip::gammaCorrection[256] = {
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4,
  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
  6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8,
  9, 9, 9, 9, 9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13,
  13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18, 19, 19, 19, 20, 20,
  21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28, 28, 29, 29, 30, 31, 31,
  32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
  50, 51, 52, 53, 54, 55, 57, 58, 59, 60, 62, 63, 65, 66, 67, 69, 70, 72, 74, 75,
  77, 79, 80, 82, 84, 86, 88, 89, 91, 93, 96, 98, 100, 102, 104, 107, 109, 111,
  114, 116, 119, 121, 124, 127, 130, 133, 135, 138, 141, 145, 148, 151, 154, 158,
  161, 165, 168, 172, 176, 180, 184, 188, 192, 196, 201, 205, 210, 214, 219, 224,
  229, 234, 239, 244, 249, 255
};

void ShdWs2812bStrip::show(){

  if (!correctlyInitialized) {
    return;
  }

  bool currentlyIgniting = false;
  for (uint8_t i = 0 ; i < numberOfSections; i++) {
    if (sections[i]->igniting) {
      currentlyIgniting = true;
    }
  }
  if (!currentlyIgniting) {
    return;
  }


  // see if it's time to update the strip:
  uint32_t currentMillis = millis();

  uint32_t millisSinceLastUpdate = (uint32_t)(currentMillis - millisLastStripUpdate);

  if ((uint32_t)millisSinceLastUpdate >= (uint32_t)millisStripUpdateInterval) {

    // TODO! Checken, warum die while-Schleife nicht funktioniert  <------------------------------
    // update millisLastStripUpdate:
    // while (millisSinceLastUpdate >= millisStripUpdateInterval) {
    //   millisLastStripUpdate += millisStripUpdateInterval;
    //   millisSinceLastUpdate -= (uint32_t)(currentMillis - millisLastStripUpdate);
    // }
    millisLastStripUpdate = millis();

    // update the LED strip:
    FastLED.show();

    // prepare all CRGBs for next show():
    for (uint8_t i = 0; i < numberOfSections; i++) {
      if (sections[i]->igniting) {
        #if DEBUG > 3
        Serial.print("Updating section no. ");
        Serial.print(i);
        Serial.println(".");
        #endif
        sections[i]->callIgnitionFunction();
      }
    }
  }
}

void ShdWs2812bStrip::initStrip(uint16_t _numberOfLeds, uint16_t _updateInterval){
  numberOfLeds = _numberOfLeds;
  if (_numberOfLeds <= MAX_NUM_OF_LEDS && _numberOfLeds > 0) {
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, numberOfLeds);
    correctlyInitialized = true;
    millisLastStripUpdate = millis();
    millisStripUpdateInterval = _updateInterval;

    FastLED.setCorrection(0xFFA0A0);
    FastLED.setTemperature(HighNoonSun);

    Serial.println();
    Serial.print("WS2812b strip initialized. Update interval: ");
    Serial.print(millisStripUpdateInterval);
    Serial.println();
  }
}

ShdWs2812bStrip::ShdWs2812bStrip(uint16_t _firstLed, uint16_t _lastLed, uint16_t _ignitionPoint, ignitionDirection _ignitionDirection, uint8_t _hopsPerShow, uint8_t _flankLength) {

  // add this object to the sections array:
  sections[numberOfSections] = this;
  numberOfSections++;
  sectionNumber = numberOfSections;

  // check if first and last LED number are plausible:
  if (_firstLed <= _lastLed && _firstLed > 0 && _lastLed <= numberOfLeds) {
    firstLed = _firstLed-1;
    lastLed = _lastLed-1;
  } else {
    correctlyInitialized = false;
    return;
  }

  flankLength = _flankLength;

  sectionLength = lastLed - firstLed + 1;

  if (_ignitionPoint > 0 && _ignitionPoint <= sectionLength) {
    ignitionPoint = _ignitionPoint - 1;
  }

  // check if hopsPerShow > 0
  if (_hopsPerShow > 0 && _hopsPerShow < lastLed - firstLed) {
    hopsPerShow = _hopsPerShow;
  } else {
    Serial.println("hopsPerShow not correct.");
    correctlyInitialized = false;
    return;
  }

  for (uint8_t i = 0; i < 3; i++) {
    savedValue[i] = 255;
    shownValue[i] = 0;
  }

  snprintf (subTopicColor, 50, "%s/Lamp/%d/setColor", name, sectionNumber);
  snprintf (pubTopicColor, 50, "%s/Lamp/%d/getColor", name, sectionNumber);
  snprintf (subTopicState, 50, "%s/Lamp/%d/setStatus", name, sectionNumber);
  snprintf (pubTopicState, 50, "%s/Lamp/%d/getStatus", name, sectionNumber);
  snprintf (pubTopicBrightness, 50, "%s/Lamp/%d/getBrightness", name, sectionNumber);

  resubscribe();

  direction = _ignitionDirection;
  if (direction == IGNITION_SINGLE_BACKWARD || direction == IGNITION_BOTH_BACKWARD ) {
    directionInverted = true;
  } else {
    directionInverted = false;
  }

  igniting = false;

  for (uint8_t i = firstLed; i <= lastLed; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  Serial.print("WS2812b: section no. ");
  Serial.print(sectionNumber);
  Serial.print(" registered. It subscribed to ");
  Serial.print(subTopicState);
  Serial.print(" and ");
  Serial.print(subTopicColor);
  Serial.println();
}

void ShdWs2812bStrip::igniteSingleDir(){

  for (uint16_t i = sectionLength - 1; i >= hopsPerShow; i--) {

    // shift as many LEDs as possible. Every LED value jumps for hopsPerShow
    int16_t targetLed;
    if ((!directionInverted && !directionTmpInverted) || (directionInverted && directionTmpInverted)) {
      targetLed = firstLed + ignitionPoint + i;
    } else {
      targetLed = firstLed + ignitionPoint - i;
    }
    if (targetLed > lastLed) {
      targetLed -= sectionLength;
    } else if (targetLed < firstLed) {
      targetLed += sectionLength;
    }

    int16_t sourceLed;
    if ((!directionInverted && !directionTmpInverted) || (directionInverted && directionTmpInverted)) {
      sourceLed = targetLed - hopsPerShow;
    } else {
      sourceLed = targetLed + hopsPerShow;
    }
    if (sourceLed > lastLed) {
      sourceLed -= sectionLength;
    } else if (sourceLed < firstLed) {
      sourceLed += sectionLength;
    }

    #if DEBUG >= 4
    Serial.print("LED ");
    Serial.print(targetLed);
    Serial.print(" from ");
    Serial.println(sourceLed);
    #endif

    leds[targetLed] = leds[sourceLed];
  }

  // fill the remaining LEDs with new color:
  #if DEBUG == 4
  Serial.println("Fill LEDs: ");
  #endif

  for (uint8_t i = hopsPerShow; i > 0; i--) {

    int16_t targetLed;
    if ((!directionInverted && !directionTmpInverted) || (directionInverted && directionTmpInverted)) {
      targetLed = firstLed + ignitionPoint + (i - 1);
    } else {
      targetLed = firstLed + ignitionPoint - (i - 1);
    }
    if (targetLed > lastLed) {
      targetLed -= sectionLength;
    } else if (targetLed < firstLed) {
      targetLed += sectionLength;
    }

    #if DEBUG == 4
    Serial.print(targetLed);
    Serial.print(", ");
    #endif

    if (fillLedWithNewColor(targetLed)) {
      ignitionCounter++;
    } else {
      ignitionCounter = 0;
    }
  }
  #if DEBUG == 4
  Serial.println();
  #endif

  if (ignitionCounter >= sectionLength + flankLength) {
    ignitionCounter = 0;
    igniting = false;
    #if DEBUG >= 3
    Serial.println("ignition is over. ");
    #endif
  }

}

void ShdWs2812bStrip::igniteBothDir(){

  // shift as many LEDs as possible. Every LED value jumps for hopsPerShow
  for (uint16_t i = (sectionLength/2); i >= hopsPerShow; i--) {

    int16_t targetLed1, targetLed2;
    if ((!directionInverted && !directionTmpInverted) || (directionInverted && directionTmpInverted)) {
      targetLed1 = firstLed + ignitionPoint + i;
      targetLed2 = firstLed + ignitionPoint - i;
    } else {
      targetLed1 = firstLed + ignitionPoint + sectionLength/2 - i;
      targetLed2 = firstLed + ignitionPoint + sectionLength/2 + i;
    }
    if (targetLed1 > lastLed) {
      targetLed1 -= sectionLength;
    } else if (targetLed1 < firstLed) {
      targetLed1 += sectionLength;
    }

    int16_t sourceLed1, sourceLed2;
    if ((!directionInverted && !directionTmpInverted) || (directionInverted && directionTmpInverted)) {
      sourceLed1 = targetLed1 - hopsPerShow;
      sourceLed2 = targetLed2 + hopsPerShow;
    } else {
      sourceLed1 = targetLed1 + hopsPerShow;
      sourceLed2 = targetLed2 - hopsPerShow;
    }
    if (targetLed2 > lastLed) {
      targetLed2 -= sectionLength;
    } else if (targetLed2 < firstLed) {
      targetLed2 += sectionLength;
    }

    if (sourceLed1 > lastLed) {
      sourceLed1 -= sectionLength;
    } else if (sourceLed1 < firstLed) {
      sourceLed1 += sectionLength;
    }
    if (sourceLed2 > lastLed) {
      sourceLed2 -= sectionLength;
    } else if (sourceLed2 < firstLed) {
      sourceLed2 += sectionLength;
    }

    #if DEBUG >= 4
    Serial.print("LEDs ");
    Serial.print(targetLed1);
    Serial.print("/");
    Serial.print(targetLed2);
    Serial.print(" from ");
    Serial.print(sourceLed1);
    Serial.print("/");
    Serial.println(sourceLed2);
    #endif

    leds[targetLed1] = leds[sourceLed1];
    leds[targetLed2] = leds[sourceLed2];
  }

  // fill the remaining LEDs:
  #if DEBUG == 4
  Serial.print("Filling LEDs: ");
  #endif
  for (uint8_t i = hopsPerShow; i > 0; i--) {
    int16_t targetLed1, targetLed2;
    if ((!directionInverted && !directionTmpInverted) || (directionInverted && directionTmpInverted)) {
      targetLed1 = firstLed + ignitionPoint + (i - 1);
      targetLed2 = firstLed + ignitionPoint - (i - 1);
    } else {
      targetLed1 = firstLed + ignitionPoint + sectionLength/2 - (i - 1);
      targetLed2 = firstLed + ignitionPoint + sectionLength/2 + (i - 1);
    }

    if (targetLed1 > lastLed) {
      targetLed1 -= sectionLength;
    } else if (targetLed1 < firstLed) {
      targetLed1 += sectionLength;
    }
    if (targetLed2 > lastLed) {
      targetLed2 -= sectionLength;
    } else if (targetLed2 < firstLed) {
      targetLed2 += sectionLength;
    }

    #if DEBUG == 4
    Serial.print(targetLed1);
    Serial.print("/");
    Serial.print(targetLed2);
    Serial.print(", ");
    #endif

    if (fillLedWithNewColor(targetLed1, targetLed2)) {
      ignitionCounter++;
    } else {
      ignitionCounter = 0;
    }
  }
  #if DEBUG == 4
  Serial.println();
  #endif

  if (ignitionCounter >= sectionLength/2 + flankLength) { // <--------- sectionLength could cause problems (used to be lastLed-firstLed+1)
    ignitionCounter = 0;
    igniting = false;
    #if DEBUG >= 3
    Serial.println("ignition is over. ");
    #endif
  }

}

void ShdWs2812bStrip::setNewColor(uint8_t _newRed, uint8_t _newGreen, uint8_t _newBlue){

  #if DEBUG >= 1
  Serial.print("WS2812b: New color: ");
  Serial.print(_newRed);
  Serial.print(", ");
  Serial.print(_newGreen);
  Serial.print(", ");
  Serial.print(_newBlue);
  Serial.print(", old set point: ");
  Serial.print(setPoint[0]);
  Serial.print(",");
  Serial.print(setPoint[1]);
  Serial.print(",");
  Serial.print(setPoint[2]);
  #endif

  // avoid turning on with 255,255,255 via color
  if (setPoint[0] == 0 && setPoint[1] == 0 && setPoint[2] == 0 && _newRed == 255 && _newGreen == 255 && _newBlue == 255) {
    _newRed = savedValue[0];
    _newGreen = savedValue[1];
    _newBlue = savedValue[2];
  }

  if (_newRed == 0 && _newBlue == 0 && _newGreen == 0) {
    directionTmpInverted = true;
  } else {
    directionTmpInverted = false;
    savedValue[0] = _newRed;
    savedValue[1] = _newGreen;
    savedValue[2] = _newBlue;
  }

  #if DEBUG >= 2
  Serial.print(", saved values: ");
  Serial.print(savedValue[0]);
  Serial.print(",");
  Serial.print(savedValue[1]);
  Serial.print(",");
  Serial.print(savedValue[2]);
  #endif

  // save new values to set point. bit shift to have a comma number
  setPoint[0] = _newRed << 8;
  setPoint[1] = _newGreen << 8;
  setPoint[2] = _newBlue << 8;

  for (uint8_t i = 0; i < 3; i++) {
    delta[i] = ((int32_t)(setPoint[i] - shownValue[i])) / flankLength;
  }

  #if DEBUG >= 2
  Serial.print(", deltas: ");
  Serial.print(delta[0]);
  Serial.print(",");
  Serial.print(delta[1]);
  Serial.print(",");
  Serial.print(delta[2]);
  Serial.println();
  #endif

  // if mqtt client is connected, publish current status:
  if (mqttClient.connected()) {
    if (setPoint[0] == 0 && setPoint[1] == 0 && setPoint[2] == 0) {
      mqttClient.publish(pubTopicState, "0");
    } else {
      mqttClient.publish(pubTopicState, "1");
    }
    #if DEBUG >= 2
    Serial.println("MQTT: State published.");
    #endif

    // publish new color:
    clearPayloadBuffer();
    snprintf(payloadBuffer, 50, "%d,%d,%d", setPoint[0] >> 8, setPoint[1] >> 8, setPoint[2] >> 8);
    mqttClient.publish(pubTopicColor, payloadBuffer);
    #if DEBUG >= 2
    Serial.print("MQTT: new color published: ");
    Serial.println(payloadBuffer);
    #endif

    // publish brightness:
    clearPayloadBuffer();
    uint16_t brightness = (max(max(setPoint[0], setPoint[1]), setPoint[2]) / 652);
    snprintf(payloadBuffer, 50, "%d", brightness);
    mqttClient.publish(pubTopicBrightness, payloadBuffer);
    #if DEBUG >= 2
    Serial.println("MQTT: rightness published.");
    #endif
  }

  // set igniting flag to start effect:
  igniting = true;
}

bool ShdWs2812bStrip::fillLedWithNewColor(uint16_t _ledIndex) {
  bool flankOver = true;
  #if DEBUG >= 5
  Serial.print("LED ");
  Serial.print(_ledIndex);
  Serial.print(": ");
  #endif

  for (uint8_t i = 0; i < 3; i++) {
    if (abs(shownValue[i] - setPoint[i]) <= abs(delta[i])) {
      shownValue[i] = setPoint[i];
    } else {
      shownValue[i] += delta[i];
      flankOver = false;          // TODO Fehler?!
    }
    #if DEBUG >= 5
    Serial.print(shownValue[i]);
    Serial.print(", ");
    #endif
    leds[_ledIndex][i] = gammaCorrection[shownValue[i] >> 8];
  }
  #if DEBUG >= 5
  Serial.print("flankOver: ");
  Serial.println(flankOver);
  #endif
  return flankOver;
}

bool ShdWs2812bStrip::fillLedWithNewColor(uint16_t _ledIndex1, uint16_t _ledIndex2) {
  bool flankOver = true;
  #if DEBUG >= 5
  Serial.print("LED ");
  Serial.print(_ledIndex1);
  Serial.print(" and ");
  Serial.print(_ledIndex2);
  Serial.print(": ");
  #endif

  for (uint8_t i = 0; i < 3; i++) {
    if (abs(shownValue[i] - setPoint[i]) <= abs(delta[i])) {
      shownValue[i] = setPoint[i];
    } else {
      shownValue[i] += delta[i];
      flankOver = false;          // TODO Fehler?!
    }
    #if DEBUG >= 5
    Serial.print(shownValue[i]);
    Serial.print(", ");
    #endif
    leds[_ledIndex1][i] = shownValue[i] >> 8;
    leds[_ledIndex2][i] = shownValue[i] >> 8;
  }
  #if DEBUG >= 5
  Serial.print("flankOver: ");
  Serial.println(flankOver);
  #endif
  return flankOver;
}

bool ShdWs2812bStrip::handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length){
  #if DEBUG >= 1
  Serial.print("MQTT: message received. Topic: ");
  Serial.print(_topic);
  Serial.print(", payload: ");
  Serial.println((char*)_payload);
  #endif

  if (strcmp(_topic, subTopicColor) == 0) {
    setNewColor(atoi(strtok((char*)_payload, ",")), atoi(strtok(NULL, ",")), atoi(strtok(NULL, ",")));
  } else if (strcmp(_topic, subTopicState) == 0) {
    if (_payload[0] == 0x30) { // ASCII "0"
      setNewColor(0,0,0);
    } else if (_payload[0] == 0x31) { // ASCII "1" // this topic is being ignored since there's always a setColor command sent after the setState on command
      // setNewColor(savedValue[0], savedValue[1], savedValue[2]);
    } else {
      return false; // Returning true since the topic is right
    }
  } else {
    return false;
  }
  return true;
}

void ShdWs2812bStrip::clearPayloadBuffer(){
  for (uint8_t i = 0; i < 50; i++) {
    payloadBuffer[i] = 0;
  }
}

void ShdWs2812bStrip::timer5msHandler(){
  if (sectionNumber == 1) {
    show();
  }
}

void ShdWs2812bStrip::callIgnitionFunction(){
  if (direction == IGNITION_SINGLE_FORWARD || direction == IGNITION_SINGLE_BACKWARD ) {
    igniteSingleDir();
  } else {
    igniteBothDir();
  }
}

void ShdWs2812bStrip::resubscribe() {
  mqttClient.subscribe(subTopicColor, 0);
  Serial.print("WS2812b: Subscribed to ");
  Serial.println(subTopicColor);

  mqttClient.subscribe(subTopicState, 0);
  Serial.print("WS2812b: Subscribed to ");
  Serial.println(subTopicState);
}
