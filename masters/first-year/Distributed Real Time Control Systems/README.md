\# Distributed Real-Time Control Systems



Project developed for the Distributed Real-Time Control Systems course of the

Master's Degree in Electrical and Computer Engineering at Instituto Superior Técnico.



\## Project — Distributed Illumination Control



The project consisted of developing a real-time distributed control system for an

illumination setup composed of multiple networked luminaires.



The implementation uses Raspberry Pi Pico microcontrollers to perform local control,

measurement and communication.



\## Stage 1 — Local Illumination Controller



The first stage focused on controlling a single luminaire.



An LDR sensor measures the illuminance produced by the system.



The measured signal is converted into an illuminance estimate and used by a local

feedback controller.



The implementation included:



\- LDR measurement and calibration

\- ADC acquisition

\- PWM LED actuation

\- Background-light calibration

\- Static-gain estimation

\- PID control

\- Set-point weighting

\- Integral anti-windup

\- Measurement filtering

\- External disturbance detection

\- PWM saturation

\- Serial command interface



The controller adapts its behaviour when an external lighting disturbance is detected in

order to avoid aggressive control actions and visible flickering.



\## Stage 2 — Distributed Real-Time Architecture



The second stage extended the local controller into a distributed control system.



Both cores of the RP2040 microcontroller were used.



\### Core 0



Responsible primarily for:



\- Communication

\- CAN bus processing

\- Input/output tasks



\### Core 1



Responsible primarily for:



\- PID calculations

\- Distributed-control algorithms



This separation prevents communication activity from interfering with time-critical control

calculations.



\## CAN Communication



The distributed luminaires communicate over a CAN bus using an MCP2515 controller.



Shared state between the RP2040 cores is protected using hardware spinlocks to prevent

concurrent access to critical resources.



This provides synchronization without requiring a full real-time operating system.



\## Technologies and Concepts



\- Raspberry Pi Pico

\- RP2040 dual-core architecture

\- C/C++

\- PID control

\- PWM

\- ADC

\- LDR sensors

\- CAN bus

\- MCP2515

\- SPI

\- Hardware spinlocks

\- Real-time systems

\- Distributed control

\- Embedded systems

