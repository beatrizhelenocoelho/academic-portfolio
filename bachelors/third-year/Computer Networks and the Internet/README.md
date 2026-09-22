\# Named Data Networking Node



Project developed for the Computer Networks and the Internet course of the Bachelor's Degree in Electrical and Computer Engineering at Instituto Superior Técnico.



\## Project Objective



The objective of this project was to implement a network node capable of participating in a content-oriented network.



Unlike traditional IP communication, where communication is primarily based on destination addresses, this application allows nodes to request and distribute objects using their names.



The program was implemented in C using TCP and UDP sockets.



\## Network Node



Each running instance represents a node in the network.



A node can:



\- Join a network

\- Connect directly to another node

\- Accept connections from other nodes

\- Create local objects

\- Request objects by name

\- Forward object requests

\- Store received objects in a local cache

\- Display network topology information

\- Leave the network



\## Object Storage



Each node maintains a list of stored objects.



Objects may either be:



\- Locally created by the node

\- Received from another node and stored in cache



When an object is requested, the node first checks whether it is already available locally.



If it is available, the object is returned immediately.



Otherwise, the request may be propagated through the network.



\## Interest Table



The implementation maintains a Pending Interest Table.



For each requested object, the table records the interfaces associated with that request.



This allows the node to keep track of pending requests and forward a received object back through the appropriate interfaces.



\## Network Communication



The application uses both TCP and UDP communication.



\### TCP



TCP sockets are used for persistent communication between neighbouring nodes.



Each node creates a listening TCP socket and accepts incoming connections from other nodes.



\### UDP



UDP communication is used for network-management operations.



\## Event Handling



Multiple network connections are handled using `select()`.



This allows the application to monitor:



\- Standard input

\- Listening sockets

\- Active neighbour connections

\- Network events



without creating a separate thread for every connection.



\## Available Commands



The command-line interface includes operations such as:



```text

join

direct join

show topology

leave

create object

retrieve object

exit

