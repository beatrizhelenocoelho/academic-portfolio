\# Mobile Networks and Internet of Things



Project developed for the Mobile Networks and Internet of Things course of the

Master's Degree in Electrical and Computer Engineering at Instituto Superior Técnico.



\## Project — Smart Wine Cellar



Smart Wine Cellar is an IoT monitoring and control system designed to supervise wine

storage conditions remotely.



The prototype monitors environmental conditions inside a wine cellar and sends the

information through a mobile network to a cloud-connected application.



\## Monitored Parameters



The system measures:



\- Temperature

\- Humidity

\- Air quality

\- Wine-container level



The measurements are collected by sensors connected to an Arduino MKR NB 1500.



\## Embedded System



The Arduino performs periodic sensor acquisition and processes the measurements locally.



The hardware includes:



\- DHT11 temperature and humidity sensor

\- MQ135 air-quality sensor

\- Ultrasonic level sensor

\- Ventilation fan

\- Status LEDs

\- Buzzer

\- Arduino MKR NB 1500



The ultrasonic sensor is used to estimate the liquid level and convert it into both

percentage and volume.



\## Alert System



Thresholds are defined for temperature, humidity, air quality and liquid level.



The system generates warnings when abnormal conditions are detected.



It also controls status LEDs to provide local indication of whether the monitored

environment is operating normally.



\## Automatic Ventilation Control



The ventilation system can operate automatically.



The fan is activated when:



\- Temperature exceeds the configured threshold

\- Unsafe air-quality conditions are detected



The fan can also be manually controlled from the remote application.



\## Mobile Network and MQTT



Sensor data is transmitted through the Arduino MKR NB 1500 mobile connection.



MQTT is used as the communication protocol between the embedded system and the remote

services.



Measurements are published as JSON messages.



\## Firebase Bridge



A Python application acts as a bidirectional bridge between MQTT and Firebase.



The bridge:



1\. Receives sensor measurements from MQTT.

2\. Parses the JSON payload.

3\. Updates the Firebase Realtime Database.

4\. Monitors Firebase for remote fan-control commands.

5\. Publishes those commands back to the Arduino using MQTT.



This allows communication in both directions between the physical system and the

application.



\## System Architecture



```text

Sensors

&#x20;  ↓

Arduino MKR NB 1500

&#x20;  ↓ Mobile Network

MQTT Broker

&#x20;  ↓

Python MQTT/Firebase Bridge

&#x20;  ↓

Firebase Realtime Database

&#x20;  ↓

Mobile Application

