#include "ESP_SmartHomeDevice.h"
#include "pwm.h"
#define DEBUG 5

ESP_SmartHomeDevice* ESP_SmartHomeDevice::shds[MAX_SHDS];
PubSubClient ESP_SmartHomeDevice::mqttClient;
WiFiClient ESP_SmartHomeDevice::wifiClient;
uint8_t ESP_SmartHomeDevice::numberOfShds = 0;
uint32_t ESP_SmartHomeDevice::lastConnectionAttempt = 0;
uint32_t ESP_SmartHomeDevice::last5msTimer = 0;
uint32_t ESP_SmartHomeDevice::last1msTimer = 0;
uint32_t ESP_SmartHomeDevice::lastMqttLoop = 0;
char* ESP_SmartHomeDevice::name;
bool ESP_SmartHomeDevice::useMdns = false;
char* ESP_SmartHomeDevice::mqttServerAddress;
uint16_t ESP_SmartHomeDevice::port;
mqttSubscription* ESP_SmartHomeDevice::lastSubscription = NULL;
bool ESP_SmartHomeDevice::firstRun = true;
uint32_t ESP_SmartHomeDevice::pwmDutyInit[MAX_PWM_CHANNELS];
uint32_t ESP_SmartHomeDevice::ioInfo[MAX_PWM_CHANNELS][3];
uint16_t ESP_SmartHomeDevice::gammaCorrection[1001] = {
  0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5,
  5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 11, 11,
  11, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 15, 16, 16, 16,
  17, 17, 17, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 22, 22,
  22, 23, 23, 24, 24, 25, 25, 25, 26, 26, 27, 27, 28, 28, 28,
  29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36,
  36, 37, 37, 38, 38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
  44, 45, 45, 46, 46, 47, 47, 48, 49, 49, 50, 50, 51, 51, 52,
  53, 53, 54, 55, 55, 56, 56, 57, 58, 58, 59, 60, 60, 61, 62,
  62, 63, 64, 64, 65, 66, 67, 67, 68, 69, 69, 70, 71, 72, 72,
  73, 74, 75, 75, 76, 77, 78, 78, 79, 80, 81, 82, 82, 83, 84,
  85, 86, 87, 87, 88, 89, 90, 91, 92, 93, 93, 94, 95, 96, 97,
  98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 108,
  109, 110, 111, 113, 114, 115, 116, 117, 118, 119, 120, 121,
  122, 123, 124, 125, 126, 127, 129, 130, 131, 132, 133, 134,
  135, 137, 138, 139, 140, 141, 143, 144, 145, 146, 148, 149,
  150, 151, 153, 154, 155, 157, 158, 159, 161, 162, 163, 165,
  166, 167, 169, 170, 172, 173, 174, 176, 177, 179, 180, 182,
  183, 185, 186, 188, 189, 191, 192, 194, 196, 197, 199, 200,
  202, 204, 205, 207, 209, 210, 212, 214, 215, 217, 219, 220,
  222, 224, 226, 228, 229, 231, 233, 235, 237, 239, 240, 242,
  244, 246, 248, 250, 252, 254, 256, 258, 260, 262, 264, 266,
  268, 270, 272, 274, 276, 279, 281, 283, 285, 287, 289, 292,
  294, 296, 298, 301, 303, 305, 308, 310, 312, 315, 317, 320,
  322, 324, 327, 329, 332, 334, 337, 339, 342, 345, 347, 350,
  352, 355, 358, 360, 363, 366, 369, 371, 374, 377, 380, 383,
  385, 388, 391, 394, 397, 400, 403, 406, 409, 412, 415, 418,
  421, 424, 427, 431, 434, 437, 440, 443, 447, 450, 453, 457,
  460, 463, 467, 470, 474, 477, 481, 484, 488, 491, 495, 498,
  502, 506, 509, 513, 517, 521, 524, 528, 532, 536, 540, 544,
  548, 552, 556, 560, 564, 568, 572, 576, 580, 585, 589, 593,
  597, 602, 606, 610, 615, 619, 624, 628, 633, 637, 642, 646,
  651, 656, 660, 665, 670, 675, 680, 684, 689, 694, 699, 704,
  709, 714, 719, 725, 730, 735, 740, 746, 751, 756, 762, 767,
  772, 778, 784, 789, 795, 800, 806, 812, 818, 823, 829, 835,
  841, 847, 853, 859, 865, 871, 877, 884, 890, 896, 903, 909,
  915, 922, 928, 935, 942, 948, 955, 962, 968, 975, 982, 989,
  996, 1003, 1010, 1017, 1024, 1032, 1039, 1046, 1053, 1061,
  1068, 1076, 1083, 1091, 1099, 1106, 1114, 1122, 1130, 1138,
  1146, 1154, 1162, 1170, 1178, 1186, 1195, 1203, 1212, 1220,
  1229, 1237, 1246, 1255, 1263, 1272, 1281, 1290, 1299, 1308,
  1317, 1326, 1336, 1345, 1354, 1364, 1373, 1383, 1393, 1402,
  1412, 1422, 1432, 1442, 1452, 1462, 1472, 1482, 1493, 1503,
  1514, 1524, 1535, 1545, 1556, 1567, 1578, 1589, 1600, 1611,
  1622, 1633, 1645, 1656, 1668, 1679, 1691, 1703, 1715, 1726,
  1738, 1750, 1763, 1775, 1787, 1800, 1812, 1825, 1837, 1850,
  1863, 1876, 1889, 1902, 1915, 1928, 1941, 1955, 1968, 1982,
  1996, 2010, 2023, 2037, 2052, 2066, 2080, 2094, 2109, 2123,
  2138, 2153, 2168, 2183, 2198, 2213, 2228, 2244, 2259, 2275,
  2290, 2306, 2322, 2338, 2354, 2371, 2387, 2403, 2420, 2437,
  2453, 2470, 2487, 2505, 2522, 2539, 2557, 2574, 2592, 2610,
  2628, 2646, 2664, 2682, 2701, 2720, 2738, 2757, 2776, 2795,
  2814, 2834, 2853, 2873, 2893, 2913, 2933, 2953, 2973, 2993,
  3014, 3035, 3056, 3077, 3098, 3119, 3141, 3162, 3184, 3206,
  3228, 3250, 3272, 3295, 3317, 3340, 3363, 3386, 3409, 3433,
  3456, 3480, 3504, 3528, 3552, 3577, 3601, 3626, 3651, 3676,
  3701, 3726, 3752, 3778, 3803, 3830, 3856, 3882, 3909, 3936,
  3963, 3990, 4017, 4045, 4072, 4100, 4128, 4157, 4185, 4214,
  4243, 4272, 4301, 4330, 4360, 4390, 4420, 4450, 4481, 4511,
  4542, 4573, 4605, 4636, 4668, 4700, 4732, 4764, 4797, 4830,
  4863, 4896, 4930, 4963, 4997, 5031, 5066, 5101, 5135, 5171,
  5206, 5241, 5277, 5313, 5350, 5386, 5423, 5460, 5498, 5535,
  5573, 5611, 5649, 5688, 5727, 5766, 5805, 5845, 5885, 5925,
  5966, 6006, 6047, 6089, 6130, 6172, 6214, 6257, 6300, 6343,
  6386, 6430, 6473, 6518, 6562, 6607, 6652, 6697, 6743, 6789,
  6836, 6882, 6929, 6977, 7024, 7072, 7120, 7169, 7218, 7267,
  7317, 7367, 7417, 7468, 7518, 7570, 7621, 7673, 7726, 7778,
  7832, 7885, 7939, 7993, 8047, 8102, 8158, 8213, 8269, 8326,
  8382, 8440, 8497, 8555, 8613, 8672, 8731, 8791, 8851, 8911,
  8972, 9033, 9095, 9157, 9219, 9282, 9345, 9409, 9473, 9538,
  9603, 9668, 9734, 9800, 9867, 9934, 10002, 10070, 10139,
  10208, 10277, 10347, 10418, 10489, 10560, 10632, 10705,
  10778, 10851, 10925, 10999, 11074, 11150, 11226, 11302,
  11379, 11457, 11535, 11613, 11692, 11772, 11852, 11933,
  12014, 12096, 12178, 12261, 12345, 12429, 12513, 12599,
  12684, 12771, 12858, 12945, 13033, 13122, 13212, 13301,
  13392, 13483, 13575, 13667, 13760, 13854, 13948, 14043,
  14139, 14235, 14332, 14430, 14528, 14627, 14726, 14827,
  14928, 15029, 15131, 15234, 15338, 15442, 15548, 15653,
  15760, 15867, 15975, 16084, 16193, 16303, 16414, 16526,
  16638, 16752, 16866, 16980, 17096, 17212, 17329, 17447,
  17566, 17686, 17806, 17927, 18049, 18172, 18295, 18420,
  18545, 18671, 18798, 18926, 19055, 19185, 19315, 19446,
  19579, 19712, 19846, 19981, 20117, 20254, 20391, 20530,
  20670, 20810, 20952, 21094, 21238, 21382, 21528, 21674,
  21821, 21970, 22119, 22270, 22421, 22574, 22727, 22882,
  23037, 23194, 23352, 23510, 23670, 23831, 23993, 24156,
  24321, 24486, 24652, 24820, 24989, 25159, 25330, 25502,
  25675, 25850, 26026, 26203, 26381, 26560, 26741, 26922,
  27106, 27290, 27475, 27662, 27850, 28039, 28230, 28422,
  28615, 28810, 29006, 29203, 29401, 29601, 29802, 30005,
  30209, 30414, 30621, 30829, 31039, 31250, 31462, 31676,
  31891, 32108, 32326, 32546, 32767
};
pwm* ESP_SmartHomeDevice::lastPwm = NULL;
uint8_t ESP_SmartHomeDevice::numberOfPwms = 0;

ESP_SmartHomeDevice::ESP_SmartHomeDevice(){
    if (numberOfShds < MAX_SHDS-1) {
        shds[numberOfShds] = this;
        numberOfShds++;
    } else {
        Serial.println("SHD: Too many devices!!");
    }
}

void ESP_SmartHomeDevice::init(char* _mqttServerAddress, uint16_t _port, char* _name){
    numberOfShds = 0;
    name = _name;
    useMdns = false;

    mqttServerAddress = _mqttServerAddress;
    port = _port;

    Serial.println("------------------------------------------");
    Serial.println();
}

void ESP_SmartHomeDevice::init(char *_name){
    numberOfShds = 0;
    name = _name;
    useMdns = true;

    Serial.println("------------------------------------------");
    Serial.println();
}

bool ESP_SmartHomeDevice::connectWifi(){
#if DEBUG > 1
    // Serial.print("SHD: connectWifi() was called. free cont stack: ");
    // Serial.print(ESP.getFreeContStack());
    // Serial.print(", free heap stack: ");
    // Serial.print(ESP.getFreeHeap());
    // Serial.println();
#endif
    if (WiFi.status() != WL_CONNECTED) {
        // WiFi.mode(WIFI_STA);
        WiFiManager wifiManager;
        wifiManager.setConfigPortalTimeout(90);
        wifiManager.autoConnect(name);
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.hostname(name);
            Serial.println("SHD: WiFiManager could establish a connection.");
            return true;
        } else {
            Serial.println("WiFiManager could not establish a connection. Resetting...");
            ESP.reset();
            return false;
        }
    } else {
#if DEBUG > 1
        Serial.println("SHD: WiFi connection was ok.");
#endif
        return true;
    }
}

void ESP_SmartHomeDevice::reconnectMqtt(){

    // start mDNS:
    // if (!MDNS.begin(name)) {
    //   Serial.print("MQTT: Trying to begin to mDNS service .");
    //   uint32_t mdnsMillis = millis();
    //   while (!MDNS.begin(name)) {
    //     Serial.print(".");
    //     delay(200);
    //     if (millis() - mdnsMillis > 5001) {
    //       Serial.println(" Could not begin mDNS service.");
    //       break;
    //     }
    //   }
    // }

    // find mqtt service via mdns:
    uint16_t n = MDNS.queryService("mqtt", "tcp");
    Serial.print("SHD: ");
    Serial.print(n);
    Serial.println(" mqtt services found.");

    if (n != 1) {
        return;
    }

    reconnectMqtt(MDNS.IP(0),  MDNS.port(0));
}

void ESP_SmartHomeDevice::reconnectMqtt(const char* _mqttServerAddress, uint16_t _port){
    // mqttClient.disconnect();

    mqttClient.setClient(wifiClient);
    mqttClient.setServer(_mqttServerAddress, _port);
    mqttClient.setCallback(ESP_SmartHomeDevice::mqttCallback);

    if (mqttClient.connect(name)) {
        Serial.println("MQTT: Now successfully connected to broker. ");
    } else {
        Serial.println("MQTT: 1 mqtt service found via mDNS but connecting to broker failed.");
    }

    // os_timer_setfn(&ESP_SmartHomeDevice::loopTimer, &ESP_SmartHomeDevice::loop, NULL);
    // os_timer_arm(&ESP_SmartHomeDevice::loopTimer, 5, true);
}

void ESP_SmartHomeDevice::reconnectMqtt(IPAddress _mqttServerAddress, uint16_t _port){
    // mqttClient.disconnect();

    mqttClient.setClient(wifiClient);
    mqttClient.setServer(_mqttServerAddress, _port);
    mqttClient.setCallback(ESP_SmartHomeDevice::mqttCallback);

    if (mqttClient.connect(name)) {
        Serial.println("MQTT: Now successfully connected to broker. ");
    } else {
        Serial.println("MQTT: 1 mqtt service found via mDNS but connecting to broker failed.");
    }

    // os_timer_setfn(&ESP_SmartHomeDevice::loopTimer, &ESP_SmartHomeDevice::loop, NULL);
    // os_timer_arm(&ESP_SmartHomeDevice::loopTimer, 5, true);
}

void ESP_SmartHomeDevice::mqttCallback(char* _topic, unsigned char* _payload, unsigned int _length){
#if DEBUG >= 1
    Serial.print("MQTT: callback topic: ");
    Serial.print(_topic);
    Serial.println(".");
#endif
    mqttSubscription* subscribtion = (mqttSubscription*)lastSubscription;
    while (subscribtion != NULL) {
        if (strcmp(_topic, subscribtion->topic) == 0) {
            subscribtion->subscriber->handleMqttRequest(_topic, _payload, _length);
            break;
        } else {
            subscribtion = subscribtion->next;
        }
    }
    // clear payload:
    uint16_t i = 0;
    while (_payload[i] != 0) {
        _payload[i] = 0;
        i++;
    }
}

void ESP_SmartHomeDevice::loop(){//void *pArg){
    uint32_t currentMicros = micros();


    if (firstRun) {
      firstRunFunction();
    }

    // Serial.println("TEST 1.");

    if (currentMicros - last1msTimer > 1000) {
        while (currentMicros - last1msTimer > 1000) {
            last1msTimer += 1000;
        }

        if (!mqttClient.loop()) {
#if DEBUG > 1
            Serial.print("MQTT: state = ");
            Serial.println(mqttClient.state());
            Serial.print("MQTT: loop() returned false. It has not been called since ");
            Serial.print(currentMicros - lastMqttLoop);
            Serial.println(" us.");
#endif
            reconnect();
            return;
        } else {
            lastMqttLoop = currentMicros;
        }

    }

    if(currentMicros - last5msTimer > 5000){
        while (currentMicros - last5msTimer > 5000) {
            last5msTimer += 5000;
        }

        for (size_t i = 0; i < numberOfShds; i++) {
            shds[i]->timer5msHandler();
        }
    }

}

void ESP_SmartHomeDevice::reconnect(){
    uint32_t currentMillis = millis();
    if (currentMillis - lastConnectionAttempt > TRY_RECONNECT_AFTER_MILLISECONDS) {

        // check wifi connection:
        if (connectWifi()) {

            // reconnect mqtt client:
            if (useMdns) {
                reconnectMqtt();
            } else {
                reconnectMqtt(mqttServerAddress, port);
            }

            // if reconnecting has been successfull, resubscribe to all topics and publish current states:
            if (mqttClient.connected()) {
                if (resubscribe()) {
                    Serial.println("MQTT: Successfully (re-)subscribed.");
                }
                for (size_t i = 0; i < numberOfShds; i++) {
                    shds[i]->republish();
                }
            }  else {
                Serial.print("SHD: Last connection attempt ");
                Serial.print(currentMillis - lastConnectionAttempt);
                Serial.println(" milliseconds ago.");
            }
        }

        // update lastConnectionAttempt
        while (lastConnectionAttempt < (currentMillis - TRY_RECONNECT_AFTER_MILLISECONDS)) {
            lastConnectionAttempt += TRY_RECONNECT_AFTER_MILLISECONDS;
        }
    }
}

bool ESP_SmartHomeDevice::mqttPublish(char *_topic, const char *_payload) {
    if (mqttClient.connected()) {
        mqttClient.publish(_topic, _payload);
    }
}

bool ESP_SmartHomeDevice::mqttConnected() {
    return mqttClient.connected();
}

bool ESP_SmartHomeDevice::mqttSubscribe(ESP_SmartHomeDevice *_subscriber, char *_topic) {
    mqttSubscription* tmp = new mqttSubscription;
    tmp->next = (mqttSubscription*)lastSubscription;
    tmp->subscriber = _subscriber;
    tmp->topic = _topic;
    // mqttClient.subscribe(tmp->topic, 0);
    // Serial.print("MQTT: subscribed to");
    // Serial.println(tmp->topic);

    lastSubscription = tmp;
    return true;
}

bool ESP_SmartHomeDevice::resubscribe() {
    if (!mqttClient.connected()) {
        return false;
    }

    mqttSubscription* subscribtion = (mqttSubscription*)lastSubscription;
    while(subscribtion != NULL) {
        mqttClient.subscribe(subscribtion->topic, 0);
        Serial.print("MQTT: subscribed to ");
        Serial.println(subscribtion->topic);
        subscribtion = subscribtion->next;
    }
    return true;
}

int8_t ESP_SmartHomeDevice::registerPwmPin(ESP_SmartHomeDevice* _owner, uint8_t _pin, bool _lowActive) {
  bool success = true;


  // check, if another pwm channel is available:
  if (numberOfPwms >= MAX_PWM_CHANNELS) {
    success = false;
  }

  // check, if requested pwm pin is still available:
  if (success) {
    pwm* localPwm = lastPwm;
    while (localPwm != NULL) {
      if (_pin == localPwm->pin) {
        success = false;
        break;
      }
      localPwm = localPwm->next;
    }
  }


  if (success) {
    pwm* newPwm = new pwm;
    newPwm->lowActive = _lowActive;
    newPwm->owner = _owner;
    newPwm->next = lastPwm;
    newPwm->pin = _pin;
    lastPwm = newPwm;
  }


  int8_t _pwmNumber = addIoInfo(lastPwm->pin);
  if (_pwmNumber != -1) {
    lastPwm->pwmNumber = _pwmNumber;
  } else {
    success = false;
  }

  if (success) {
    if (!lastPwm->lowActive) {
      pwmDutyInit[numberOfPwms-1] = 0;
    } else {
      pwmDutyInit[numberOfPwms-1] = gammaCorrection[1000];
    }
    pinMode(lastPwm->pin, OUTPUT);
  }

  if (success) {
    Serial.print("PWM: Registered pin ");
    Serial.print(lastPwm->pin);
    Serial.print(" as pwm number ");
    Serial.println(lastPwm->pwmNumber);
  }

  if (success) {
    return lastPwm->pwmNumber;
  } else {
    return -1;
  }
}

bool ESP_SmartHomeDevice::setPwmPermill(ESP_SmartHomeDevice *_owner, uint8_t _pwmNumber, uint16_t _value) {
  bool success = true;

  pwm* localPwm = lastPwm;
  while (localPwm->pwmNumber != _pwmNumber) {
    if (localPwm->next == NULL) {
      success = false;
      break;
    }
    localPwm = localPwm->next;
  }

  if (success) {
    if (localPwm->owner != _owner) {
      success = false;
    }
  }

  if (success) {
    if (_value > 1000 || _value < 0) {
      success = false;
    }
  }

  if (success) {
    if (!localPwm->lowActive) {
      pwm_set_duty(gammaCorrection[_value], localPwm->pwmNumber);
    } else {
      pwm_set_duty(gammaCorrection[1000] - gammaCorrection[_value], localPwm->pwmNumber);
    }
    pwm_start();
  }

  return success;
}

void ESP_SmartHomeDevice::firstRunFunction() {
  firstRun = false;
  pwm_init(gammaCorrection[1000], pwmDutyInit, numberOfPwms, ioInfo);
}

int8_t ESP_SmartHomeDevice::addIoInfo(uint8_t _pin){
    bool success = true;
    switch (_pin) {
      case 2:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_GPIO2_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO2;
        ioInfo[numberOfPwms][2] = 2;
        break;

      case 3:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_U0RXD_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO3;
        ioInfo[numberOfPwms][2] = 3;
        break;

      case 4:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_GPIO4_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO4;
        ioInfo[numberOfPwms][2] = 4;
        break;

      case 5:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_GPIO5_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO5;
        ioInfo[numberOfPwms][2] = 5;
        break;

      case 10:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_SD_DATA3_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO10;
        ioInfo[numberOfPwms][2] = 10;
        break;

      case 12:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_MTDI_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO12;
        ioInfo[numberOfPwms][2] = 12;
        break;

      case 13:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_MTCK_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO13;
        ioInfo[numberOfPwms][2] = 13;
        break;

      case 14:
        ioInfo[numberOfPwms][0] = PERIPHS_IO_MUX_MTMS_U;
        ioInfo[numberOfPwms][1] = FUNC_GPIO14;
        ioInfo[numberOfPwms][2] = 14;
        break;

      default:
        success = false;
        break;
    }

    if (success) {
      numberOfPwms++;
      return numberOfPwms-1;
    } else {
      return -1;
    }
}
