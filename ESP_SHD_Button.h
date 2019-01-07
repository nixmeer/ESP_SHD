#include "ESP_SmartHomeDevice.h"
#include <FunctionalInterrupt.h>


class ShdButton : public ESP_SmartHomeDevice {
public:
  ShdButton(uint8_t _pin, bool _lowActive, uint32_t _millisDebounce, uint32_t _millisLongClick, uint32_t _millisMultiClick);
private:
  static uint8_t numberOfButtons;

  void timer5msHandler();
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  void resubscribe();

  void handleInterrupt();

  bool lowActive;
  uint8_t pin;
  char pubTopic[50];

  uint32_t millisDebounce;
  uint32_t millisLongClick;
  uint32_t millisMultiClick;

  bool currentlyClicked;
  uint32_t lastInterruptTime;
  uint32_t firstClickTime;
  uint8_t clickCounter;
};
