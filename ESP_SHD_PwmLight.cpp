#include "ESP_SHD_PwmLight.h"
#include "pwm.h"

#define DEBUG 10

bool Shd_PwmLight::firstRun = true;
uint8_t Shd_PwmLight::numberOfPwmPins = 0;
uint32_t Shd_PwmLight::pwmDutyInit[MAX_PWM_CHANNELS];
uint32_t Shd_PwmLight::ioInfo[MAX_PWM_CHANNELS][3];
uint32_t Shd_PwmLight::gammaCorrection[101] = {
  0, 1, 4, 10, 20, 33, 51, 72,
  98, 129, 164, 204, 250, 300, 356, 417,
  484, 557, 635, 719, 809, 905, 1007, 1115,
  1230, 1351, 1479, 1613, 1753, 1901, 2055, 2216,
  2384, 2559, 2741, 2929, 3126, 3329, 3539, 3757,
  3983, 4215, 4456, 4703, 4959, 5222, 5493, 5771,
  6057, 6352, 6654, 6964, 7282, 7608, 7942, 8285,
  8635, 8994, 9361, 9736, 10120, 10512, 10913, 11322,
  11740, 12166, 12600, 13044, 13496, 13957, 14427, 14905,
  15392, 15888, 16393, 16907, 17430, 17962, 18504, 19054,
  19613, 20181, 20759, 21346, 21942, 22548, 23162, 23787,
  24420, 25063, 25715, 26377, 27049, 27730, 28420, 29121,
  29830, 30550, 31279, 32018, 32767
};

Shd_PwmLight::Shd_PwmLight(uint8_t _pin, bool _lowActive, uint8_t _millisUpdateInterval, uint16_t _flankLength){
  pwmNumber = numberOfPwmPins;
  numberOfPwmPins++;
  pin = _pin;
  lowActive = _lowActive;
  millisUpdateInterval = _millisUpdateInterval;
  delta = 100/(_flankLength / millisUpdateInterval);
  flankOver = true;
  setPoint = 0;
  millisLastUpdate = millis();
  lastBrightnessGreaterZero = 100;
  currentBrightness = setPoint;

  snprintf (pubTopicBrightness, 60, "%s/PwmLight/%d/getBrightness", name, numberOfPwmPins);
  snprintf (pubTopicState, 60, "%s/PwmLight/%d/getStatus", name, numberOfPwmPins);
  snprintf (subTopicBrightness, 60, "%s/PwmLight/%d/setBrightness", name, numberOfPwmPins);
  snprintf (subTopicState, 60, "%s/PwmLight/%d/setStatus", name, numberOfPwmPins);

  resubscribe();

  pwmDutyInit[pwmNumber] = 0;
  pinMode(pin, OUTPUT);
  addIoInfo();

  // debug output:
  Serial.print("New pwm light ");
  Serial.print(pwmNumber);
  Serial.print(" registered. It subscribed to ");
  Serial.print(subTopicBrightness);
  Serial.print(" and ");
  Serial.print(subTopicState);
  Serial.print(". Free stack: ");
  Serial.print(ESP.getFreeHeap());
  Serial.print(" bytes");
  Serial.println();
}

void Shd_PwmLight::timer5msHandler() {

  // return, if this light is not changing its brightness
  if (flankOver) {
    return;
  }

  // initialize ESP8266_new_pwm, if this is the first call of timer5msHandler
  if (firstRun) {
    firstRun = false;
    pwm_init(gammaCorrection[100], pwmDutyInit, numberOfPwmPins, ioInfo);

    #if DEBUG > 0
    Serial.print("SHD: PwmLight: Initialized PWM. pwmPeriod: ");
    Serial.print(gammaCorrection[100]);
    Serial.print(", pwmDutyInit[");
    Serial.print(pwmNumber);
    Serial.print("]: ");
    Serial.print(pwmDutyInit[pwmNumber]);
    Serial.print(", numberOfPwmPins: ");
    Serial.println(numberOfPwmPins);
    #endif
  }

  uint32_t currentMillis = millis();
  if(currentMillis - millisLastUpdate >= millisUpdateInterval){

    #if DEBUG > 3
    Serial.print("SHD: PwmLight: Updating no. ");
    Serial.print(pwmNumber);
    Serial.print(" at ");
    Serial.print(currentMillis);
    #endif

    // Increase millisLastUpdate to keep interval:
    millisLastUpdate += millisUpdateInterval;
    if(currentMillis - millisLastUpdate >= millisUpdateInterval) {
      millisLastUpdate = millis();
    }

    if (abs((int8_t)((int8_t)setPoint - (int8_t)currentBrightness)) <= delta) {
      currentBrightness = setPoint;
      flankOver = true;
    } else if (((int8_t)(setPoint - currentBrightness)) > 0) {
      currentBrightness += delta;
      if (currentBrightness > 100) {
        currentBrightness = 100;
        flankOver = true;
      }
    } else if(((int8_t)(setPoint - currentBrightness)) < 0) {
      currentBrightness -= delta;
      if (currentBrightness < 0) {
        currentBrightness = 0;
        flankOver = true;
      }
    }

    #if DEBUG > 3
    Serial.print(". Percentage: ");
    Serial.print(currentBrightness);
    #endif
    if (lowActive) {
      #if DEBUG > 3
      Serial.print(". Value: ");
      Serial.println(gammaCorrection[100] - gammaCorrection[currentBrightness]);
      #endif
      pwm_set_duty(gammaCorrection[100] - gammaCorrection[currentBrightness], pwmNumber);
      pwm_start();
    } else {
        #if DEBUG > 3
      Serial.print(". Value: ");
      Serial.println(gammaCorrection[currentBrightness]);
      #endif
      pwm_set_duty(gammaCorrection[currentBrightness], pwmNumber);
      pwm_start();
    }
  }
}

bool Shd_PwmLight::handleMqttRequest(char *_topic, unsigned char *_payload, uint16_t _length){
  if (strcmp(_topic, subTopicState) == 0) {
    if (_payload[0] == 0x30) {
      setBrightness(0);
    } else if (_payload[0] == 0x31) {
      setBrightness(lastBrightnessGreaterZero);
    }
  } else if (strcmp(_topic, subTopicBrightness) == 0) {
    setBrightness(atoi((char*)_payload));
  } else {
    return false;
  }
}


void Shd_PwmLight::setBrightness(uint8_t _percentage){
  if (_percentage < 0 || _percentage > 100) {
    return;
  }
  if (_percentage > 0) {
    // save every percentage greater 0 for restoring after turning the light off and on
    lastBrightnessGreaterZero = _percentage;
    mqttClient.publish(pubTopicState, "1");
  } else {
    mqttClient.publish(pubTopicState, "0");
  }

  // save new set point
  setPoint = _percentage;

  // publish new brightness
  char payload[5];
  snprintf (payload, 5, "%d", setPoint);
  mqttClient.publish(pubTopicBrightness, payload);

  // start fading process:
  flankOver = false;
}

void Shd_PwmLight::resubscribe(){
  mqttClient.subscribe(subTopicBrightness, 0);
  Serial.print("SHD: PwmLight subscribed to ");
  Serial.println(subTopicBrightness);

  mqttClient.subscribe(subTopicState, 0);
  Serial.print("SHD: PwmLight subscribed to ");
  Serial.println(subTopicState);
}

bool Shd_PwmLight::addIoInfo(){
    switch (pin) {
      case 2:
        ioInfo[pwmNumber][0] = PERIPHS_IO_MUX_GPIO2_U;
        ioInfo[pwmNumber][1] = FUNC_GPIO2;
        ioInfo[pwmNumber][2] = 2;
        break;

      case 3:
        ioInfo[pwmNumber][0] = PERIPHS_IO_MUX_U0RXD_U;
        ioInfo[pwmNumber][1] = FUNC_GPIO3;
        ioInfo[pwmNumber][2] = 3;
        break;

      case 4:
        ioInfo[pwmNumber][0] = PERIPHS_IO_MUX_GPIO4_U;
        ioInfo[pwmNumber][1] = FUNC_GPIO4;
        ioInfo[pwmNumber][2] = 4;
        break;

      case 5:
        ioInfo[pwmNumber][0] = PERIPHS_IO_MUX_GPIO5_U;
        ioInfo[pwmNumber][1] = FUNC_GPIO5;
        ioInfo[pwmNumber][2] = 5;
        break;

      case 10:
        ioInfo[pwmNumber][0] = PERIPHS_IO_MUX_SD_DATA3_U;
        ioInfo[pwmNumber][1] = FUNC_GPIO10;
        ioInfo[pwmNumber][2] = 10;
        break;

      case 12:
        ioInfo[pwmNumber][0] = PERIPHS_IO_MUX_MTDI_U;
        ioInfo[pwmNumber][1] = FUNC_GPIO12;
        ioInfo[pwmNumber][2] = 12;
        break;

      case 13:
        ioInfo[pwmNumber][0] = PERIPHS_IO_MUX_MTCK_U;
        ioInfo[pwmNumber][1] = FUNC_GPIO13;
        ioInfo[pwmNumber][2] = 13;
        break;

      case 14:
        ioInfo[pwmNumber][0] = PERIPHS_IO_MUX_MTMS_U;
        ioInfo[pwmNumber][1] = FUNC_GPIO14;
        ioInfo[pwmNumber][2] = 14;
        break;

      default:
        break;
    }
}
