// Right now this is the code for Task 4. To run the code for 
// Task 3, just copy the contents of the file 'Task_3.cpp' here.

#include <Arduino.h>

#include "AK09918.h"
#include "ICM20600.h"
#include <Stepper.h>
#include <Wire.h>

#include <math.h>

#define STEPS 200
#define dirPin 9
#define stepPin 8
#define sw1Pin 11
#define sw2Pin 10

// Define stepper motor connections and motor interface type,
// motor interface type must be set to 1 when using a driver
Stepper stepper(STEPS, 8, 9);
#define motorInterfaceType 1

int left, right, middle;

AK09918_err_type_t err;
int32_t x, y, z;
AK09918 ak09918;
ICM20600 icm20600(true);
int16_t acc_x, acc_y, acc_z;
int32_t offset_x, offset_y, offset_z;
double roll, pitch;
const int deadzone = 10;
float stepper_delay;
bool sleep = true;

// Find the magnetic declination at your location
// http://www.magnetic-declination.com/
double declination_lisbon = -1.1;

// Read interval variables
unsigned long imu_read_last = 0;
const int imu_read_interval = 50; // Read sensors every 50ms (20Hz)

void calibrate(uint32_t timeout, int32_t* offsetx, int32_t* offsety, int32_t* offsetz);
void calibrate_limits();
void setup_imu();
void moveMotor1(int steps, int safetyPin1);
void moveMotorBack(int safetyPin1, int safetyPin2);

void setup() {
    // Join I2C bus
    Wire.begin();
    Serial.begin(9600);

    // Motor pins
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(sw1Pin, INPUT);
    pinMode(sw2Pin, INPUT);

    calibrate_limits();

    setup_imu();

    // Initial values
    pitch = 0;
    stepper_delay = 100000;
    sleep = true;
    digitalWrite(dirPin, HIGH);
}

void loop() {
    unsigned long current_time = millis();
    
    if (current_time - imu_read_last >= imu_read_interval) {
        imu_read_last = current_time;

        // Get accelerations
        acc_x = icm20600.getAccelerationX();
        acc_y = icm20600.getAccelerationY();
        acc_z = icm20600.getAccelerationZ();

        // Pitch calculation
        pitch = atan2(-(float)acc_x, sqrt((float)acc_y*acc_y+(float)acc_z*acc_z))*57.3; // In degrees

        if (pitch == 0 || abs(pitch) < deadzone) {
            sleep = true;
        } else {
            sleep = false;
            // Set stepper delay according to pitch (speed)
            stepper_delay = 50000/abs(pitch); // Higher pitch -> lower stepper_delay -> faster movement
            // Set direction
            digitalWrite(dirPin, pitch < 0 ? HIGH : LOW);
        }
    }

    if(!sleep) {
        // Move stepper
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(stepper_delay); // Changes the speed
        digitalWrite(stepPin, LOW);
        delayMicroseconds(stepper_delay); // Changes the speed
    }

    // Check safety switches
    if (digitalRead(sw1Pin)==0 || digitalRead(sw2Pin)==0) {
        moveMotorBack(digitalRead(sw1Pin), digitalRead(sw2Pin));
    }    
}

void setup_imu() {
    err = ak09918.initialize();
        icm20600.initialize();
        ak09918.switchMode(AK09918_POWER_DOWN);
        ak09918.switchMode(AK09918_CONTINUOUS_100HZ);


        err = ak09918.isDataReady();
        while (err != AK09918_ERR_OK) {
            Serial.println("Waiting Sensor");
            delay(100);
            err = ak09918.isDataReady();
        }

        Serial.println("Start figure-8 calibration after 2 seconds.");
        delay(2000);
        calibrate(5000, &offset_x, &offset_y, &offset_z);
        Serial.println("");
}

void calibrate(uint32_t timeout, int32_t* offsetx, int32_t* offsety, int32_t* offsetz) {
    int32_t value_x_min = 0;
    int32_t value_x_max = 0;
    int32_t value_y_min = 0;
    int32_t value_y_max = 0;
    int32_t value_z_min = 0;
    int32_t value_z_max = 0;
    uint32_t timeStart = 0;

    ak09918.getData(&x, &y, &z);

    value_x_min = x;
    value_x_max = x;
    value_y_min = y;
    value_y_max = y;
    value_z_min = z;
    value_z_max = z;
    delay(100);

    timeStart = millis();

    while ((millis() - timeStart) < timeout) {
        ak09918.getData(&x, &y, &z);

        /* Update x-Axis max/min value */
        if (value_x_min > x) {
            value_x_min = x;
            // Serial.print("Update value_x_min: ");
            // Serial.println(value_x_min);

        } else if (value_x_max < x) {
            value_x_max = x;
            // Serial.print("update value_x_max: ");
            // Serial.println(value_x_max);
        }

        /* Update y-Axis max/min value */
        if (value_y_min > y) {
            value_y_min = y;
            // Serial.print("Update value_y_min: ");
            // Serial.println(value_y_min);

        } else if (value_y_max < y) {
            value_y_max = y;
            // Serial.print("update value_y_max: ");
            // Serial.println(value_y_max);
        }

        /* Update z-Axis max/min value */
        if (value_z_min > z) {
            value_z_min = z;
            // Serial.print("Update value_z_min: ");
            // Serial.println(value_z_min);

        } else if (value_z_max < z) {
            value_z_max = z;
            // Serial.print("update value_z_max: ");
            // Serial.println(value_z_max);
        }

        Serial.print(".");
        delay(100);

    }

    *offsetx = value_x_min + (value_x_max - value_x_min) / 2;
    *offsety = value_y_min + (value_y_max - value_y_min) / 2;
    *offsetz = value_z_min + (value_z_max - value_z_min) / 2;
}

void calibrate_limits() {
    left = 0;
    right = 0;

    Serial.println("Going left"); // When viewed from the robot frame
    
    // It's the right when facing the front of the robot
    
    while (digitalRead(sw2Pin)==1) {
        // Go up until the switch is reached
        moveMotor1( 10, sw2Pin );
        left +=10;
    }
    Serial.print("Left at ");
    Serial.print(left);
    Serial.print("   ");
    Serial.print(digitalRead(sw2Pin));
    Serial.print("  ");
    Serial.println(digitalRead(sw1Pin));
    // At this point the motor is at the highest position

    Serial.println("Going right");
    while (digitalRead(sw1Pin)==1) {
        // Do down until the switch is reached
        moveMotor1( -10, sw1Pin );
        right +=10;
    }
    Serial.print("Right at ");
    Serial.print(right);
    Serial.print("   ");
    Serial.print(digitalRead(sw2Pin));
    Serial.print("  ");
    Serial.println(digitalRead(sw1Pin));
    
    middle = right / 2;
    moveMotor1(middle, sw2Pin);
    Serial.print("Middle point at  ");
    Serial.println(middle);

    Serial.println("Setup completed");
}

void moveMotor1(int steps, int safetyPin1) {

  digitalWrite(dirPin, steps > 0 ? HIGH : LOW); // Set direction

  for (int i=0; i<abs(steps); i++) {

    if (digitalRead(safetyPin1)==1) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(1000);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(1000); 
    }
    else {
      break;
    }
  }
}

void moveMotorBack(int safetyPin1, int safetyPin2){
    // Set direction backwards from the safety switch
    if(safetyPin1 == 0){
        digitalWrite(dirPin, HIGH);
    }
    else if(safetyPin2 == 0){
        digitalWrite(dirPin, LOW);
    }

    // Quickly move back 5 steps
    for(int i=0; i<100; i++){
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(500);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(500); 
    }
}