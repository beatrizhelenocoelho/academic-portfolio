\# Systems Programming



Project developed for the Systems Programming course of the Master's Degree in

Electrical and Computer Engineering at Instituto Superior Técnico.



\## Project — Space Trash



The objective of this project was to develop a distributed real-time simulation game in C.



The simulated universe is threatened by the accumulation of space debris.



Players control trash ships that navigate through the universe, collect space debris and

transport it to recycling planets.



A central server maintains the state of the universe and executes the physical simulation,

while clients communicate with the server to control individual ships.



\## System Architecture



The project is divided into several components:



\### Universe Simulator



The universe simulator is responsible for creating and updating the simulated environment.



It manages:



\- Planets

\- Space debris

\- Object positions

\- Velocities and accelerations

\- Gravitational forces

\- Collision behaviour

\- Graphical rendering



The physical state of the universe is continuously updated according to the implemented

simulation rules.



\### Universe Server



The server maintains the shared state of the multiplayer simulation.



It is responsible for:



\- Accepting client connections

\- Managing multiple trash ships

\- Processing player commands

\- Updating ship positions

\- Managing trash collection

\- Managing trash deposition on recycling planets

\- Synchronizing the global universe state



\### Trash-Ship Client



The client allows the player to interact with the simulated universe.



It receives keyboard input and sends movement commands to the server.



The client also receives information about the current state of the universe.



\### Communication



Communication between components was implemented using network messaging.



Protocol messages are defined using Protocol Buffers, allowing structured communication

between the different components.



The project also uses ZeroMQ for distributed communication.



\### Concurrency



The final version of the system uses multiple execution threads to separate tasks such as:



\- Network communication

\- Physics calculations

\- Server processing

\- Client interaction



This allows the simulation and communication components to operate concurrently.



\### Dashboard



A separate dashboard was developed to monitor information from the distributed simulation.



The dashboard receives information from the system and displays statistics about the

current universe state.



\## Configuration



Simulation parameters are stored in configuration files.



These parameters define properties such as:



\- Universe dimensions

\- Number of planets

\- Trash limits

\- Simulation parameters



\## Technologies and Concepts



\- C

\- Python

\- POSIX threads

\- ZeroMQ

\- Protocol Buffers

\- Socket communication

\- SDL

\- libconfig

\- Distributed systems

\- Real-time simulation

\- Concurrent programming

\- Client-server architecture

