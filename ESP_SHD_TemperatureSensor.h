#ifndef ESP_SHD_TEMPERATURE_SENSOR
#define ESP_SHD_TEMPERATURE_SENSOR

#define PUBLISH_TIME_IN_MS 500

extern "C" {
  #include "user_interface.h"
}
#include "PubSubClient.h"
#include "ESP_SmartHomeDevice.h"

class ShdTemperatureSensor : public ESP_SmartHomeDevice {
public:
  ShdTemperatureSensor();
private:
  bool handleMqttRequest(char* _topic, byte* _payload, unsigned int _length);
  char pubTopic[50];
  char message[10];
  os_timer_t timer;
  void publishTemperature();
  void timer5msHandler();
  int timerCounter;
};

#endif
