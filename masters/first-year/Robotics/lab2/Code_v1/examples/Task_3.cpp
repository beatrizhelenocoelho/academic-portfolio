#include <Arduino.h>

#define dirPin 9
#define stepPin 8
#define sw1Pin 11 // Right switch
#define sw2Pin 10 // Left switch

// Variables for calibration
int left, right, middle;
const int CALIBRATION_SPEED = 1000; // Microseconds delay

void moveMotor1(int steps, int safetyPin1);
void calibrate_limits();

void setup() {
    Serial.begin(9600);

    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    
    pinMode(sw1Pin, INPUT_PULLUP);
    pinMode(sw2Pin, INPUT_PULLUP);

    Serial.println("Starting Centering Calibration...");
    delay(1000); 

    calibrate_limits();
    
    Serial.println("Done! Motor is now at center. Standing by.");
}

void loop() {
    // This is empty so the motor stops after calibration
}

void moveMotor1(int steps, int safetyPin1) {
    // Sets direction: HIGH for positive steps, LOW for negative
    digitalWrite(dirPin, steps > 0 ? HIGH : LOW); 

    for (int i = 0; i < abs(steps); i++) {
        // Read the switch. If it's LOW, then the switch was pressed
        if (digitalRead(safetyPin1) == HIGH) { 
            digitalWrite(stepPin, HIGH);
            delayMicroseconds(CALIBRATION_SPEED);
            digitalWrite(stepPin, LOW);
            delayMicroseconds(CALIBRATION_SPEED);
        } else {
            break; // Stop stepping if switch is hit
        }
    }
}

void calibrate_limits() {
    left = 0;
    right = 0;

    // Move until it hits the Left Switch (sw2Pin)
    Serial.println("Step 1: Finding Left Limit...");
    while (digitalRead(sw2Pin) == HIGH) {
        moveMotor1(10, sw2Pin);
        left += 10;
    }
    
    delay(100);

    // Move until it hits the Right Switch (sw1Pin) and count steps
    Serial.println("Step 2: Finding Right Limit & Measuring distance...");
    while (digitalRead(sw1Pin) == HIGH) {
        moveMotor1(-10, sw1Pin);
        right += 10;
    }

    // Calculate the middle
    middle = right / 2;
    Serial.print("Total Span: "); Serial.println(right);
    Serial.print("Moving to Middle (steps): "); Serial.println(middle);

    // Move back to the center. Since we are currently at the 
    // right switch, we move "Forward" to get to center
    moveMotor1(middle, sw2Pin); 
}