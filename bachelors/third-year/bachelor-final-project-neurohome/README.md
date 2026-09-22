\# NeuroHome — Bachelor's Final Project



Final integrative project developed during the Bachelor's Degree in Electrical and Computer Engineering at Instituto Superior Técnico.



\## Project Overview



NeuroHome is a non-invasive home monitoring system designed to support elderly people who live alone.



The objective of the project was to develop a technological solution capable of monitoring daily routines inside a home while preserving the user's independence and avoiding wearable monitoring devices.



The system combines environmental and motion sensors, embedded controllers, local processing, cloud services and a caregiver application.



The main goal is not only to detect immediate dangerous situations, but also to identify changes in daily routines that may indicate potential health or well-being problems.



\## Problem



Elderly people living alone can be exposed to situations that are difficult to detect quickly.



Existing monitoring solutions often depend on wearable devices such as watches, bracelets or emergency buttons.



These solutions may require interaction from the elderly person, regular charging or permanent use of the device.



NeuroHome was designed as a passive monitoring system installed directly in the home.



Once installed, the system can operate without requiring the elderly person to actively interact with it.



\## Proposed Solution



Sensors are installed throughout the home to monitor:



\- Movement

\- Temperature

\- Gas concentration



Each room contains a secondary sensing unit.



The collected information is transmitted to a central processing unit, which analyses the data and identifies the user's current location and routine patterns.



The system can then send information and alerts to a cloud platform accessible by caregivers.



\## System Architecture



The system uses a distributed architecture composed of three main levels.



\### Sensor Nodes



Each monitored room contains sensors connected to an ESP32 microcontroller.



The ESP32 is responsible for:



\- Reading the sensors

\- Performing local processing

\- Detecting relevant events

\- Sending information to the central unit



The sensing system includes:



\- Motion sensors

\- Temperature sensors

\- Gas sensors



Battery-powered operation was considered in order to simplify installation and avoid permanent cabling throughout the home.



\### Central Processing Unit



A Raspberry Pi 4 acts as the main controller of the system.



It is responsible for:



\- Receiving information from the ESP32 nodes

\- Storing information locally

\- Processing movement information

\- Learning daily routine patterns

\- Detecting abnormal behaviour

\- Sending processed information and alerts to the cloud



\### Cloud and Caregiver Application



Processed information is made available through a caregiver-facing application.



The application provides information such as:



\- Current room occupied by the user

\- Time spent in each room

\- Daily activity history

\- Routine comparisons

\- Alerts

\- Historical data



\## Routine Monitoring



One of the main objectives of NeuroHome is to infer a person's normal routine from movement between rooms.



Instead of using cameras or wearable devices, movement sensors positioned throughout the home determine the approximate location of the user.



Over time, the system can establish typical behaviour patterns.



Changes in those patterns can then be identified and presented to caregivers.



Examples include:



\- Spending an unusually long time in the bathroom

\- Long nighttime periods outside the bedroom

\- Leaving the home for an unexpectedly long period

\- Significant changes in daily routine

\- Abnormally long periods of inactivity



\## Safety Monitoring



In addition to routine analysis, the system monitors environmental conditions.



Examples of situations that may trigger alerts include:



\- Elevated gas concentration

\- Abnormal temperatures

\- Unexpected periods of inactivity

\- Significant routine deviations



This allows the system to combine immediate home safety monitoring with longer-term behavioural analysis.



\## Prototype



The prototype integrates:



\- ESP32 microcontrollers

\- Raspberry Pi 4

\- Motion sensors

\- Temperature sensors

\- Gas sensors

\- Wi-Fi communication

\- Local processing

\- Cloud communication

\- Caregiver application



The architecture was designed to be modular, allowing additional rooms and sensing units to be added when required.



\## Prototype Results



The developed prototype achieved the following reported results:



| Test | Result |

| --- | ---: |

| Room detection accuracy | 85% |

| Routine pattern recognition | 70% |

| Gas and temperature detection | 90% |

| Timely alert generation | 90% |

| Installation time | < 30 min |

| Local communication delay | \~1 s |

| Cloud communication delay | \~10 s |



The prototype was also presented to caregivers and support institutions to obtain feedback regarding the usefulness and usability of the proposed system.



\## My Contribution



The project was developed by a multidisciplinary project team.



My work focused primarily on the data processing component of the system, including the processing and interpretation of sensor data used to analyse activity and routine patterns.



\## Project Documentation



This repository contains the main documentation produced for the project:



```text

bachelor-final-project-neurohome/

├── README.md

├── executive-summary.pdf

├── poster.pdf

└── presentations/

&#x20;   ├── pitch\_deck\_eq5.pdf

&#x20;   └── pitch\_deck\_gr5.pdf

