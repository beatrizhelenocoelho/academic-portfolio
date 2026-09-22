\# Pioneer 3DX Occupancy Grid Mapping



Project developed for the Autonomous Systems course of the Bachelor's Degree in Electrical and Computer Engineering at Instituto Superior Técnico.



\## Project Objective



The objective of this project was to implement and evaluate a 2D mapping system for a mobile robot operating in an initially unknown environment.



A Pioneer 3DX mobile robot equipped with a Hokuyo laser range finder was used to collect real sensor data.



The project focused on the mapping problem assuming that the robot's position was known or estimated separately.



The implemented system generates an occupancy grid representation of the environment using laser measurements and robot pose information.



\## Occupancy Grid Mapping



The environment is represented as a grid of square cells.



Each cell can represent:



\- Free space

\- Occupied space

\- Unknown space



Laser measurements are projected from the robot position into the grid.



Bresenham's line algorithm is used to determine the cells crossed by each laser beam.



Cells crossed by the laser are considered evidence of free space, while the final cell associated with an obstacle receives evidence of occupancy.



Instead of directly storing probabilities, the implementation uses a log-odds representation, allowing occupancy estimates to be updated iteratively as new sensor measurements are received.



\## Robot and Sensors



The experimental platform consisted of:



\- Pioneer 3DX mobile robot

\- Hokuyo URG-04LX laser range finder

\- Wheel encoder odometry

\- ROS



The Hokuyo sensor publishes laser measurements through the `/scan` ROS topic.



Robot odometry is obtained through the `/odom` frame and used to estimate the robot's position and orientation.



\## Mapping Implementation



The mapping algorithm was implemented in Python and integrated with ROS.



For every incoming laser scan, the system:



1\. Obtains the robot pose.

2\. Processes each valid laser measurement.

3\. Calculates the position of the detected obstacle.

4\. Converts world coordinates into grid coordinates.

5\. Applies Bresenham's line algorithm between the robot and obstacle.

6\. Updates free cells and occupied cells using log-odds.

7\. Converts the resulting values into occupancy probabilities.

8\. Publishes the generated occupancy grid.

9\. Allows the map to be exported as an image and ROS-compatible YAML file.



\## Odometry Analysis and Calibration



During real-world experiments, accumulated odometry error caused visible distortion in the generated maps, particularly after completing long trajectories.



To evaluate this effect, the Pioneer was driven along controlled trajectories and the estimated path was analysed.



The robot's left and right motor gains were experimentally adjusted to reduce systematic drift.



The initial configuration used:



```text

Left motor gain:  1.0

Right motor gain: 1.0

