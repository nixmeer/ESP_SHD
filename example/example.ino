#include "PubSubClient.h"
#include "WiFiManager.h"
#include <ArduinoOTA.h>
#include "ESP_SmartHomeDevice.h"
#include "ESP_SHD_MotionSensor.h"
#include "ESP_SHD_TemperatureSensor.h"

#define MODUL_NAME "Flur/Spiegel"

void setup() {
  Serial.begin(115200);
  Serial.println("Serial started.");

  WiFi.mode(WIFI_STA);
  WiFiManager wifiManager;
  wifiManager.autoConnect();
  WiFi.hostname(MODUL_NAME);

  setupArduinoOta();

  ESP_SmartHomeDevice::init(MODUL_NAME);

  new ShdMotionSensor(10);
  new ShdTemperatureSensor();

}

void loop() {

  ArduinoOTA.handle();

  // remove, if MQTT reconnect works with timer
  ESP_SmartHomeDevice::loop();
}

void setupArduinoOta(){
  // set hostname visible in Arduino IDE
  ArduinoOTA.setHostname(MODUL_NAME);

  // set Password in hash value
  //ArduinoOTA.setPassword("Dihiw2007");

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
