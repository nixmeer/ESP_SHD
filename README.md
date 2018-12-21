# ESP_SmartHomeDevice
Keep it simple and stupid. Everyone should be able to build his or her smart home.

This project has two step plan:

1st: Create a platform that can be used by every unexperienced coder to create a personalized smart home device (shd) based on a ESP8266.

2nd: Add a web server that allows non-programmers (such as the coders mom) can change settings of the device.

# Basic Code Structure

There is a separate class for very function which is implemented as a child class of ESP_SmartHomeDevice. After initializing ESP_SmartHomeDevice, objects of these classes can be created and used. All of these object use a single mqtt client which is provided by ESP_SmartHomeDevice. All children have to implement a couple of functions that can be called by ESP_SmartHomeDevice. These are:

1st: bool handleMqttRequest(char * _topic, unsigned char * _payload, uint16_t _length);

2nd: void resubscribe();

3rd: void timer5msHandler();


# Basic usage in Arduino

Call "ESP_SmartHomeDevice::init()" to set the name of the shd and connect the mqtt client to the broker. Arguments are either just "char * _name", if the broker is discoverable using mDNS, or "char * _mqttServerAddress, uint16_t _port, char * _name ".

# Motion Sensor

To add a motion sensor, just call "new ShdMotionSensor(5)". It publishes it's status to "_name/Motion" (_name = argument of ESP_SmartHomeDevice::init())

# Temperature Sensor

To add a motion sensor, just call "new ShdMotionSensor(5)". It publishes it's status to "_name/Temperature" (_name = argument of ESP_SmartHomeDevice::init())

# WS2812b strip
Call

# mosquitto broker on raspberry pi

You can use any mqtt broker for this project. In order to have all devices run stable and reconnect after a loss of power, it's recommended to use mDNS to expose the mqtt broker service to the network so all SHDs can connect to it. After installing mosquitto on the raspberry pi, call "sudo nano /etc/avahi/services/mqtt.service". Fill the following:

"<?xml version="1.0" standalone='no'?>
<!DOCTYPE service-group SYSTEM "avahi-service.dtd">
<service-group>
 <name replace-wildcards="yes">MQTT on %h</name>
  <service>
   <type>_mqtt._tcp</type>
   <port>1883</port>
  </service>
</service-group>"

Then reboot. SHDs should find the broker now automatically.
