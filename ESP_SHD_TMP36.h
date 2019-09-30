#ifndef ESP_SHD_TMP36_SENSOR
#define ESP_SHD_TMP36_SENSOR

#define PUBLISH_TIME_IN_MS 500

extern "C" {
  #include "user_interface.h"
}
#include "PubSubClient.h"
#include "ESP_SmartHomeDevice.h"

class ShdTmp36Sensor : public ESP_SmartHomeDevice {
public:
  ShdTmp36Sensor();
private:
  bool handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length);
  char pubTopic[50];
  char message[10];
  os_timer_t timer;
  void publishTemperature();
  void timer5msHandler();
  int timerCounter;
  void republish();
};

#endif
