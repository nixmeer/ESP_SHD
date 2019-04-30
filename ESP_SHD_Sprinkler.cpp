#include "ESP_SHD_Sprinkler.h"

uint8_t ShdSprinkler::sprinklerCount = 0;

ShdSprinkler::ShdSprinkler(uint8_t _pin, bool _lowActive)
  : pin(_pin), lowActive(_lowActive) {

  sprinklerCount++;
  sprinklerNumber = sprinklerCount;

  for (uint8_t i = 0; i < 10; i++) {
    buffer[i] = 0;
  }

  Serial.print("DEBUG: ");
  Serial.println(DEBUG);

  pinMode(pin, OUTPUT);
  setOutput(LOW);
  targetStatus = false;
  currentStatus = false;

  durationTicks = 5 * 60 * 200;

  snprintf(subTopicSetActive, 50, "%s/Sprinkler/%d/setActive", name, sprinklerNumber);
  snprintf(pubTopicGetActive, 50, "%s/Sprinkler/%d/getActive", name, sprinklerNumber);
  snprintf(pubTopicGetInUse, 50, "%s/Sprinkler/%d/getInUse", name, sprinklerNumber);
  snprintf(subTopicSetDuration, 50, "%s/Sprinkler/%d/setDuration", name, sprinklerNumber);
  snprintf(pubTopicGetDuration, 50, "%s/Sprinkler/%d/getDuration", name, sprinklerNumber);
  snprintf(pubTopicGetRemainingDuration, 60, "%s/Sprinkler/%d/getRemainingDuration", name, sprinklerNumber);

  resubpub();
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
      mqttClient.publish(pubTopicGetRemainingDuration, buffer);
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
      setOutput(LOW);
      currentStatus = false;
      tickCounter = 0;
      mqttClient.publish(pubTopicGetInUse, "0");
      mqttClient.publish(pubTopicGetActive, "0");
    } else {
      setOutput(HIGH);
      currentStatus = true;
      mqttClient.publish(pubTopicGetInUse, "1");
    }
  }

}

void ShdSprinkler::resubpub() {

  mqttClient.subscribe(subTopicSetActive, 0);
  #if DEBUG >= 1
  Serial.print("SPRINKLER: No. ");
  Serial.print(sprinklerNumber);
  Serial.print(" subscribed to ");
  Serial.println(subTopicSetActive);
  #endif

  mqttClient.subscribe(subTopicSetDuration, 0);
  #if DEBUG >= 1
  Serial.print("SPRINKLER: No. ");
  Serial.print(sprinklerNumber);
  Serial.print(" subscribed to ");
  Serial.println(subTopicSetDuration);
  #endif

  if (targetStatus) {
    mqttClient.publish(pubTopicGetActive, "1");
  } else {
    mqttClient.publish(pubTopicGetActive, "0");
  }

  if (currentStatus) {
    mqttClient.publish(pubTopicGetInUse, "1");
    itoa((durationTicks-tickCounter)/200, buffer, 10);
    mqttClient.publish(pubTopicGetRemainingDuration, buffer);
  } else {
    mqttClient.publish(pubTopicGetInUse, "0");
  }

  itoa(durationTicks/200, buffer, 10);
  mqttClient.publish(pubTopicGetDuration, buffer);
}

bool ShdSprinkler::handleMqttRequest(char* _topic, byte* _payload, uint16_t _length) {
  if (strcmp(_topic, subTopicSetActive) == 0) {
    if (_payload[0] == 0x30) {
      targetStatus = false;
      mqttClient.publish(pubTopicGetActive, "0");
      #if DEBUG > 0
      Serial.println("SPRINKLER: targetStatus: false");
      #endif
    } else if (_payload[0] == 0x31) {
      targetStatus = true;
      mqttClient.publish(pubTopicGetActive, "1");
      #if DEBUG > 0
      Serial.println("SPRINKLER: targetStatus: true");
      #endif
    } else {
      #if DEBUG > 0
      Serial.println("SPRINKLER: wrong payload.");
      #endif
    }
  } else if (strcmp(_topic, subTopicSetDuration) == 0) {
    durationTicks = atoi((char*)_payload) * 200;
    itoa(durationTicks/200, buffer, 10);
    mqttClient.publish(pubTopicGetDuration, buffer);
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
      Serial.println("SPRINKLER: turned on");
    } else {
      Serial.println("SPRINKLER: turned off");
    }
    #endif
  } else {
    digitalWrite(pin, _target);
    #if DEBUG > 0
    if (_target) {
      Serial.println("SPRINKLER: turned on");
    } else {
      Serial.println("SPRINKLER: turned off");
    }
    #endif
  }
}
