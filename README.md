# MQTTtrial
Connecting and ESP32 with an Android app via an MQTT broker

## Setup

Board
- The "IoT" board is an Arduino Nano ESP32, but I believe any ESP will do.
- The board has a potmeter between GND and 3V3, and the slider on A0.
- The board has and RGB led connected to three GPIO pins (that is standard on the Nano ESP32).
- I have written Arduino [firmware](mqtt)

MQTT broker
- The board sends MQTT messages to an MQTT broker.
- I used [test.mosquitto.org](test.mosquitto.org) as test broker.
- It is free, and I used no account, so anybody who knows the topic names can see/spoof them.

MQTT client
- As an MQTT client I used [IoT MQTT panel](https://play.google.com/store/apps/details?id=snr.lab.iotmqttpanel.prod).
- In the app create a dashboard (connection name is up to you, client id is left blank, broker is test.mosquitto.org, port is 1883, network is TCP).
- Add eg a gauge and fill out MCPmqtt1POT (see #define below) as topic name.
- Add eg a text log and fill out MCPmqtt1CNT (see #define below) as topic name, single payload, digital display.
-  Add eg a button and fill out MCPmqtt1BUT (see #define below) as topic name, and red as payload, make the button color red.

## Photos

![Board](images/board.jpg)

![App screenshot](images/IoT_MQTT_Panel.jpg)


(end)
