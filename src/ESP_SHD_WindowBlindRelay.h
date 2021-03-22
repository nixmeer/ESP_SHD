#ifndef ESP_SHD_WINDOW_BLIND_RELAY_H
#define ESP_SHD_WINDOW_BLIND_RELAY_H

#include "ESP_SmartHomeDevice.h"

#define TOPIC_LENGTH 60
#define MQTT_PAYLOAD_BUFFER_SIZE 10
#define DEBOUNCE_MS 500
#define NUMBER_OF_LOGIC_CKECKS 10
#define DEBUG 0

enum movementStatus { MOVING_UP, MOVING_DOWN, NOT_MOVING };

class ShdWindowBlindRelay : public ESP_SmartHomeDevice {
public:
    ShdWindowBlindRelay(uint8_t _relayUpPin, uint8_t _relayDownPin, uint8_t _fullClosingTimeS, bool _relayLowActive);
    ShdWindowBlindRelay(uint8_t _relayUpPin, uint8_t _relayDownPin, uint8_t _fullClosingTimeS, bool _relayLowActive, uint8_t _buttonUpPin, uint8_t _buttonDownPin, bool _buttonsLowActive );
private:
    bool handleMqttRequest(char* _topic, byte* _payload, uint16_t _length);
    void republish();
    void timer5msHandler();
    static uint8_t windowBlindRelayCount;

    const uint8_t relayUpPin;
    const uint8_t relayDownPin;
    const uint8_t relayLowActive;
    const uint8_t buttonUpPin;
    const uint8_t buttonDownPin;
    const uint8_t buttonsLowActive;

    bool buttonUpPressed;
    bool buttonDownPressed;  
    bool interruptOccured; 
    uint32_t lastUpInterrupt;
    uint32_t lastDownInterrupt;
    void buttonUpInterrupt();
    void buttonDownInterrupt();

    uint32_t overRunTicks;
    uint32_t overRunTickCounter;

    const uint32_t fullClosingTicks;
    uint32_t currentPositionTicks;
    uint32_t targetPositionTicks;
    uint16_t ticksSinceMovementChanged;
    uint16_t minTicksForMovementchange;
    void updateCurrentPositionTicks();
    void setTargetPercentage(uint8_t _newTargetPosition);

    void movementControl();
    movementStatus currentMovementStatus;
    movementStatus targetMovementStatus;
    void updateOutputs();
    void setRelay(uint8_t _relayPin, bool _status);

    char subTopicTargetPosition[TOPIC_LENGTH];
    char pubTopicTargetPosition[TOPIC_LENGTH];
    char pubTopicCurrentPosition[TOPIC_LENGTH];
    char subTopicHoldPosition[TOPIC_LENGTH];
    char pubTopicMovingStatus[TOPIC_LENGTH];
    char payloadBuffer[MQTT_PAYLOAD_BUFFER_SIZE];

    void clearPayloadBuffer();
    void updateCurrentMovementStatus();
    void updateTargetMovementStatus();
    void processButtonStates();
};

#endif