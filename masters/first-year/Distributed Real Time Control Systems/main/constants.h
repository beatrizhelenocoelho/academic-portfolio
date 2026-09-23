#ifndef CONSTANTS_H
#define CONSTANTS_H

//  PINS
const int LED_PIN = 15;
const int DAC_RANGE = 4095;
const int LDR_PIN = A0;

// LED driving circuit
const int R_led = 47; //ohm

// illuminance reading circuit
const float Vcc = 3.3; //volt
const float R_fixed = 10e3; //ohm
const float C = 10e-6; //farad

// PID CONSTANTS 
extern float m;
extern float b;
extern float kp_init;
extern float ki_init;
extern float b_init;
extern float kt_init;
const float h =0.01; //s

// EXTRAS
const float P_max = 0.02;

// Calibration Constants
//const float m = -0.8; 
// b = 5.86 para b1; b=6.12
// b2=6.3 b1=6.36 b3=6.24
//m2=-0.87
//const float b=6.24; //6.12 

#endif 