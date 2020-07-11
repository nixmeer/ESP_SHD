# ESP_SmartHomeDevice
Keep it simple and stupid. Everyone should be able to build his or her smart home device (SHD).
## What this project is supposed to do:
This code runs on ESP8266 and is optimized to work together with [HomeBridge](https://github.com/nfarina/homebridge) and [mqttthing](https://github.com/arachnetech/homebridge-mqttthing)
- [x] create an easy to use platform that uses wifi and mqtt to talk to [mqttthing](https://github.com/arachnetech/homebridge-mqttthing)
- [ ] print json-config-code for homebridge-mqttthing
- [ ] create a webserver and a settings class that allow setup via GUI
## Supported devices
- [x] Stateless switches
- [x] TMP36 sensors
- [x] Motion sensors
- [x] Single color PWM light
- [x] Temperature PWM light
- [x] WS2512b strips
- [x] Sprinkler
- [x] Relay/Switch
## Libraries and cores used
- [Arduino core for ESP8266 WiFi chip](https://github.com/esp8266/Arduino) (v2.4.2)
- [pubSubClient](https://github.com/knolleary/pubsubclient) by knolleary (v2.7.0)
- [FastLED](https://github.com/FastLED/FastLED) by FastLED (v3.3.2)
- [WiFiManager](https://github.com/tzapu/WiFiManager) by tzapu (v0.15.0)
## Code Structure
There is a separate class for each device. Every device is implemented as a child class of ESP_SmartHomeDevice. After initializing ESP_SmartHomeDevice, objects of these classes may be created. All of these objects use one common mqtt client provided by ESP_SmartHomeDevice. Each child class of ESP_SmartHomeDevice has to implement the following member functions:
- `bool handleMqttRequest(char * _topic, unsigned char * _payload, uint16_t _length)` is being called after a request has been pushed to the mqtt client
- `void resubscribe()` is being called after the connection to the mqtt broker was lost
- `void timer5msHandler()` is being called every 5 ms
## Basic usage in Arduino
```
ESP_SmartHomeDevice::init(const char* _mqttServerAddress, uint16_t _port, char* _name);
ESP_SmartHomeDevice::init(char* _name)
```
Call `ESP_SmartHomeDevice::init(...)` to setup and initialize wifi and mqtt in the `setup()` function of your .ino file. If there is no known wifi network, a wifi access point will be created which let's you connect the ESP to a wifi network. If you use **one** local mqtt broker and it is discoverable via mDNS, call `ESP_SmartHomeDevice::init(char* _name)`. The shd then automatically connects to it. If it discovers 0 or more than one, it resets itself.
Now just create new devices using `new Shd...()`.
MQTT topics do always begin with the `_name` parameter initialized here.
## Creating devices
### Motion Sensor
To add a motion sensor, just call `new ShdMotionSensor(uint8_t _pin)`. It publishes it's status (`true` or `false` as char) to `_name/Motion`.
### Temperature Sensor
To add a temperature sensor, just call `new ShdTmp36Sensor()`. No pin is needed, since there is only one ADC available in the ESP8266. It publishes the temperature to `_name/Temperature`.
### WS2812b strip
Only stips at pin 4 are Supported.
Call ` ShdWs2812bStrip::initStrip(uint16_t _numberOfLeds, uint16_t _updateInterval)`. First argument sets the number of LEDs of the entire strip, second argument sets the update interval for the strip in milliseconds. 41 ms (24 fps) looked jerky, 25 ms (40 fps) looks much better.
Call `new ShdWs2812bStrip(uint16_t _firstLed, uint16_t _lastLed, uint16_t _ignitionPoint, ignitionDirection _ignitionDirection, uint8_t _hopsPerShow, uint8_t _flankLength)` to create a light section, multiple can be created for one strip and be used simultaneously. These sections can overlap (not yet tested).
- `_firstLed` is this section's first LED (value 1 or greater)
- `_lastLed` is this section's last LED (less or equal to `_numberOfLeds`)
- `_ignitionPoint` is the LED that all effects start from
- `_ignitionDirection` is `IGNITION_SINGLE_FORWARD`, `IGNITION_SINGLE_BACKWARD`, `IGNITION_BOTH_FORWARD` or `IGNITION_BOTH_BACKWARD`
- `_hopsPerShow` is the number of LEDs that the turn-on effect is shifted each frame. This has an impact on the effects speed.
- `_flankLength` sets the number of LEDs that are needed to transfer from one color to another
ShdWs2812bStrip subscribes to `_name/Lamp/_sectionNumber/setColor` and `_name/Lamp/_sectionNumber/setStatus`. `_sectionNumber`is increased for every new section starting at 1.
ShdWs2812bStrip publishes its status (0 and 1) to `_name/Lamp/_sectionNumber/getStatus`, its color to `_name/Lamp/_sectionNumber/getColor` and publishes its brightness to `_name/Lamp/_sectionNumber/getBrightness`.
### Button
To add a new button, call `new ShdButton(uint8_t _pin, bool _lowActive, uint32_t _millisDebounce, uint32_t _millisLongClick, uint32_t _millisMultiClick)`. It publishes to `_name/Button/_buttonNumber`. It detects single clicks ("1"), double clicks("2") and long presses ("L"). `_buttonNumber` is increased for every new button starting at 1.
### Relay
To ad a new relay, use `new ShdRelay(uint8_t _pin, uint32_t _millisBetweenToggle, bool _lowActive)`. It subscribes `_name/Relay/_relayNumber/setStatus` where `_name` is the device name and `_relayNumber` counts the created relays starting at 1. It publishes its status to `_name/Relay/_relayNumber/getStatus`. `_millisBetweenToggle` sets the minimum time between to toggles avoiding the relay to be switched to quickly.
# Hardware
This repository contains the eagle and gerber files of my SHD hardware. The PCBs can be ordered from jlcpcb.com or any other pcb manufacturer. The following parts are necessary for a basic assembly:
- ESP12f module
- XC6206 voltage regulator
- 3 resistors (10k0)
- pinheader (7 pins)
To programm the ESP8266, you can use any 3.3 V FTDI board. I use the ESP01-flasher and created a adapter cable using jumper wires and heat shrink tubing. Note: most ESP01-flashers need to have a GND connection to GPIO 0 added to put the ESP8266 to flash mode.
# mosquitto broker on raspberry pi
You can use any mqtt broker with this code. In order to have all devices run stable and reconnect after a loss of power, it's recommended to use mDNS to expose the mqtt broker service to the network, so all SHDs can automatically connect to it. After installing mosquitto on the raspberry pi, call `sudo nano /etc/avahi/services/mqtt.service`. Fill the following:
```
<?xml version="1.0" standalone='no'?>
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
 <name replace-wildcards="yes">MQTT on %h</name>
  <service>
   <type>_mqtt._tcp</type>
   <port>1883</port>
  </service>
</service-group>
```
Then reboot. SHDs should find the broker now automatically.
