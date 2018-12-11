#include "ESP_SHD_WS2812bStrip.h"
#include "FastLED.h"

#define DEBUG 5

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

  // see if it's time to update the strip:
  uint16_t currentMillis = millis();

  bool currentlyIgniting = false;
  for (uint8_t i = 0 ; i < numberOfSections; i++) {
    if (sections[i]->igniting) {
      currentlyIgniting = true;
    }
  }
  if (!currentlyIgniting) {
    #if DEBUG >= 9
    Serial.println("No strip currently igniting.");
    #endif
    return;
  }

  uint16_t millisSinceLastUpdate = (uint16_t)(currentMillis - millisLastStripUpdate);

  if (millisLastStripUpdate >= millisStripUpdateInterval) {

    // update millisLastStripUpdate:
    while (millisLastStripUpdate >= millisStripUpdateInterval) {
      millisLastStripUpdate += millisStripUpdateInterval;
    }

    // Serial.println("TEST");

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

void ShdWs2812bStrip::initStrip(uint16_t _numberOfLeds, uint8_t _updateInterval){
  numberOfLeds = _numberOfLeds;
  if (_numberOfLeds <= MAX_NUM_OF_LEDS && _numberOfLeds > 0) {
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, numberOfLeds);
    correctlyInitialized = true;
    millisLastStripUpdate = millis();
    millisStripUpdateInterval = _updateInterval;
    Serial.println();
    Serial.print("WS2812b strip initialized.");
    Serial.println();
  }
}

ShdWs2812bStrip::ShdWs2812bStrip(uint16_t _firstLed, uint16_t _lastLed, uint16_t _ignitionPoint, ignitionDirection _ignitionDirection, uint8_t _hopsPerShow, uint8_t _flankLength,const char * _stripName)
  : ignitionPoint(_ignitionPoint){


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

  snprintf (subTopicColor, 50, "%s/Lampe/%d/setColor", name, sectionNumber);
  snprintf (pubTopicColor, 50, "%s/Lampe/%d/getColor", name, sectionNumber);
  snprintf (subTopicState, 50, "%s/Lampe/%d/setStatus", name, sectionNumber);
  snprintf (pubTopicState, 50, "%s/Lampe/%d/getStatus", name, sectionNumber);
  snprintf (pubTopicBrightness, 50, "%s/Lampe/%d/getBrightness", name, sectionNumber);

  resubscribe();

  direction = _ignitionDirection;

  igniting = false;


  for (uint8_t i = firstLed; i <= lastLed; i++) {
    leds[i] = CRGB::Black;
  }

  Serial.print("New WS2812b section no. ");
  Serial.print(sectionNumber);
  Serial.print(" registered.");
  Serial.println();
}

void ShdWs2812bStrip::igniteSingleForward(){

  for (uint16_t i = (lastLed - firstLed); i > hopsPerShow; i--) {

    // shift as many LEDs as possible. Every LED value jumps for hopsPerShow
    uint16_t targetLed = i + ignitionPoint + firstLed;
    if (targetLed > lastLed) {
      targetLed -= (lastLed - firstLed);
    } else if (targetLed < firstLed) {
      targetLed += (lastLed - firstLed);
    }

    uint16_t sourceLed = targetLed - hopsPerShow;
    if (sourceLed > lastLed) {
      sourceLed -= (lastLed - firstLed);
    } else if (sourceLed < firstLed) {
      sourceLed += (lastLed - firstLed);
    }

    #if DEBUG >= 4
    Serial.print("To ");
    Serial.print(targetLed);
    Serial.print(" from ");
    Serial.println(sourceLed);
    #endif

    leds[targetLed] = leds[sourceLed];
  }

  // fill the remaining LEDs with new color:
  #if DEBUG == 4
  Serial.print("Fill LEDs ");
  #endif
  for (uint8_t i = hopsPerShow; i > 0; i--) {

    uint16_t targetLed = i - 1 + ignitionPoint;
    if (targetLed > lastLed) {
      targetLed -= (lastLed - firstLed);
    } else if (targetLed < firstLed) {
      targetLed += (lastLed - firstLed);
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

  if (ignitionCounter > lastLed - firstLed + 1) {
    ignitionCounter = 0;
    igniting = false;
  }

}

void ShdWs2812bStrip::igniteSingleBackward(){}

void ShdWs2812bStrip::igniteBoth(){}

void ShdWs2812bStrip::setNewColor(uint8_t _newRed, uint8_t _newGreen, uint8_t _newBlue){

  #if DEBUG >= 1
  Serial.print("New color: ");
  Serial.print(_newRed);
  Serial.print(", ");
  Serial.print(_newBlue);
  Serial.print(", ");
  Serial.print(_newGreen);
  #endif

  if (_newRed == 0 && _newBlue == 0 && _newGreen == 0) {
    for (uint8_t i = 0; i < 3; i++) {
      savedValue[i] = shownValue[i] >> 8;
    }
  }

  setPoint[0] = _newRed << 8;
  setPoint[1] = _newGreen << 8;
  setPoint[2] = _newBlue << 8;

  for (uint8_t i = 0; i < 3; i++) {
    delta[i] = ((int16_t)(setPoint[i] - shownValue[i])) / flankLength;
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

  clearPayloadBuffer();
  snprintf (payloadBuffer, 50, "%d,%d,%d", setPoint[0] >> 8, setPoint[1] >> 8, setPoint[2] >> 8);
  mqttClient.publish(pubTopicColor, payloadBuffer);
  #if DEBUG >= 2
  Serial.println("MQTT: Color published.");
  #endif

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

void ShdWs2812bStrip::callIgnitionFunction(){
  if (direction == IGNITION_FORWARD) {
    igniteSingleForward();
  } else if (direction == IGNITION_BACKWARD) {
    igniteSingleBackward();
  } else {
    igniteBoth();
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
