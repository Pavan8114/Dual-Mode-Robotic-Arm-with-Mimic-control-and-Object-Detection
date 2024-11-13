#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Constants for buttons
const int button1Pin = 2; // Pin for record button
const int button2Pin = 3; // Pin for playback button

// PCA9685 setup
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVOMIN 150 // Min pulse length
#define SERVOMAX 600 // Max pulse length

// Define servo control variables
#define NUM_SERVOS 4
#define MAX_STEPS 180
int servoPositions[NUM_SERVOS][MAX_STEPS]; // Array to store recorded positions
int currentStep = 0;
int recordedSteps = 0;
bool isRecording = false;
bool isPlaying = false;

// Pin connections for potentiometers
const int potPins[NUM_SERVOS] = {A0, A1, A2, A3};
int potValues[NUM_SERVOS] = {0};

// Control state tracking
bool lastButton1State = HIGH;
bool lastButton2State = HIGH;

// Servo control helper function
void setServoAngle(int servo, int angle) {
  int pulse = map(angle, 0, 180, SERVOMIN, SERVOMAX);
  pwm.writeMicroseconds(servo, pulse);
}

void setup() {
  Serial.begin(115200);
  pwm.begin();
  pwm.setPWMFreq(50); // Set frequency to 50Hz

  // Configure button pins
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);

  // Initialize servos to default positions
  for (int i = 0; i < NUM_SERVOS; i++) {
    setServoAngle(i, 90); // Start at 90 degrees
  }
}

void loop() {
  // Read button states
  bool button1State = digitalRead(button1Pin);
  bool button2State = digitalRead(button2Pin);

  // Check if record button is pressed
  if (button1State == LOW && lastButton1State == HIGH) {
    isRecording = !isRecording;
    if (isRecording) {
      currentStep = 0; // Reset step count for new recording
      Serial.println("Recording started...");
    } else {
      recordedSteps = currentStep; // Save total recorded steps
      Serial.println("Recording stopped.");
    }
  }
  lastButton1State = button1State;

  // Check if play button is pressed
  if (button2State == LOW && lastButton2State == HIGH) {
    if (recordedSteps > 0) {
      isPlaying = !isPlaying;
      if (isPlaying) {
        currentStep = 0;
        Serial.println("Playback started...");
      } else {
        Serial.println("Playback stopped.");
      }
    }
  }
  lastButton2State = button2State;

  // Record positions if recording mode is active
  if (isRecording) {
    for (int i = 0; i < NUM_SERVOS; i++) {
      potValues[i] = analogRead(potPins[i]);
      int angle = map(potValues[i], 0, 1023, 0, 180);
      setServoAngle(i, angle);
      if (currentStep < MAX_STEPS) {
        servoPositions[i][currentStep] = angle;
      }
    }
    currentStep++;
  }

  // Playback recorded positions if playback mode is active
  if (isPlaying) {
    for (int i = 0; i < NUM_SERVOS; i++) {
      int angle = servoPositions[i][currentStep];
      setServoAngle(i, angle);
    }
    currentStep++;
    if (currentStep >= recordedSteps) {
      currentStep = 0; // Loop playback
    }
    delay(20); // Adjust for smoother motion
  }
}
