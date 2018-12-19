#include "ESP_SHD_WS2812bStrip.h"
#include "FastLED.h"
#include <FunctionalInterrupt.h>

#define DEBUG 0
#define DEBOUNCE_MILLIS 500

uint16_t ShdWs2812bStrip::millisLastStripUpdate;
uint16_t ShdWs2812bStrip::millisStripUpdateInterval;
CRGB ShdWs2812bStrip::leds[MAX_NUM_OF_LEDS];
ShdWs2812bStrip * ShdWs2812bStrip::sections[MAX_NUM_OF_SECTIONS];
uint16_t ShdWs2812bStrip::numberOfLeds = 0;
uint8_t ShdWs2812bStrip::numberOfSections = 0;
bool ShdWs2812bStrip::correctlyInitialized = false;

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
        #if DEBUG >= 1
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
  buttonDetached = false;

  // check if first and last LED number are plausible:
  if (_firstLed <= _lastLed && _firstLed > 0 && _lastLed <= numberOfLeds) {
    firstLed = _firstLed-1;
    lastLed = _lastLed-1;
  } else {
    correctlyInitialized = false;
    return;
  }

  if (_flankLength < lastLed - firstLed) {
    flankLength = _flankLength;
  } else {
    Serial.println("flankLength not correct.");
    correctlyInitialized = false;
    return;
  }

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

  snprintf (subTopicColor, 50, "%s/Lampe/%d/setColor", name, sectionNumber);
  snprintf (pubTopicColor, 50, "%s/Lampe/%d/getColor", name, sectionNumber);
  snprintf (subTopicState, 50, "%s/Lampe/%d/setStatus", name, sectionNumber);
  snprintf (pubTopicState, 50, "%s/Lampe/%d/getStatus", name, sectionNumber);
  snprintf (pubTopicBrightness, 50, "%s/Lampe/%d/getBrightness", name, sectionNumber);

  resubscribe();

  direction = _ignitionDirection;
  if (direction == IGNITION_BACKWARD) {
    directionInverted = true;
  } else {
    directionInverted = false;
  }

  igniting = false;

  for (uint8_t i = firstLed; i <= lastLed; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  Serial.print("New WS2812b section no. ");
  Serial.print(sectionNumber);
  Serial.print(" registered. It subscribed to ");
  Serial.print(subTopicState);
  Serial.print(" and ");
  Serial.print(subTopicColor);
  Serial.print(". Free stack: ");
  Serial.print(ESP.getFreeHeap());
  Serial.print(" bytes");
  Serial.println();
}

void ShdWs2812bStrip::igniteSingleDir(){

  for (uint16_t i = sectionLength - 1; i >= hopsPerShow; i--) {

    // shift as many LEDs as possible. Every LED value jumps for hopsPerShow
    int16_t targetLed;
    if (!directionInverted) {
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
    if (!directionInverted) {
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
    if (!directionInverted) {
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
    if (!directionInverted) {
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
    if (!directionInverted) {
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
    if (!directionInverted) {
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
  Serial.print("New color: ");
  Serial.print(_newRed);
  Serial.print(", ");
  Serial.print(_newGreen);
  Serial.print(", ");
  Serial.print(_newBlue);
  #endif

  if (_newRed == 0 && _newBlue == 0 && _newGreen == 0 && setPoint[0] != 0 && setPoint[1] != 0 && setPoint[2] != 0) {
    for (uint8_t i = 0; i < 3; i++) {
      savedValue[i] = shownValue[i] >> 8;
    }
    directionInverted = !directionInverted;
  } else if (_newRed != 0 && _newGreen != 0 && _newBlue != 0 && setPoint[0] == 0 && setPoint[1] == 0 && setPoint[2] == 0) {
    directionInverted = !directionInverted;
  }

  setPoint[0] = _newRed << 8;
  setPoint[1] = _newGreen << 8;
  setPoint[2] = _newBlue << 8;

  for (uint8_t i = 0; i < 3; i++) {
    delta[i] = ((int32_t)(setPoint[i] - shownValue[i])) / flankLength;
  }

  #if DEBUG >= 2
  Serial.print(". Deltas: ");
  Serial.print(delta[0]);
  Serial.print(", ");
  Serial.print(delta[1]);
  Serial.print(", ");
  Serial.print(delta[2]);
  Serial.println();
  #endif

  // publish current status:
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
  snprintf (payloadBuffer, 50, "%d,%d,%d", setPoint[0] >> 8, setPoint[1] >> 8, setPoint[2] >> 8);
  mqttClient.publish(pubTopicColor, payloadBuffer);
  #if DEBUG >= 2
  Serial.print("MQTT: new Color published: ");
  Serial.println(payloadBuffer);
  #endif

  // publish brightness:
  clearPayloadBuffer();
  uint16_t brightness = (max(max(setPoint[0], setPoint[1]), setPoint[2]) / 652);
  snprintf (payloadBuffer, 50, "%d", brightness);
  mqttClient.publish(pubTopicBrightness, payloadBuffer);
  #if DEBUG >= 2
  Serial.println("MQTT: Brightness published.");
  #endif

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
    leds[_ledIndex][i] = shownValue[i] >> 8;
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
  Serial.print("MQTT topic: ");
  Serial.print(_topic);
  Serial.print(", payload: ");
  Serial.println((char*)_payload);
  #endif

  if (strcmp(_topic, subTopicColor) == 0) {
    setNewColor(atoi(strtok((char*)_payload, ",")), atoi(strtok(NULL, ",")), atoi(strtok(NULL, ",")));
  } else if (strcmp(_topic, subTopicState) == 0) {
    if (_payload[0] == 0x30) { // ASCII "0"
      setNewColor(0,0,0);
    } else if (_payload[0] == 0x31) { // ASCII "1"
      setNewColor(savedValue[0], savedValue[1], savedValue[2]);
    } else {
      return false;
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
  if (direction == IGNITION_FORWARD) {
    igniteSingleDir();
  } else if (direction == IGNITION_BACKWARD) {
    igniteSingleDir();
  } else {
    igniteBothDir();
  }
}

void ShdWs2812bStrip::resubscribe() {
  mqttClient.subscribe(subTopicColor, 0);
  Serial.print("SHD subscribed to ");
  Serial.println(subTopicColor);

  mqttClient.subscribe(subTopicState, 0);
  Serial.print("SHD subscribed to ");
  Serial.println(subTopicState);
}
