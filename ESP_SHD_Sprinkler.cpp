#include "ESP_SHD_Sprinkler.h"

#define DEBUG 0

uint8_t ShdSprinkler::sprinklerCount = 0;

ShdSprinkler::ShdSprinkler(uint8_t _pin, bool _lowActive)
  : pin(_pin), lowActive(_lowActive) {

  sprinklerCount++;
  sprinklerNumber = sprinklerCount;

  for (uint8_t i = 0; i < 10; i++) {
    buffer[i] = 0;
  }

  targetStatus = false;
  currentStatus = false;

  pinMode(pin, OUTPUT);
  setOutput(0);

  durationTicks = 5 * 200;

  snprintf(subTopicSetActive, 50, "%s/Sprinkler/%d/setActive", name, sprinklerNumber);
  snprintf(pubTopicGetActive, 50, "%s/Sprinkler/%d/getActive", name, sprinklerNumber);
  snprintf(pubTopicGetInUse, 50, "%s/Sprinkler/%d/getInUse", name, sprinklerNumber);
  snprintf(subTopicSetDuration, 50, "%s/Sprinkler/%d/setDuration", name, sprinklerNumber);
  snprintf(pubTopicGetDuration, 50, "%s/Sprinkler/%d/getDuration", name, sprinklerNumber);
  snprintf(pubTopicGetRemainingDuration, 60, "%s/Sprinkler/%d/getRemainingDuration", name, sprinklerNumber);

  subscribe();
  republish();
}

void ShdSprinkler::timer5msHandler() {
  // deactivate, if durationTicks is over:
  if (currentStatus) {
    tickCounter++;
    if (tickCounter >= durationTicks) {
      targetStatus = false;
    }

    // update remaining duration:
    if (tickCounter - publishTickCounter > 30*200) {
      itoa((durationTicks-tickCounter)/200, buffer, 10);
      mqttPublish(pubTopicGetRemainingDuration, buffer);
      publishTickCounter = tickCounter;
      #if DEBUG > 2
      Serial.print("MQTT: Published to ");
      Serial.print(pubTopicGetRemainingDuration);
      Serial.print(": ");
      Serial.print((durationTicks-tickCounter)/200/60);
      Serial.print(":");
      Serial.println(((durationTicks-tickCounter)/200)-((durationTicks-tickCounter)/200/60)*60);
      #endif
    }
  }


  // adapt targetStatus
  if (targetStatus != currentStatus) {
    if (targetStatus == false) {
      setOutput(0);
      currentStatus = false;
      tickCounter = 0;
      mqttPublish(pubTopicGetInUse, "0");
      mqttPublish(pubTopicGetActive, "0");
    } else {
      setOutput(1);
      currentStatus = true;
      mqttPublish(pubTopicGetInUse, "1");
      itoa((durationTicks)/200, buffer, 10);
      mqttPublish(pubTopicGetRemainingDuration, buffer);
    }
  }

}

void ShdSprinkler::subscribe() {

    mqttSubscribe(this, subTopicSetActive);
    #if DEBUG >= 1
    Serial.print("SPRINKLER: No. ");
    Serial.print(sprinklerNumber);
    Serial.print(" subscribed to ");
    Serial.println(subTopicSetActive);
    #endif

    mqttSubscribe(this, subTopicSetDuration);
    #if DEBUG >= 1
    Serial.print("SPRINKLER: No. ");
    Serial.print(sprinklerNumber);
    Serial.print(" subscribed to ");
    Serial.println(subTopicSetDuration);
    #endif
}

void ShdSprinkler::republish() {

  if (targetStatus) {
    mqttPublish(pubTopicGetActive, "1");
  } else {
    mqttPublish(pubTopicGetActive, "0");
  }

  if (currentStatus) {
    mqttPublish(pubTopicGetInUse, "1");
    itoa((durationTicks-tickCounter)/200, buffer, 10);
    mqttPublish(pubTopicGetRemainingDuration, buffer);
  } else {
    mqttPublish(pubTopicGetInUse, "0");
  }

  itoa(durationTicks/200, buffer, 10);
  mqttPublish(pubTopicGetDuration, buffer);
}

bool ShdSprinkler::handleMqttRequest(char* _topic, byte* _payload, uint16_t _length) {
  if (strcmp(_topic, subTopicSetActive) == 0) {
    if (_payload[0] == 0x30) {
      targetStatus = false;
      mqttPublish(pubTopicGetActive, "0");
      #if DEBUG > 0
      Serial.println("SPRINKLER: targetStatus: false");
      #endif
    } else if (_payload[0] == 0x31) {
      targetStatus = true;
      mqttPublish(pubTopicGetActive, "1");
      #if DEBUG > 0
      Serial.println("SPRINKLER: targetStatus: true");
      #endif
    } else {
      #if DEBUG > 0
      Serial.println("SPRINKLER: wrong payload.");
      #endif
    }
  } else if (strcmp(_topic, subTopicSetDuration) == 0) {
    #if DEBUG > 3
    Serial.print("SPRINKLER: set duration to ");
    Serial.println((char*)_payload);
    Serial.println();
    #endif
    durationTicks = atoi((char*)_payload) * 200 / 10;
    itoa(durationTicks/200*10, buffer, 10);
    mqttPublish(pubTopicGetDuration, buffer);
    #if DEBUG > 0
    Serial.print("SPRINKLER: durationTicks: ");
    Serial.println(durationTicks);
    #endif
  } else {
    return false;
  }
  return true;
}

void ShdSprinkler::setOutput(bool _target) {
  if (lowActive) {
    digitalWrite(pin, !_target);
    #if DEBUG > 0
    if (_target) {
      Serial.print("SPRINKLER: No. ");
      Serial.print(sprinklerNumber);
      Serial.println(" turned on");
    } else {
      Serial.print("SPRINKLER: No. ");
      Serial.print(sprinklerNumber);
      Serial.println(" turned off");
    }
    #endif
  } else {
    digitalWrite(pin, _target);
    #if DEBUG > 0
    if (_target) {
      Serial.print("SPRINKLER: No. ");
      Serial.print(sprinklerNumber);
      Serial.println(" turned on");
    } else {
      Serial.print("SPRINKLER: No. ");
      Serial.print(sprinklerNumber);
      Serial.println(" turned off");
    }
    #endif
  }
}
