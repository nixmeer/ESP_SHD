#include "ESP_SHD_WS2812bStrip.h"
#include "FastLED.h"

uint16_t ShdWs2812bStrip::millisLastStripUpdate;
uint16_t ShdWs2812bStrip::millisStripUpdateInterval;
CRGB ShdWs2812bStrip::leds[MAX_NUM_OF_LEDS];
ShdWs2812bStrip * ShdWs2812bStrip::sections[MAX_NUM_OF_SECTIONS];
uint8_t ShdWs2812bStrip::numberOfSections = 0;
bool ShdWs2812bStrip::correctlyInitialized = false;

void ShdWs2812bStrip::show(){
  if (!correctlyInitialized) {
    return;
  }

  // see if it's time to update the strip:
  uint16_t currentMillis = millis();

  if (currentMillis - millisLastStripUpdate >= millisStripUpdateInterval) {

    // update millisLastStripUpdate:
    while (currentMillis - millisLastStripUpdate >= millisStripUpdateInterval) {
      millisLastStripUpdate += millisStripUpdateInterval;
    }

    // update the LED strip:
    FastLED.show();

    // prepare all CRGBs for next show():
    for (uint8_t i = 0; i < numberOfSections; i++) {
      if (sections[i]->igniting) {
        sections[i]->*ignitionFunction();
      }
    }
  }
}

void ShdWs2812bStrip::initStrip(int _numberOfLeds){
  numberOfLeds = _numberOfLeds;
  if (_numberOfLeds <= MAX_NUM_OF_LEDS && _numberOfLeds > 0) {
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, numberOfLeds);
    correctlyInitialized = true;
  }
}

ShdWs2812bStrip::ShdWs2812bStrip(uint16_t _firstLed, uint16_t _lastLed, uint16_t _ignitionPoint, ignitionDirection _ignitionDirection, uint8_t _hopsPerShow, uint8_t _flankLength,const char * _stripName)
  : ignitionPoint(_ignitionPoint){

  // add this object to the sections array:
  sections[numberOfSections] = this;
  numberOfSections++;
  sectionNumber = numberOfSections;

  // check if first and last LED number are plausible:
  if (_firstLed < _lastLed && _firstLed > 0 && _lastLed < numberOfLeds) {
    firstLed = _firstLed-1;
    lastLed = _lastLed-1;
  } else {
    correctlyInitialized = false;
    return;
  }

  if (_flankLength < lastLed - firstLed) {
    flankLength = _flankLength;
  } else {
    correctlyInitialized = false;
    return;
  }

  // check if hopsPerShow > 0
  if (_hopsPerShow > 0 && _hopsPerShow < lastLed - firstLed) {
    hopsPerShow = _hopsPerShow;
  } else {
    correctlyInitialized = false;
    return;
  }

  snprintf (subTopicColor, 50, "%s/%d/setColor", name, sectionNumber);
  snprintf (pubTopicColor, 50, "%s/%d/getColor", name, sectionNumber);
  snprintf (subTopicState, 50, "%s/%d/setStatus", name, sectionNumber);
  snprintf (pubTopicState, 50, "%s/%d/getStatus", name, sectionNumber);
  snprintf (pubTopicBrightness, 50, "%s/%d/getBrightness", name, sectionNumber);

  mqttClient.subscribe(subTopicColor, 0);
  mqttClient.subscribe(subTopicState, 0);

  if (_ignitionDirection == IGNITION_FORWARD) {
    ignitionFunction = &ShdWs2812bStrip::igniteSingleForward;
  } else if (_ignitionDirection == IGNITION_BACKWARD) {
    ignitionFunction = &ShdWs2812bStrip::igniteSingleBackward;
  } else {
    ignitionFunction = &ShdWs2812bStrip::igniteBoth;
  }
  igniting = false;


  for (uint8_t i = firstLed; i <= lastLed; i++) {
    leds[i] = CRGB::Black;
  }
}

void ShdWs2812bStrip::igniteSingleForward(){

  for (uint16_t i = (lastLed - firstLed); i > hopsPerShow; i--) {

    // shift as many LEDs as possible. Every LED value jumps for hopsPerShow
    uint16_t sourceLed = i + ignitionPoint + firstLed;
    if (sourceLed > lastLed) {
      sourceLed -= (lastLed - firstLed);
    } else if (sourceLed < firstLed) {
      sourceLed += (lastLed - firstLed);
    }

    uint16_t targetLed = sourceLed - hopsPerShow;
    if (targetLed > lastLed) {
      targetLed -= (lastLed - firstLed);
    } else if (targetLed < firstLed) {
      targetLed += (lastLed - firstLed);
    }

    leds[targetLed] = leds[sourceLed];
  }

  // fill the remaining LEDs with new color:
  for (uint8_t i = hopsPerShow-1; i >= 0; i--) {

    uint16_t targetLed = i + ignitionPoint;
    if (targetLed > lastLed) {
      targetLed -= (lastLed - firstLed);
    } else if (targetLed < firstLed) {
      targetLed += (lastLed - firstLed);
    }

    if (fillLedWithNewColor(targetLed)) {
      ignitionCounter++;
    } else {
      ignitionCounter = 0;
    }
  }

  if (ignitionCounter > lastLed - firstLed + 1) {
    ignitionCounter = 0;
    igniting = false;
  }

}

void ShdWs2812bStrip::setNewColor(uint8_t _newRed, uint8_t _newGreen, uint8_t _newBlue){

  if (_newRed == 0 && _newBlue == 0 && _newGreen == 0) {
    for (uint8_t i = 0; i < 3; i++) {
      savedValue[i] = shownValue[i] >> 8;
    }
  }

  setPoint[0] = _newRed << 8;
  setPoint[1] = _newGreen << 8;
  setPoint[2] = _newBlue << 8;

  for (uint8_t i = 0; i < 3; i++) {
    delta[i] = (setPoint[i] - shownValue[i]) / flankLength;
  }

  // publish current status:
  if (setPoint[0] == 0 && setPoint[1] == 0 && setPoint[2] == 0) {
    mqttClient.publish(pubTopicState, "0");
  } else {
    mqttClient.publish(pubTopicState, "1");
  }

  clearPayloadBuffer();
  snprintf (payloadBuffer, 50, "%d,%d,%d", setPoint[0] >> 8, setPoint[1] >> 8, setPoint[2] >> 8);
  mqttClient.publish(pubTopicColor, payloadBuffer);

  clearPayloadBuffer();
  uint16_t brightness = (max(max(setPoint[0], setPoint[1]), setPoint[2]) / 652);
  snprintf (payloadBuffer, 50, "%d", brightness);
  mqttClient.publish(pubTopicBrightness, payloadBuffer);

  igniting = true;
}

bool ShdWs2812bStrip::fillLedWithNewColor(uint16_t _ledIndex) {
  bool flankOver = true;
  for (uint8_t i = 0; i < 3; i++) {
    if (abs(shownValue[i] - setPoint[i]) <= abs(delta[i])) {
      shownValue[i] = setPoint[i];
    } else {
      shownValue[i] += delta[i];
      flankOver = false;          // TODO Fehler?!
    }
    leds[_ledIndex][i] = shownValue[i] >> 8;
  }
  return flankOver;
}

bool ShdWs2812bStrip::handleMqttRequest(char* _topic, unsigned char* _payload, unsigned int _length){
  if (strcmp(_topic, subTopicColor) == 0) {
    setNewColor(atoi(strtok((char*)_payload, ",")), atoi(strtok(NULL, ",")), atoi(strtok(NULL, ",")));
  } else if (strcmp(_topic, subTopicState) == 0) {
    if (strcmp((char*)_payload, "0") == 0) {
      setNewColor(0,0,0);
    } else if (strcmp((char*)_payload, "1")) {
      setNewColor(savedValue[0], savedValue[1], savedValue[2]);
    }
  } {
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
