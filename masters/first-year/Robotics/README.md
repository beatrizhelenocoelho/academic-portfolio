\# Robotics



Projects developed for the Robotics course of the Master's Degree in

Electrical and Computer Engineering at Instituto Superior Técnico.



\## Overview



The course included two practical robotics projects addressing different areas of robotics:

autonomous vehicle assistance and human augmentation.



A technical paper on autonomous vehicles was also developed, focusing on robotic

architectures, decision-making and the challenges associated with autonomous driving.



\## Lab 1 — Lane Tracing Assist System



The objective of the first project was to design and simulate a Lane Tracing Assist (LTA)

system for a vehicle.



The project was developed in MATLAB and Simulink using an existing Lane Keeping Assist

example as the starting point.



The original system was modified by implementing:



\- A simplified kinematic bicycle model for the vehicle

\- A PD steering controller

\- Lateral deviation estimation

\- Heading error estimation

\- Smooth activation of the lane-assistance system

\- Driver override using turn indicators

\- Camera-based lane detection

\- Visualization of the vehicle and road environment in simulation



A simulated onboard camera was used to detect lane boundaries and estimate the vehicle's

position relative to the centre of the lane.



The controller uses the lateral deviation and heading error to calculate the steering

correction required to keep the vehicle inside the lane.



\## Lab 2 — Human Augmentation Device



The second project consisted of developing a one-degree-of-freedom human augmentation

device designed to assist users with limited mobility.



The prototype was designed as a button-pushing assistant capable of controlling the

position of a linear slider using hand gestures.



An IMU mounted on the user measures motion and orientation.



The system estimates the pitch angle of the IMU and uses it to control the velocity and

direction of a NEMA 17 stepper motor connected to the slider.



The implementation included:



\- IMU sensor acquisition

\- I2C communication

\- Pitch estimation from accelerometer measurements

\- Gesture-based control

\- Stepper motor control

\- Slider position control

\- Embedded C++ implementation

\- Arduino Uno

\- PlatformIO



The relationship between IMU inclination and motor stepping frequency allows the user to

control the slider naturally by tilting the sensor.



\## Autonomous Vehicles Study



A separate technical paper was developed on the challenges of autonomous vehicles from a

robotics perspective.



The study addressed:



\- Levels of driving automation

\- Autonomous vehicle architectures

\- Perception

\- Planning

\- Control

\- Safety

\- Decision-making

\- Ethical challenges



\## Project Structure



```text

robotics/

├── README.md

├── lab1/

│   ├── Robotics\_Lab1\_report.pdf

│   └── LaneKeepingAssistWithLaneDetectionExample/

├── lab2/

│   ├── Robotics\_Lab2\_report\_v1.pdf

│   └── Code\_v1/

└── report/

&#x20;   └── challenges-in-autonomous-vehicles.pdf

