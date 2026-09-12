#include <Arduino.h> 

const int SWITCH_PIN = 2; // ON/OFF switch on digital pin 2
const int PAUSE_PIN = 3; // Pause/Play button on INT1
const int FLOW_PIN = A0; // Flow sensor that is output on analog pin A0
const int POT_PIN = A1; // Potentiometer wiper on analog pin A1
const int LED_PIN = 8; // Warning LED on digital pin 8 
const int MOTOR_PIN = 9; // PWM output to MOSFET gate on digital pin 9

const int THRESHOLD = 20; // There is a 100 mV threshold and each ADC step is  5V/1023 = 4.89 mV, multiply this by 20 to get 100 mV, therefor the threhold is 20.

int motorSpeed = 0; // Last PWM value, ranging from 0-255
volatile bool motorRunning = true; // volatile; the ISR can change this at any time, so the main loop must check it each time through the loop

void pauseISR() { // Runs the instant the button is pressed
    motorRunning = !motorRunning; // Toggle the motorRunning variable between true and flase
} 

void setup() { 
    pinMode(SWITCH_PIN, INPUT_PULLUP); // Set the switch pin as an input with internal pull-up resistor enabled, the pin reads HIGH until the switch pulls it to GND
    pinMode(PAUSE_PIN, INPUT_PULLUP); // Button wired to GND, reads HIGH until pressed
    pinMode(LED_PIN, OUTPUT); // Set the LED pin as an output which drives current out
    pinMode(MOTOR_PIN, OUTPUT); // PWM pin must be an OUTPUT to drive the MOSFET gate

    attachInterrupt(digitalPinToInterrupt(PAUSE_PIN), pauseISR, FALLING); // When the button is pressed, the ISR will run and toggle the motorRunning variable
}

void loop() {

    if (digitalRead(SWITCH_PIN) == HIGH) { // If the switch_pin reads as HIGH then the switch is open meaning the system is off, and the pull-up holds it at 5V
        analogWrite(MOTOR_PIN, 0); // Turn the motor off by setting the PWM duty to 0
        digitalWrite(LED_PIN, LOW); // Warning LED is off when the system is off
        motorSpeed = 0; // Reset the motor speed to 0 so it restarts
        delay(1000); // Wait 1 second before checking the switch again
        return; // Skip the rest of the loop() and start over
    }

if (!motorRunning) { // If the motor is paused
    analogWrite(MOTOR_PIN, 0); // Turn the motor off 
    digitalWrite(LED_PIN, HIGH); // Warning LED on
    delay(100); // Wait 100 ms before checking the switch again
    return; // Skip the rest of the loop() and start over
    }

int measured = analogRead(FLOW_PIN); // Read the flow sensor
int desired = analogRead(POT_PIN); // Read the potentiometer\\

if (abs(measured - desired) > THRESHOLD) { // If the difference between the measured and desired values is greater than the threshold
    digitalWrite(LED_PIN, HIGH); // Turn the warning LED on
} else {
    digitalWrite(LED_PIN, LOW); // Turn the warning LED off
}

if (measured > desired) { // If the flow is too fast
    motorSpeed = max(motorSpeed -1, 0); // Decrease the motor speed by 1, but don't let it go below 0
} else if (measured < desired) { // If the flow is too slow
    motorSpeed = min(motorSpeed +1, 255); // Increase the motor speed by 1, but don't let it go above 255
    }

analogWrite(MOTOR_PIN, motorSpeed); // Write the current motor speed to the PWM to control the motor 

delay (1000); // Wait 1 second before checking the switch again
} 
