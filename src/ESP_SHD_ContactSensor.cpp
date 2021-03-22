#include "ESP_SHD_ContactSensor.h"
uint8_t ShdContactSensor::contactSensorCount = 0;

ShdContactSensor::ShdContactSensor(contactSensorType _type, bool _sleepAfterPublish, bool _getBatteryStatus)
  : ShdContactSensor(_type, 10, false, _sleepAfterPublish, 1000, _getBatteryStatus) { }

ShdContactSensor::ShdContactSensor(contactSensorType _type, uint8_t _pin, bool _lowAcive, bool _sleepAfterPublish, uint16_t _intervalMillis, bool _getBatteryStatus) : swSerial(04,05) {
  csNumber = ++contactSensorCount;
  csType = _type;
  pin = _pin;
  lowActive = _lowAcive;
  sleepAfterPublish = _sleepAfterPublish;
  updateInterval = _intervalMillis/5;
  getBatteryStatus = _getBatteryStatus;
  if (updateInterval < 1) {
    updateInterval = 1;
  }
  updateCounter = updateInterval;


  #if DEBUG > 0
  swSerial.begin(9600);
  pinMode(4, INPUT);
  pinMode(5, OUTPUT);
  swSerial.println();
  swSerial.println("-----------------------");
  swSerial.println("SoftwareSerial started.");
  #endif

  getStatus();

  snprintf(pubTopicState, MQTT_TOPIC_SIZE, "%s/ContactSensor/%d/getStatus", name, csNumber);
  if (getBatteryStatus) {
    snprintf(pubTopicBattery, MQTT_TOPIC_SIZE, "%s/ContactSensor/%d/getBatteryStatus", name, csNumber);
  }

}

void ShdContactSensor::timer5msHandler() {

  //--------------REMOVE
  swSerial.println("5 ms");
  //------- REMOVE ABOVE

  if (sleepAfterPublish) {
    return;
  }

  if (++updateCounter < updateInterval) {
    return;
  }
  updateCounter = 0;

  // getStatus();

  republish();
}

bool ShdContactSensor::handleMqttRequest(char* _topic, unsigned char* _payload, uint16_t _length) {
  return false;
}

void ShdContactSensor::republish() {
  swSerial.println("republish()");
  // mqttPublish(pubTopicState, payloadBuffer);
  // switch (lastStatus) {
  //   case CS_OPENED:
  //     mqttPublish(pubTopicState, "true");
  //     break;
  //   case CS_CLOSED:
  //     mqttPublish(pubTopicState, "false");
  //     break;
  //   case CS_ERROR:
  //     mqttPublish(pubTopicState, "ERROR 1");
  //     break;
  //   default:
  //     break;
  // }

  if (getBatteryStatus) {
    // ....
  }

  if (sleepAfterPublish) {
    delay(500);
    swSerial.println("deepSleep");
    ESP.deepSleep(0);
  }
}

void ShdContactSensor::getStatus() {
  switch (csType) {
    case CS_NORMAL:
      lastStatus = getStatusNormal();
      break;
    case CS_LSC_DOOR_SENROR:
      lastStatus = getStatusLsc();
      break;
  }
}

csStatus ShdContactSensor::getStatusNormal() {
  if (digitalRead(pin) && !lowActive || !digitalRead(pin) && lowActive) {
      return CS_CLOSED;
  } else {
    return CS_OPENED;
  }
}

csStatus ShdContactSensor::getStatusLsc() {

  swSerial.println("TEST 1");
  Serial.begin(9600);

  uint8_t reply[14];
  uint8_t replySize;



  /* Befehle an die MCU
    lscCommunication(1,0,0,...)  get device info from MCU
    lscCommunication(2,1,0,...)  LED flashing fast
    lscCommunication(2,1,1,...)  power on, LED slashing slowly
    lscCommunication(2,1,2,...)  LED off
    lscCommunication(2,1,3,...)  ??
    lscCommunication(2,1,4,...)  get device status von MCU
  */


  lscCommunication(1, 0, 0, reply, 14, &replySize);
  // delay(500);
  // for (uint8_t i = 0; i < 14; i++) {
  //   reply[i] = 0;
  // }
  // lscCommunication(2, 1, 1, reply, 14, &replySize);
  //lscCommunication(2, 1, 4, reply, 14, &replySize);


  lscCommunication(2, 1, 4, reply, 13, &replySize);

  // for (size_t i = 0; i < 50; i++) {
  //   payloadBuffer[0] = 0;
  // }
  // snprintf(payloadBuffer, 50, "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x s: %i", reply[0], reply[1], reply[2], reply[3], reply[4], reply[5], reply[6], reply[7], reply[8], reply[9], reply[10], reply[11], reply[12], reply[13], replySize);

  /* serial data from MCU:
     0      1    2        3    4       5       6   7   8   9     10   11
 sync1  sync2       msgType       length                       data
    55     aa    0        5    0       5       1   1   0   1      1    d        CLOSED
    55     aa    0        5    0       5       1   1   0   1      0    c        OPENED
    55     aa    0        5    0       5       3   4   0   1      2   13        after reboot
    55     aa    0        3    0       0       2                                button long press
    55     aa    0        1    0      24      7b  22  70  22     3a   22   69
  */

  // double check checksum:
  uint8_t cs = 0;
  for (size_t i = 0; i < 6+reply[5]; i++) {
    cs += reply[i];
  }
  if (cs != reply[6 + reply[5]]) {
    // checksum wrong:
    return CS_ERROR;
  }

  // check message type (byte 3), message lenght (byte 5) and status (byte 6)
  if (reply[3] == 5 && reply[5] == 5 && reply[6] == 1) {
    if (reply[10] == 1) {
      return CS_CLOSED;
    } else if (reply[10] == 0) {
      return CS_OPENED;
    }
  }

  if (reply[3] == 3) {
    sleepAfterPublish = !sleepAfterPublish;
  }


  return CS_ERROR;
}

void ShdContactSensor::lscCommunication(byte _cmd, byte _len, byte _value, uint8_t* _reply, uint8_t _maxReplySize, uint8_t* _replySize) {

  // wait for old bla to end:
  while (Serial.available()) {}

  // send message:
  swSerial.print("Sent: ");
  Serial.write(0x55);
  Serial.write(0xAA);
  Serial.write(0x00);
  swSerial.print(" 55 AA 00 ");
  Serial.write(_cmd);
  swSerial.print(_cmd, 16);
  Serial.write(0x00);
  swSerial.print(" 00 ");
  Serial.write(_len);
  swSerial.print(_len, 16);
  swSerial.print(" ");
  if(_len == 1) {
    Serial.write(_value);
    swSerial.print(_value, 16);
    swSerial.print(" ");
  };
  byte checksum = 0xff + _cmd + _len + _value;
  Serial.write(checksum);
  swSerial.print(checksum, 16);
  Serial.flush();
  swSerial.println();
  // prep to logging:
  uint8_t count = 0;

  // wait for reply:
  for (uint16_t i = 0; i < 1000; i++) {
    if (i == 999) {
      return;
    }
    if (Serial.available()) {
      break;
    }
    delay(1);
  }

  swSerial.print("Recv: ");
  // process reply:
  while (Serial.available()) {
    uint8_t data = Serial.read();

    if (count < _maxReplySize && _reply != NULL) {
      _reply[count] = data;
      count++;
    }
    swSerial.print(data,16);
    swSerial.print(" ");

    // wait for next serial data:
    delay(2);
  }
  swSerial.println("; ");

  *_replySize = count;
}
