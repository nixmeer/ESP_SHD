#include "ESP_SHD_WindowBlindRelay.h"
#include <FunctionalInterrupt.h>

uint8_t ShdWindowBlindRelay::windowBlindRelayCount = 0;

ShdWindowBlindRelay::ShdWindowBlindRelay(uint8_t _relayUpPin, uint8_t _relayDownPin, uint8_t _fullClosingTimeS, bool _relayLowActive) 
    : ShdWindowBlindRelay(_relayUpPin, _relayDownPin, _fullClosingTimeS, _relayLowActive, 0, 0, false) {

}

ShdWindowBlindRelay::ShdWindowBlindRelay(uint8_t _relayUpPin, uint8_t _relayDownPin, uint8_t _fullClosingTimeS, bool _relayLowActive, uint8_t _buttonUpPin, uint8_t _buttonDownPin, bool _buttonsLowActive) 
    : relayUpPin(_relayUpPin), relayDownPin(_relayDownPin), fullClosingTicks(_fullClosingTimeS*200), relayLowActive(_relayLowActive), buttonUpPin(_buttonUpPin), buttonDownPin(_buttonDownPin), buttonsLowActive(_buttonsLowActive) {

    windowBlindRelayNumber = ++windowBlindRelayCount;

    currentMovementStatus = NOT_MOVING;
    targetMovementStatus = NOT_MOVING;
    lastMovementStatus = NOT_MOVING;
    updateOutputs();
    overRunTicks = OVERRUN_TICKS;
    overRunTickCounter = 0;
    minTicksForMovementchange = TICKS_BEFORE_MOVEMENT_CHANGE;
    publishedTicksAgo = MQTT_PUBLISH_INTERVAL_NOT_MOVING_TICKS;

    currentPositionTicks = 0;

    buttonUpPressed = false;
    buttonDownPressed = false;
    interruptOccured = false;

    snprintf(subTopicTargetPosition, TOPIC_LENGTH, "%s/WindowBlindRelay/%d/setTarget", name, windowBlindRelayNumber);
    snprintf(subTopicHoldPosition, TOPIC_LENGTH, "%s/WindowBlindRelay/%d/setHold", name, windowBlindRelayNumber);
    snprintf(pubTopicTargetPosition, TOPIC_LENGTH, "%s/WindowBlindRelay/%d/getTarget", name, windowBlindRelayNumber);
    snprintf(pubTopicCurrentPosition, TOPIC_LENGTH, "%s/WindowBlindRelay/%d/getCurrentPosition", name, windowBlindRelayNumber);
    snprintf(pubTopicMovingStatus, TOPIC_LENGTH, "%s/WindowBlindRelay/%d/getMovingStatus", name, windowBlindRelayNumber);

    mqttSubscribe(this, subTopicTargetPosition);
    mqttSubscribe(this, subTopicHoldPosition);

    pinMode(relayUpPin, OUTPUT);
    pinMode(relayDownPin, OUTPUT);

    setRelay(relayUpPin, false);
    setRelay(relayDownPin, false);

    if(buttonUpPin != buttonDownPin) {
        Serial.println("WBR: configuring buttons...");
        pinMode(buttonUpPin, INPUT_PULLUP);
        attachInterrupt(buttonUpPin, std::bind(&ShdWindowBlindRelay::buttonUpInterrupt, this), CHANGE);
        pinMode(buttonDownPin, INPUT_PULLUP);
        attachInterrupt(buttonDownPin, std::bind(&ShdWindowBlindRelay::buttonDownInterrupt, this), CHANGE);
    }

    #if DEBUG > 0
    Serial.print("WBR: Successfully created WindowBlindRelay no. ");
    Serial.println(windowBlindRelayNumber);
    Serial.print("WBR: Subscribed to ");
    Serial.println(subTopicTargetPosition);
    Serial.print("WBR: Subscribed to ");
    Serial.println(subTopicHoldPosition);
    Serial.print("WBR: Publishes to ");
    Serial.println(pubTopicTargetPosition);
    Serial.print("WBR: Publishes to ");
    Serial.println(pubTopicCurrentPosition);
    Serial.print("WBR: Publishes to ");
    Serial.println(pubTopicMovingStatus);
    #endif
}

void ShdWindowBlindRelay::buttonUpInterrupt() {
    // Debounce:
    uint32_t currentMillis = millis();
    if(currentMillis - lastUpInterrupt < DEBOUNCE_MS) {
        return; // Ignore this interrupt;
    }
    lastUpInterrupt = currentMillis;

    interruptOccured = true;

    // Check for logic state:
    uint8_t summe = 0;
    for (uint8_t i = 0; i < NUMBER_OF_LOGIC_CHECKS; i++) {
        summe += digitalRead(buttonUpPin);
    }

    // If currently moving, stop movement by leaving buttonDownPressed = false:
    if (currentMovementStatus != NOT_MOVING) {
        return;
    }

    // If  not moving, check for lowActive buttons and set buttonUpPressed accordingly (leaving it false when it is "unpressed"):
    if (!buttonsLowActive) {
        if(summe > (NUMBER_OF_LOGIC_CHECKS/2)) {
            buttonUpPressed = true;
        }
    } else {
        if(summe < (NUMBER_OF_LOGIC_CHECKS/2)) {
            buttonUpPressed = true;
        }
    }
}

void ShdWindowBlindRelay::buttonDownInterrupt() {
    // Debounce:
    uint32_t currentMillis = millis();
    if(currentMillis - lastDownInterrupt < DEBOUNCE_MS) {
        return; // Ignore this interrupt;
    }
    lastDownInterrupt = currentMillis;

    interruptOccured = true;

    // Check for logic state:
    uint8_t summe = 0;
    for (uint8_t i = 0; i < NUMBER_OF_LOGIC_CHECKS; i++) {
        summe += digitalRead(buttonDownPin);
    }

    // If currently moving, stop movement by leaving buttonDownPressed = false:
    if (currentMovementStatus != NOT_MOVING) {
        return;
    }
    
    // If  not moving, check for lowActive buttons and set buttonUpPressed accordingly (leaving it false when it is "unpressed"):
    if (!buttonsLowActive) {
        if(summe > (NUMBER_OF_LOGIC_CHECKS/2)) {
            buttonDownPressed = true;
        }
    } else {
        if(summe < (NUMBER_OF_LOGIC_CHECKS/2)) {
            buttonDownPressed = true;
        }
    }
}

void ShdWindowBlindRelay::timer5msHandler() {
    #if DEBUG > 4
    Serial.print("WBR: currentPositionTicks = ");
    Serial.print(currentPositionTicks);
    Serial.print(", targetPositionTicks = ");
    Serial.print(targetPositionTicks);
    Serial.print(", targetMovementStatus = ");
    switch (targetMovementStatus)
    {
    case MOVING_UP:
        Serial.print("up");
        break;
    case MOVING_DOWN:
        Serial.print("down");
        break;
    case NOT_MOVING:
        Serial.print("not");
        break;
    default:
        Serial.print("error");
        break;
    }
    Serial.print(", currentMovementStatus = ");
    switch (currentMovementStatus)
    {
    case MOVING_UP:
        Serial.print("up");
        break;
    case MOVING_DOWN:
        Serial.print("down");
        break;
    case NOT_MOVING:
        Serial.print("not");
        break;
    default:
        Serial.print("error");
        break;
    }
    Serial.println();
    #endif

    processButtonStates();
    updateCurrentPositionTicks();
    updateTargetMovementStatus();
    updateCurrentMovementStatus();
    updateOutputs();

    checkForPublishing();
}

void ShdWindowBlindRelay::checkForPublishing() {
    if (currentMovementStatus != NOT_MOVING && publishedTicksAgo >= MQTT_PUBLISH_INTERVAL_MOVING_TICKS) {
        republish();
        publishedTicksAgo = 0;
    } else if (currentMovementStatus == NOT_MOVING && publishedTicksAgo >= MQTT_PUBLISH_INTERVAL_NOT_MOVING_TICKS) {
        republish();
        publishedTicksAgo = 0;
    }
    
    publishedTicksAgo++;
}

void ShdWindowBlindRelay::processButtonStates() {
    if (!interruptOccured){
        return;
    }

    #if DEBUG > 2
    Serial.print("WBR: Interrupt: ");
    #endif

    if (buttonUpPressed && !buttonDownPressed) {
        // MOVE UP:
        setTargetPercentage(100);
        #if DEBUG > 2
        Serial.print("button up.");
        #endif
    } else if (!buttonUpPressed && buttonDownPressed) {
        // MOVE DOWN:
        setTargetPercentage(0);
        #if DEBUG > 2
        Serial.print("button down.");
        #endif
    } else { 
        // STOP:
        switch (currentMovementStatus)
        {
        case MOVING_UP:
            if (currentPositionTicks < fullClosingTicks) {
                targetPositionTicks = currentPositionTicks + 1;
            }
            break;
        case MOVING_DOWN:
            if (currentPositionTicks > 0) {
                targetPositionTicks = currentPositionTicks - 1;
            }
            break;
        case NOT_MOVING:
            targetPositionTicks = currentPositionTicks;
            break;
        default:
            break;
        }
        #if DEBUG > 2
        Serial.print("stop.");
        #endif
    }
    #if DEBUG > 2
    Serial.println();
    #endif
    
    // Reset flags:
    interruptOccured = false;
    buttonDownPressed = false;
    buttonUpPressed = false;
}

void ShdWindowBlindRelay::setTargetPercentage(int _newTargetPosition) {
    if (_newTargetPosition > 100) {
        _newTargetPosition = 100;
    } else if (_newTargetPosition < 0) {
        _newTargetPosition = 0;
    }
    
    targetPositionTicks = (_newTargetPosition * fullClosingTicks) / 100.0;
    #if DEBUG > 0
    Serial.print("WBR: New target position: ");
    Serial.print(_newTargetPosition);
    Serial.print(" (");
    Serial.print(targetPositionTicks);
    Serial.println(")");
    #endif
}

void ShdWindowBlindRelay::updateTargetMovementStatus() {
    #if DEBUG > 4
    Serial.print("WBR: currentPositionTicks: ");
    Serial.print(currentPositionTicks);
    Serial.print(", targetPositionTicks: ");
    Serial.print(targetPositionTicks);
    Serial.println();
    #endif

    // If NOT_MOVING, then only begin moving if target position is more than TICKS_BEFORE_MOVEMENT_CHANGE ticks away:
    if (targetMovementStatus == NOT_MOVING) {
        if ((int16_t)(currentPositionTicks) < (int16_t)(targetPositionTicks - TICKS_BEFORE_MOVEMENT_CHANGE)) {
            targetMovementStatus = MOVING_UP; 
        } else if((int16_t)(currentPositionTicks) > (int16_t)(targetPositionTicks + TICKS_BEFORE_MOVEMENT_CHANGE)) {
            targetMovementStatus = MOVING_DOWN; 
        }
    } else { // If moving, keep moving until target position is reached:
        if (currentPositionTicks < targetPositionTicks) {
            targetMovementStatus = MOVING_UP; 
        } else if(currentPositionTicks > targetPositionTicks + TICKS_BEFORE_MOVEMENT_CHANGE) {
            targetMovementStatus = MOVING_DOWN; 
        } else {

            // Keep targetMovementStatus at both ends (fully opened or fully closed)
            // for overRunTicks:
            if (overRunTickCounter < overRunTicks && (targetPositionTicks == fullClosingTicks || targetPositionTicks == 0)) {
                overRunTickCounter++;
                return;
            }
            
            overRunTickCounter = 0;
            targetMovementStatus = NOT_MOVING;
        }
    }
}

void ShdWindowBlindRelay::updateOutputs() {
    if (lastMovementStatus == currentMovementStatus) {
        return;
    }
    lastMovementStatus = currentMovementStatus;
    republish();

    if (currentMovementStatus == MOVING_UP) {
        setRelay(relayUpPin, true);
        setRelay(relayDownPin, false);
    } else if (currentMovementStatus == MOVING_DOWN) {
        setRelay(relayUpPin, false);
        setRelay(relayDownPin, true);
    } else {
        setRelay(relayUpPin, false);
        setRelay(relayDownPin, false);
    }
}

void ShdWindowBlindRelay::setRelay(uint8_t _relayPin, bool _status) {
    if (!relayLowActive){
        Serial.print("WBR: setting pin ");
        Serial.print(_relayPin);
        Serial.print(" to ");
        Serial.println(_status);
        digitalWrite(_relayPin, _status);
    } else {
        Serial.print("WBR: setting pin ");
        Serial.print(_relayPin);
        Serial.print(" to ");
        Serial.println(!_status);
        digitalWrite(_relayPin, !_status);
    }
}

void ShdWindowBlindRelay::updateCurrentMovementStatus() {
    ticksSinceMovementChanged++;
    
    if((currentMovementStatus != targetMovementStatus) && (ticksSinceMovementChanged > minTicksForMovementchange)) {
        ticksSinceMovementChanged = 0;
        if(currentMovementStatus == NOT_MOVING) {
            currentMovementStatus = targetMovementStatus; 
        } else {
            currentMovementStatus = NOT_MOVING; 
        }
    }
}

void ShdWindowBlindRelay::updateCurrentPositionTicks() {
    switch (currentMovementStatus) {
        case MOVING_UP:
            if(currentPositionTicks < fullClosingTicks) {
                currentPositionTicks++;
            }
            break;
        case MOVING_DOWN:
            if(currentPositionTicks > 0) {
                currentPositionTicks--;
            }
            break;
        case NOT_MOVING:
            if(currentPositionTicks <= fullClosingTicks && currentPositionTicks >= 0) {
                currentPositionTicks = currentPositionTicks;
            }
            break;
    }
    
}

bool ShdWindowBlindRelay::handleMqttRequest(char* _topic, byte* _payload, uint16_t _length){
    #if DEBUG > 0
    Serial.print("WBR: MQTT request: ");
    Serial.print(_topic);
    Serial.print(", ");
    Serial.print((char*)_payload);
    Serial.print(", ");
    Serial.println(_length);
    #endif

    if (strcmp(_topic, subTopicTargetPosition) == 0) {
        setTargetPercentage(atoi((char*)_payload));
    } else if (strcmp(_topic, subTopicHoldPosition) == 0) {
        targetPositionTicks = currentPositionTicks;
    } else {
        return false;
    }
    
    republish();
    return true;
}

void ShdWindowBlindRelay::republish() {
    // Publish movementStatus:
    switch (targetMovementStatus) {
    case MOVING_UP:
        mqttPublish(pubTopicMovingStatus, "INCREASING");
        break;
    case MOVING_DOWN:
        mqttPublish(pubTopicMovingStatus, "DECREASING");
    break;
    case NOT_MOVING:
        mqttPublish(pubTopicMovingStatus, "STOPPED");
        break;
    default:
        break;
    }
    
    // Publish current position:
    uint16_t _positionPercent = (currentPositionTicks * 100)/(fullClosingTicks);
    clearPayloadBuffer();
    snprintf(payloadBuffer, MQTT_PAYLOAD_BUFFER_SIZE, "%i", _positionPercent);
    mqttPublish(pubTopicCurrentPosition, payloadBuffer);

    // Publish target position:
    uint16_t _targetPercent = (targetPositionTicks * 100)/(fullClosingTicks);
    clearPayloadBuffer();
    snprintf(payloadBuffer, MQTT_PAYLOAD_BUFFER_SIZE, "%i", _targetPercent);
    mqttPublish(pubTopicTargetPosition, payloadBuffer);
}

void ShdWindowBlindRelay::clearPayloadBuffer() {
    for(uint16_t i = 0; i < MQTT_PAYLOAD_BUFFER_SIZE; i++) {
        payloadBuffer[i] = 0;
    }
}
