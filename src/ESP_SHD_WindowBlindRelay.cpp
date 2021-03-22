#include "ESP_SHD_WindowBlindRelay.h"
#include <FunctionalInterrupt.h>

uint8_t ShdWindowBlindRelay::windowBlindRelayCount = 0;

ShdWindowBlindRelay::ShdWindowBlindRelay(uint8_t _relayUpPin, uint8_t _relayDownPin, uint8_t _fullClosingTimeS, bool _relayLowActive) 
    : ShdWindowBlindRelay(_relayUpPin, _relayDownPin, _fullClosingTimeS, _relayLowActive, 0, 0, false) {

}

ShdWindowBlindRelay::ShdWindowBlindRelay(uint8_t _relayUpPin, uint8_t _relayDownPin, uint8_t _fullClosingTimeS, bool _relayLowActive, uint8_t _buttonUpPin, uint8_t _buttonDownPin, bool _buttonsLowActive) 
    : relayUpPin(_relayUpPin), relayDownPin(_relayDownPin), fullClosingTicks(_fullClosingTimeS*200), relayLowActive(_relayLowActive), buttonUpPin(_buttonUpPin), buttonDownPin(_buttonDownPin), buttonsLowActive(_buttonsLowActive) {

    currentMovementStatus = NOT_MOVING;
    targetMovementStatus = NOT_MOVING;
    overRunTicks = 2 * 200;
    overRunTickCounter = 0;
    minTicksForMovementchange = 100;

    currentPositionTicks = 0;

    buttonUpPressed = false;
    buttonDownPressed = false;
    interruptOccured = false;

    snprintf(subTopicTargetPosition, 50, "%s/WindowBlindRelay/%d/setTarget", name, windowBlindRelayCount);
    snprintf(subTopicHoldPosition, 50, "%s/WindowBlindRelay/%d/setHold", name, windowBlindRelayCount);
    snprintf(pubTopicTargetPosition, 50, "%s/WindowBlindRelay/%d/getTarget", name, windowBlindRelayCount);
    snprintf(pubTopicCurrentPosition, 50, "%s/WindowBlindRelay/%d/getCurrentPosition", name, windowBlindRelayCount);
    snprintf(pubTopicMovingStatus, 50, "%s/WindowBlindRelay/%d/getMovingStatus", name, windowBlindRelayCount);

    mqttSubscribe(this, subTopicTargetPosition);
    mqttSubscribe(this, subTopicHoldPosition);

    if(buttonUpPin != buttonDownPin) {
        pinMode(buttonUpPin, INPUT_PULLUP);
        attachInterrupt(buttonUpPin, std::bind(&ShdWindowBlindRelay::buttonUpInterrupt, this), CHANGE);
        pinMode(buttonDownPin, INPUT_PULLUP);
        attachInterrupt(buttonDownPin, std::bind(&ShdWindowBlindRelay::buttonDownInterrupt, this), CHANGE);
    }
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
    for (uint8_t i = 0; i < NUMBER_OF_LOGIC_CKECKS; i++) {
        summe += digitalRead(buttonUpPin);
    }

    if (!buttonsLowActive) {
        if(summe > (NUMBER_OF_LOGIC_CKECKS/2)) {
            buttonUpPressed = true;
        }
    } else {
        if(summe < (NUMBER_OF_LOGIC_CKECKS/2)) {
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
    for (uint8_t i = 0; i < NUMBER_OF_LOGIC_CKECKS; i++) {
        summe += digitalRead(buttonDownPin);
    }

    // Save status
    if (!buttonsLowActive) {
        if(summe > (NUMBER_OF_LOGIC_CKECKS/2)) {
            buttonDownPressed = true;
        }
    } else {
        if(summe < (NUMBER_OF_LOGIC_CKECKS/2)) {
            buttonDownPressed = true;
        }
    }
}

void ShdWindowBlindRelay::timer5msHandler() {
    processButtonStates();
    updateCurrentPositionTicks();
    updateTargetMovementStatus();
    updateCurrentMovementStatus();
    updateOutputs();
}

void ShdWindowBlindRelay::processButtonStates() {
    if (!interruptOccured){
        return;
    }
    interruptOccured = false;

    if (buttonUpPressed && !buttonDownPressed) {
        setTargetPercentage(100);
    } else if (!buttonUpPressed && buttonDownPressed) {
        setTargetPercentage(0);
    } 
}

void ShdWindowBlindRelay::setTargetPercentage(uint8_t _newTargetPosition) {
    targetPositionTicks = (_newTargetPosition * fullClosingTicks) / 100.0;
}

void ShdWindowBlindRelay::updateTargetMovementStatus() {
    if (currentPositionTicks < targetPositionTicks) {
        targetMovementStatus = MOVING_UP; 
    } else if(currentPositionTicks > targetPositionTicks) {
        targetMovementStatus = MOVING_DOWN; 
    } else {

        // Keep targetMovementStatus at both ends (fully opened or fully closed)
        // for overRunTicks:
        if (overRunTickCounter < overRunTicks && (targetPositionTicks == fullClosingTicks || fullClosingTicks == 0)) {
            overRunTickCounter++;
            return;
        }
        
        overRunTickCounter = 0;
        targetMovementStatus = NOT_MOVING;
    }
    
}

void ShdWindowBlindRelay::updateOutputs() {
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
        digitalWrite(_relayPin, _status);
    } else {
        digitalWrite(_relayPin, !_status);
    }
}

void ShdWindowBlindRelay::updateCurrentMovementStatus() {
    ticksSinceMovementChanged++;
    
    if((currentMovementStatus != targetMovementStatus) && (ticksSinceMovementChanged > minTicksForMovementchange)) {
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
            if(currentPositionTicks > 0) {
                currentPositionTicks--;
            }
            break;
        case MOVING_DOWN:
            if(currentPositionTicks < fullClosingTicks) {
                currentPositionTicks++;
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
