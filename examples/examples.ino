#include <ArduinoOTA.h>
#include "ESP_SHD.h"

// activate Arduino OTA:
#define OTA

// define module name (used for mqtt topics):
#define MODUL_NAME "testroom/detailedLocation" // replace it with whatever fits your personal needs

void setup() {
  Serial.begin(115200);
  Serial.println("Serial started.");

  #ifdef OTA
  setupArduinoOta();
  #endif

  ESP_SmartHomeDevice::init(MODUL_NAME);

  // Adding a PwmLight:
  new ShdPwmSingleColorLight(12, true, 25, 1000);
  // ShdPwmSingleColorLight(uint8_t _pin, bool _lowActive, uint8_t _millisUpdateInterval, uint16_t _flankLength);

  // Adding a motion sensor:
  new ShdMotionSensor(5); // standard pin: 5, old pin: 10
  // ShdMotionSensor(uint8_t _pin);

  // Adding a TMP36 sensor:
  new ShdTmp36Sensor();

  // Adding a DS18B20 sensor:
  new ShdDs18b20Sensor(6);
  // ShdDs18b20Sensor(uint8_t _pin);

  // Adding a WS2812b strip with two sections:
  ShdWs2812bStrip::initStrip(122, 25);
  new ShdWs2812bStrip(1, 100, 1, IGNITION_BOTH_FORWARD, 1, 30);
  new ShdWs2812bStrip(101, 122, 101, IGNITION_SINGLE_FORWARD, 1, 25);
  // ShdWs2812bStrip(uint16_t _firstLed, uint16_t _lastLed, uint16_t ignitionPoint, ignitionDirection _ignitionDirection, uint8_t _hopsPerShow, uint8_t _flankLength);

  // Adding a button:
  new ShdButton(5, true, 20, 500, 1000);
  // ShdButton(uint8_t _pin, bool _lowActive, uint32_t _millisDebounce, uint32_t _millisLongClick, uint32_t _millisMultiClick)

  // Adding a sprinkler:
  new ShdSprinkler(12, false);
  // ShdSprinkler(uint8_t _pin, bool _lowActive);

  // Adding a relay:
  new ShdRelay(12, 100, false, true);
  // ShdRelay(uint8_t _pin, uint32_t _millisBetweenToggle, bool _lowActive, bool _valueAtBeginning);

  // Adding a DHT-Sensor:
  new ShdDht22Sensor(13, 30, 22);
  // ShdDht22Sensor(uint8_t _pin, uint32_t _intervalS, uint8_t _dhtType);

  // Adding a PWM light with temperature:
  new ShdPwmTemperatureLight(MODE_WARM_PWM_COLD_PWM, 2, 3, false, false, 20, 500);

  // Adding a window cover with two relays:
  new ShdWindowBlindRelay(4, 5, 35, false, 2, 3, false); 

}

void loop() {

  #ifdef OTA
  ArduinoOTA.handle();
  #endif

  // remove, if MQTT reconnect works with timer
  ESP_SmartHomeDevice::loop();
}

void setupArduinoOta(){
  // set hostname visible in Arduino IDE
  ArduinoOTA.setHostname(MODUL_NAME);

  // set Password in hash value
  //ArduinoOTA.setPassword("Test1234");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  #ifdef DEBUG
  Serial.println("OTA ready");
  Serial.println();
  #endif

}
