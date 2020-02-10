#include "ESP_SHD_PwmLight.h"
#include "pwm.h"


uint16_t ShdPwmLight::gammaCorrection[1001] = {
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 11, 11, 11, 11, 11,
    11, 11, 11, 12, 12, 12, 12, 12, 12, 12,
    13, 13, 13, 13, 13, 13, 13, 14, 14, 14,
    14, 14, 14, 14, 15, 15, 15, 15, 15, 15,
    16, 16, 16, 16, 16, 16, 17, 17, 17, 17,
    17, 18, 18, 18, 18, 18, 18, 19, 19, 19,
    19, 19, 20, 20, 20, 20, 21, 21, 21, 21,
    21, 22, 22, 22, 22, 23, 23, 23, 23, 24,
    24, 24, 24, 25, 25, 25, 25, 26, 26, 26,
    26, 27, 27, 27, 27, 28, 28, 28, 29, 29,
    29, 29, 30, 30, 30, 31, 31, 31, 32, 32,
    32, 33, 33, 33, 34, 34, 34, 35, 35, 35,
    36, 36, 36, 37, 37, 38, 38, 38, 39, 39,
    39, 40, 40, 41, 41, 42, 42, 42, 43, 43,
    44, 44, 45, 45, 45, 46, 46, 47, 47, 48,
    48, 49, 49, 50, 50, 51, 51, 52, 52, 53,
    53, 54, 54, 55, 55, 56, 56, 57, 58, 58,
    59, 59, 60, 61, 61, 62, 62, 63, 64, 64,
    65, 65, 66, 67, 67, 68, 69, 69, 70, 71,
    72, 72, 73, 74, 74, 75, 76, 77, 77, 78,
    79, 80, 80, 81, 82, 83, 84, 84, 85, 86,
    87, 88, 89, 90, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105,
    106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
    116, 118, 119, 120, 121, 122, 123, 125, 126, 127,
    128, 129, 131, 132, 133, 135, 136, 137, 139, 140,
    141, 143, 144, 145, 147, 148, 150, 151, 153, 154,
    156, 157, 159, 160, 162, 163, 165, 166, 168, 170,
    171, 173, 175, 176, 178, 180, 182, 183, 185, 187,
    189, 190, 192, 194, 196, 198, 200, 202, 204, 206,
    208, 210, 212, 214, 216, 218, 220, 222, 224, 227,
    229, 231, 233, 235, 238, 240, 242, 245, 247, 249,
    252, 254, 257, 259, 262, 264, 267, 269, 272, 274,
    277, 280, 283, 285, 288, 291, 294, 296, 299, 302,
    305, 308, 311, 314, 317, 320, 323, 326, 329, 333,
    336, 339, 342, 346, 349, 352, 356, 359, 363, 366,
    370, 373, 377, 380, 384, 388, 391, 395, 399, 403,
    407, 411, 415, 419, 423, 427, 431, 435, 439, 443,
    448, 452, 456, 461, 465, 470, 474, 479, 483, 488,
    493, 497, 502, 507, 512, 517, 522, 527, 532, 537,
    542, 547, 552, 558, 563, 569, 574, 580, 585, 591,
    596, 602, 608, 614, 620, 626, 632, 638, 644, 650,
    656, 663, 669, 675, 682, 688, 695, 702, 708, 715,
    722, 729, 736, 743, 750, 757, 765, 772, 779, 787,
    794, 802, 810, 818, 825, 833, 841, 849, 858, 866,
    874, 882, 891, 899, 908, 917, 926, 935, 943, 953,
    962, 971, 980, 990, 999, 1009, 1018, 1028, 1038, 1048,
    1058, 1068, 1078, 1089, 1099, 1110, 1120, 1131, 1142, 1153,
    1164, 1175, 1186, 1198, 1209, 1221, 1233, 1245, 1256, 1268,
    1281, 1293, 1305, 1318, 1331, 1343, 1356, 1369, 1382, 1396,
    1409, 1422, 1436, 1450, 1464, 1478, 1492, 1506, 1521, 1535,
    1550, 1565, 1580, 1595, 1610, 1626, 1641, 1657, 1673, 1689,
    1705, 1722, 1738, 1755, 1772, 1789, 1806, 1823, 1840, 1858,
    1876, 1894, 1912, 1930, 1949, 1968, 1986, 2006, 2025, 2044,
    2064, 2084, 2103, 2124, 2144, 2165, 2185, 2206, 2227, 2249,
    2270, 2292, 2314, 2336, 2359, 2381, 2404, 2427, 2450, 2474,
    2498, 2521, 2546, 2570, 2595, 2620, 2645, 2670, 2696, 2721,
    2747, 2774, 2800, 2827, 2854, 2882, 2909, 2937, 2965, 2994,
    3022, 3051, 3081, 3110, 3140, 3170, 3200, 3231, 3262, 3293,
    3325, 3357, 3389, 3421, 3454, 3487, 3521, 3554, 3588, 3623,
    3658, 3693, 3728, 3764, 3800, 3836, 3873, 3910, 3948, 3985,
    4024, 4062, 4101, 4140, 4180, 4220, 4260, 4301, 4342, 4384,
    4426, 4468, 4511, 4555, 4598, 4642, 4687, 4732, 4777, 4823,
    4869, 4916, 4963, 5010, 5058, 5107, 5156, 5205, 5255, 5305,
    5356, 5407, 5459, 5511, 5564, 5618, 5671, 5726, 5781, 5836,
    5892, 5948, 6005, 6063, 6121, 6179, 6239, 6298, 6359, 6420,
    6481, 6543, 6606, 6669, 6733, 6798, 6863, 6928, 6995, 7062,
    7129, 7198, 7267, 7336, 7407, 7478, 7549, 7622, 7695, 7768,
    7843, 7918, 7994, 8070, 8147, 8226, 8304, 8384, 8464, 8545,
    8627, 8710, 8793, 8877, 8962, 9048, 9135, 9222, 9311, 9400,
    9490, 9581, 9673, 9765, 9859, 9953, 10049, 10145, 10242, 10340,
    10439, 10539, 10640, 10742, 10845, 10949, 11054, 11160, 11266, 11374,
    11483, 11593, 11704, 11816, 11930, 12044, 12159, 12276, 12393, 12512,
    12632, 12753, 12875, 12998, 13123, 13249, 13375, 13504, 13633, 13763,
    13895, 14028, 14163, 14298, 14435, 14574, 14713, 14854, 14996, 15140,
    15285, 15431, 15579, 15728, 15879, 16031, 16185, 16340, 16496, 16654,
    16814, 16975, 17137, 17301, 17467, 17634, 17803, 17974, 18146, 18320,
    18495, 18672, 18851, 19032, 19214, 19398, 19584, 19771, 19961, 20152,
    20345, 20540, 20737, 20935, 21136, 21338, 21542, 21749, 21957, 22167,
    22380, 22594, 22810, 23029, 23249, 23472, 23697, 23924, 24153, 24384,
    24618, 24854, 25092, 25332, 25575, 25820, 26067, 26316, 26568, 26823,
    27080, 27339, 27601, 27865, 28132, 28402, 28674, 28948, 29226, 29505,
    29788, 30073, 30361, 30652, 30946, 31242, 31541, 31843, 32148, 32456, 32767
};

ShdPwmLight::ShdPwmLight(uint8_t _pin, bool _lowActive, uint8_t _millisUpdateInterval, uint16_t _flankLength){
    pwmNumber = numberOfPwmPins;
    numberOfPwmPins++;
    pin = _pin;
    lowActive = _lowActive;
    millisUpdateInterval = _millisUpdateInterval;
    flankOver = true;
    setPoint = 0;
    millisLastUpdate = millis();
    lastBrightnessGreaterZero = 100;
    currentBrightness = setPoint;
    
    snprintf (pubTopicBrightness, 60, "%s/PwmLight/%d/getBrightness", name, numberOfPwmPins);
    snprintf (pubTopicState, 60, "%s/PwmLight/%d/getStatus", name, numberOfPwmPins);
    snprintf (subTopicBrightness, 60, "%s/PwmLight/%d/setBrightness", name, numberOfPwmPins);
    snprintf (subTopicState, 60, "%s/PwmLight/%d/setStatus", name, numberOfPwmPins);
    
    registerPwm(pin);
    
    subscribe();
    republish();
    
    // debug output:
    Serial.print("PWM: New pwm light ");
    Serial.print(numberOfPwmPins);
    Serial.print(" registered. It subscribed to ");
    Serial.print(subTopicBrightness);
    Serial.print(" and ");
    Serial.print(subTopicState);
    Serial.println();
}

void ShdPwmLight::timer5msHandler() {
    
    // check, if fade is active:
    if (currentBrightness == setPoint) {
        return;
    }
    
    // check, if currentBrightness is close to setPoint
    if (abs(setPoint - currentBrightness) <= delta) {
        currentBrightness = setPoint;
    }
    
    
    if (currentBrightness < setPoint) {
        currentBrightness += delta;
    } else {
        currentBrightness -= delta;
    }
    
    // set PWM to currentBrightness:
    if (lowActive) {
        pwm_set_duty(gammaCorrection[1000] - gammaCorrection[currentBrightness], pwmNumber);
        pwm_start();
    } else {
        pwm_set_duty(gammaCorrection[currentBrightness], pwmNumber);
        pwm_start();
    }
}

void ShdPwmLight::timer5msHandler() {
    
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
    
    // check, if it time to update the pwm:
    uint32_t currentMillis = millis();
    if(currentMillis - millisLastUpdate >= millisUpdateInterval){
        return;
    }
    
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

bool ShdPwmLight::handleMqttRequest(char *_topic, unsigned char *_payload, uint16_t _length){
    if (strcmp(_topic, subTopicState) == 0) {
        if (_payload[0] == 0x30) {
                                                                            #if DEBUG > 3
                                                                                        Serial.println("SHD: PwmLight: OFF");
                                                                            #endif
            setBrightness(0);
        } else if (_payload[0] == 0x31) {
                                                                            #if DEBUG > 3
                                                                                        Serial.println("SHD: PwmLight: ON");
                                                                            #endif
            setBrightness(lastBrightnessGreaterZero);
        }
    } else if (strcmp(_topic, subTopicBrightness) == 0) {
                                                                            #if DEBUG > 3
                                                                                    Serial.print("SHD: PwmLight: Brightness set to ");
                                                                                    Serial.print(atoi((char*)_payload));
                                                                                    Serial.println(" %");
                                                                            #endif
        setBrightness(atoi((char*)_payload));
    } else {
        return false;
    }
}


void ShdPwmLight::setBrightness(uint8_t _percentage){
    if (_percentage < 0 || _percentage > 100) {
        return;
    }
    
    // save new set point
    setPoint = _percentage * 10; //
    
    if (_percentage > 0) {
        // save every percentage greater 0 for restoring after turning the light off and on
        lastBrightnessGreaterZero = setPoint;
        mqttPublish(pubTopicState, "1");
    } else {
        mqttPublish(pubTopicState, "0");
    }
    
    
    // publish new brightness
    char payload[5];
    snprintf (payload, 5, "%d", _percentage);
    mqttPublish(pubTopicBrightness, payload);
    
    // calculate delta:
    if (abs(currentBrightness - setPoint) >= 50) {
        delta = 50 / abs(currentBrightness - setPoint);
    } else {
        delta = 1;
    }
}

void ShdPwmLight::subscribe(){
    mqttSubscribe(this, subTopicBrightness);
#if DEBUG > 0
    Serial.print("SHD: PwmLight subscribed to ");
    Serial.println(subTopicBrightness);
#endif
    
    mqttSubscribe(this, subTopicState);
#if DEBUG > 0
    Serial.print("SHD: PwmLight subscribed to ");
    Serial.println(subTopicState);
#endif
}

void ShdPwmLight::republish(){
    if (currentBrightness != 0) {
        mqttPublish(pubTopicState, "1");
    } else {
        mqttPublish(pubTopicState, "0");
    }
    char payload[5];
    snprintf (payload, 5, "%d", setPoint);
    mqttPublish(pubTopicBrightness, payload);
}

bool ShdPwmLight::addIoInfo(){
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
