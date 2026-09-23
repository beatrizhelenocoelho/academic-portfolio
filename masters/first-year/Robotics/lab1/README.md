Requirements:



-> A computer running the Windows operating system;



-> Matlab R2025b or later installed with these Packages/Toolboxes/Add-ons installed:

 	- Automated Driving Toolbox;

 	- Computer Vision Toolbox;

 	- Deep Learning Toolbox;

 	- Image Processing Toolbox version;

 	- MATLAB Support for MinGW-w64 C/C++/Fortran Compiler version;

 	- Model Predictive Control Toolbox;

 	- Sensor Fusion and Tracking Toolbox;

 	- Simulink;

 	- Simulink 3D Animation version;



-> A controller/gamepad with a USB cable to connect to your computer;



Preparation:



-> To be able to use the controller/gamepad with simulink, you might need to install a software like "DS4Windows" for your computer to recognize the controller/gamepad correctly. There are several tutorials on Youtube on how to use this software and all the set up required, this is one of them: https://www.youtube.com/watch?v=pa61YwXXiw4



Steps:



-> Unzip the file "**Robotis**.zip";



-> Open the folder "**LaneKeepingAssistWithLaneDetectionExample**" inside the folder that was extracted from the .zip file in matlab;



-> In the Matlab "Command Window", run the command "helperLKACleanUp" and then command "helperLKASetUp" (this might take a few minutes);



-> Once the symboll ">>" appears after running the command "helperLKASetUp", open the simulink by clicking twice in the file "LKATestBenchExample.slx" on the Matlab "Files" window;



-> With the simulink window open, connect your controller/gamepad to your computer via USB cable;



->Enable Fast Restart to compile only for the first simulation;



-> Click the button "run" to start the simulation (this might also take a few minuts until the actual simulation starts), a new window should appear showing the 3D simulation

