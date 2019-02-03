#include "ESP_SHD_Button.h"
uint8_t ShdButton::numberOfButtons = 0;


ShdButton::ShdButton(uint8_t _pin, bool _lowActive, uint32_t _millisDebounce, uint32_t _millisLongClick, uint32_t _millisMultiClick){
  pin = _pin;
  lowActive = _lowActive;

  numberOfButtons++;

  clickCounter = 0;
  currentlyClicked = false;

  firstClickTime = 0;
  lastInterruptTime = 0;
  millisDebounce = _millisDebounce;
  millisLongClick = _millisLongClick;
  millisMultiClick = _millisMultiClick;

  pinMode(pin, INPUT_PULLUP);
  attachInterrupt(pin, std::bind(&ShdButton::handleInterrupt, this), CHANGE);

  snprintf (pubTopic, 50, "%s/Button/%d", name, numberOfButtons);

  // debug output:
  Serial.print("New button registered. It publishes to ");
  Serial.print(pubTopic);
  Serial.println();
}

void ShdButton::handleInterrupt(){
  uint32_t currentMillis = millis();
  if (digitalRead(pin) != lowActive && currentMillis - lastInterruptTime > millisDebounce) { // Button is now clicked and not in bounce time
    lastInterruptTime = currentMillis;
    currentlyClicked = true;
    clickCounter++;
    if (currentMillis - firstClickTime > millisMultiClick) {
      firstClickTime = currentMillis;
    }
  } else {
    currentlyClicked = false;
  }
}

void ShdButton::timer5msHandler(){
  if (clickCounter > 0) {
    uint32_t currentMillis = millis();
    if (clickCounter == 1 && currentlyClicked == false && currentMillis - firstClickTime > millisMultiClick) {
      if (mqttClient.publish(pubTopic, "1")) {
        #if DEBUG > 1
        Serial.print("Button event has been published to ");
        Serial.print(pubTopic);
        Serial.print(" at ");
        Serial.print(millis());
        Serial.print(" value: ");
        Serial.println("L");
        #endif
      }
      clickCounter = 0;
    } else if (clickCounter >= 2 && currentlyClicked == false && currentMillis - firstClickTime > millisMultiClick){
      if (mqttClient.publish(pubTopic, "2")) {
        #if DEBUG > 1
        Serial.print("Button event has been published to ");
        Serial.print(pubTopic);
        Serial.print(" at ");
        Serial.print(millis());
        Serial.print(" value: ");
        Serial.println("L");
        #endif
      }
      clickCounter = 0;
    } else if (clickCounter == 1 && currentlyClicked == true && currentMillis - firstClickTime > millisLongClick) {
      if (mqttClient.publish(pubTopic, "L")) {
        #if DEBUG > 1
        Serial.print("Button event has been published to ");
        Serial.print(pubTopic);
        Serial.print(" at ");
        Serial.print(millis());
        Serial.print(" value: ");
        Serial.println("L");
        #endif
      }
      clickCounter = 0;
    }

  } else {
    return;
  }
}

bool ShdButton::handleMqttRequest(char *_topic, unsigned char *_payload, uint16_t _length){
  return false;
}

void ShdButton::resubpub(){
  return;
}
