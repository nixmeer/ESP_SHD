#include "ESP_SHD_WS2812bStrip.h"

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

  if (!igniting) {
    return;
  }

  if (currentMillis - millisLastStripUpdate >= millisStripUpdateInterval) {

    // update millisLastStripUpdate:
    while (currentMillis - millisLastStripUpdate >= millisStripUpdateInterval) {
      millisLastStripUpdate += millisStripUpdateInterval;
    }

    // update the LED strip:
    FastLED::show();

    // prepare all CRGBs for next show():
    for (uint8_t i = 0; i < numberOfSections; i++) {
      sections[i]->ignitionFunction();
    }
  }
}

void ShdWs2812bStrip::initStrip(int _numberOfLeds){
  numberOfLeds = _numberOfLeds;
  if (_numberOfLeds <= MAX_NUM_OF_LEDS) {
    FastLED::add<FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, numberOfLeds); // TODO:
    correctlyInitialized = true;
  }
}

ShdWs2812bStrip::ShdWs2812bStrip(uint16_t _firstLed, uint16_t _lastLed, uint16_t _ignitionPoint, ignitionFunction _ignitionDirection, uint8_t _hopsPerShow, uint8_t _flankLength,const char * _stripName)
  : ignitionPoint(_ignitionPoint){

  // add this object to the sections array:
  sections[numberOfSections] = this;
  numberOfSections++;

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

  snprintf (supTopic, 50, "%s/Section%d/set", name, numberOfSections);
  snprintf (supTopic, 50, "%s/Section%d/current", name, numberOfSections);

  if (_ignitionDirection == IGNITION_FORWARD) {
    ignitionFunction = igniteSingleForward;
  } else if (_ignitionDirection == IGNITION_BACKWARD) {
    ignitionFunction = igniteSingleBackward;
  } else {
    ignitionFunction = igniteBoth;
  }

  mqttClient.subscribe(subTopic, 0);

  for (uint8_t i = firstLed; i <= lastLed; i++) {
    leds[i] = CRGB::Black;
  }
}

void ShdWs2812bStrip::igniteSingleForward(){

  for (uint16_t i = (lastLed - firstLed); i > hopsPerShow; i--) {

    // shift as many LEDs as possible. Every LED value jumps for hopsPerShow
    uint16_t sourceLed = i + ignitionPoint + firstLed;
    if (sourceLed > lastLed) {
      sourceLed -= (lastLed - firstLed)
    } else if (sourceLed < firstLed) {
      sourceLed += (lastLed - firstLed);
    }

    uint16_t targetLed = sourceLed - hopsPerShow;
    if (targetLed > lastLed) {
      targetLed -= (lastLed - firstLed)
    } else if (targetLed < firstLed) {
      targetLed += (lastLed - firstLed);
    }

    leds[targetLed] = leds[sourceLed];
  }

  // fill the remaining LEDs with new color:
  for (uint8_t i = hopsPerShow-1; i >= 0; i--) {

    uint16_t sourceLed = i + ignitionPoint + 1;
    if (sourceLed > lastLed) {
      sourceLed -= (lastLed - firstLed)
    } else if (sourceLed < firstLed) {
      sourceLed += (lastLed - firstLed);
    }

    uint16_t targetLed = i + ignitionPoint;
    if (targetLed > lastLed) {
      targetLed -= (lastLed - firstLed)
    } else if (targetLed < firstLed) {
      targetLed += (lastLed - firstLed);
    }

    if (fillLedWithNewColor(sourceLed, targetLed)) {
      ignitionCounter++;
    } else {
      ignitionCounter = 0;
    }
  }

  if (ignitionCounter > lastLed - firstLed + 1) {
    igniting = false;
  }

}

void ShdWs2812bStrip::setNewColor(uint8_t _newRed, uint8_t _newGreen, uint8_t _newBlue){
  setPoint[0] = _newRed << 8;
  setPoint[1] = _newGreen << 8;
  setPoint[2] = _newBlue << 8;

  for (uint8_t i = 0; i < 3; i++) {
    delta[i] = (setPoint[i] - shownValue[i]) / flankLength;
  }

  igniting = true;
}

bool ShdWs2812bStrip::fillLedWithNewColor(uint16_t _targetLed, uint16_t _sourceLed) {
  bool flankOver = true;
  for (uint8_t i = 0; i < 3; i++) {
    if (leds[_targetLed][i] - leds[_sourceLed][i] < delta[i]) {
      leds[_targetLed][i] = setPoint[i];
    } else {
      leds[_targetLed][i] = leds[_sourceLed] + delta[i];
      flankOver false;
    }
  }

  return flankOver;
}
