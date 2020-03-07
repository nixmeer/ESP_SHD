#include "ESP_SmartHomeDevice.h"
#include <FunctionalInterrupt.h>


class ShdRelay : public ESP_SmartHomeDevice {
public:
    ShdRelay(uint8_t _pin, uint32_t _millisBetweenToggle, bool _lowActive, bool _valueAtBeginning);
private:
    static uint8_t relayCount;
    uint8_t relayNumber;

    void timer5msHandler();
    bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
    void republish();

    void setOuput(bool _setPoint);

    bool setPoint;
    bool currentStatus;
    uint16_t cycleCounterSinceLastToggle;
    uint16_t minCyclesBetweenToggles;

    const uint8_t pin;
    const bool lowActive;

    char subTopicSetStatus[50];
    char pubTopicGetStatus[50];
};
