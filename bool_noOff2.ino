#include <Arduino.h>
#include <FastLED.h>
#include <Servo.h>

const int motorPins[] = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83};
const int dirPin = 12;
const int pulPin = 46;

const int driverOut1 = 84;
const int driverOut2 = 85;
const int servoPin = 12;
const int servoPWM = 13;

Servo myServo;

const int numPumps = sizeof(motorPins) / sizeof(motorPins[0]);
const int numStepsPerRevolution = 6400;

#define NUM_LEDS 32
#define LED_PIN 11

CRGB leds[NUM_LEDS];

// Number of LEDs in the moving group
#define GROUP_SIZE 5
float animation = 0;
float fadeAnimation = 0;

// Animation delay in milliseconds
#define ANIMATION_DELAY 100 // Decreased delay for faster movement

bool iswashing = false;
bool isLedAnimationRunning = false; // Track the state of the LED animation

void runPumps(int param1, int param2) {
  // Implement the pump logic here
  Serial.println("Running pumps");
}

void runServo(int angle, int runSpeed) {
  int mappedSpeed = map(runSpeed, 0, 2000, 0, 180);
  
  if (angle >= 0) {
    for (int i = 0; i <= angle; ++i) {
      myServo.write(i);
      delay(mappedSpeed);
    }
  } else {
    for (int i = 0; i >= angle; --i) {
      myServo.write(i);
      delay(mappedSpeed);
    }
  }
}

void runStepper(int angle, int runTime) {
  Serial.print("Running stepper motor, Angle: " + String(angle) + " Run Time: " + String(runTime) + "\r\n");
  //Serial.print(angle);
  //Serial.print(", Run time: ");
  //Serial.println(runTime);

  int direction = (angle >= 0) ? HIGH : LOW;
  angle = abs(angle);
  digitalWrite(dirPin, direction);

  // Calculate the number of steps based on the angle
  int steps = angle * numStepsPerRevolution / 360;

  // Run the stepper motor
  for (int i = 0; i < steps; i++) {
    digitalWrite(pulPin, HIGH);
    delayMicroseconds(runTime);
    digitalWrite(pulPin, LOW);
    delayMicroseconds(runTime);
  }

  // Run for the requested time
  //delay(runTime);
}


void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.setBrightness(50);
  Serial.begin(115200);
  FastLED.clear();
  FastLED.show();
}

void adiosMf() {
  // Initialize color values
  CRGB brownishYellow = CRGB(218, 165, 32); // Brownish Yellow color
  CRGB red = CRGB(255, 0, 0);

  // Set the LED animation state to running
  isLedAnimationRunning = true;

  while (isLedAnimationRunning) {
    // Move the group of brownish yellow LEDs from left to right on a red background
    for (int i = 0; i <= NUM_LEDS - GROUP_SIZE; i++) {
      for (int j = 0; j < GROUP_SIZE; j++) {
        leds[i + j] = CRGB::Green; // brownishYellow; // Set LEDs to brownish yellow
      }
      FastLED.show();
      delay(ANIMATION_DELAY);

      // Check for stop condition
      if (!isLedAnimationRunning) {
        // Turn off the LEDs and exit the loop
        FastLED.clear();
        FastLED.show();
        return;
      }

      // Clear the group
      for (int j = 0; j < GROUP_SIZE; j++) {
        leds[i + j] = red; // Set LEDs to red background
      }
    }

    // Move the group of brownish yellow LEDs from right to left on a red background
    for (int i = NUM_LEDS - GROUP_SIZE; i >= 0; i--) {
      for (int j = 0; j < GROUP_SIZE; j++) {
        leds[i + j] = CRGB::Green; //brownishYellow; // Set LEDs to brownish yellow
      }
      FastLED.show();
      delay(ANIMATION_DELAY);

      // Check for stop condition
      if (!isLedAnimationRunning) {
        // Turn off the LEDs and exit the loop
        FastLED.clear();
        FastLED.show();
        return;
      }

      // Clear the group
      for (int j = 0; j < GROUP_SIZE; j++) {
        leds[i + j] = CRGB::Pink; // Set LEDs to red background
      }
    }
  }
}

void processCommand(String command) {
  char separator = ',';

  int firstBracketIndex = command.indexOf('(');
  int secondBracketIndex = command.indexOf(')', firstBracketIndex + 1);

  if (firstBracketIndex != -1 && secondBracketIndex != -1) {
    // Extract motor type and SID from the first set of brackets
    String motorTypeAndSID = command.substring(firstBracketIndex + 1, secondBracketIndex);
    motorTypeAndSID.trim();  // Remove leading/trailing whitespaces
    int spaceIndex = motorTypeAndSID.indexOf(' ');

    if (spaceIndex != -1) {
      String motorType = motorTypeAndSID.substring(0, spaceIndex);
      String SID = motorTypeAndSID.substring(spaceIndex + 1);

      Serial.print("SID: ");
      Serial.println(SID);

      // Relay control logic based on motorType
      if (motorType == "PUMPMOTOR_OPERATION") {
        digitalWrite(driverOut1, HIGH);
        iswashing = true;
      } else if (motorType == "WASHING_OPERATION") {
        digitalWrite(driverOut2, HIGH);
        iswashing = true;
      } else if (motorType.equalsIgnoreCase("LED_OPERATION")) {
        // Extract the function name and value from the command
        int index = command.indexOf(separator, secondBracketIndex + 1);
        String functionAndValue = command.substring(index + 1, command.length() - 1);
        functionAndValue.trim();  // Remove leading/trailing whitespaces

        // Find the first comma after the function name
        int commaIndex = functionAndValue.indexOf(separator);

        if (commaIndex != -1) {
          // Extract the function name without parentheses
          String functionName = functionAndValue.substring(0, commaIndex);
          functionName.replace("(", "");  // Remove leading parenthesis
          functionName.trim();  // Remove leading/trailing whitespaces

          // Extract the value
          int value = functionAndValue.substring(commaIndex + 1).toInt();

          Serial.print("Function Name: ");
          Serial.println(functionName);
          Serial.print("Value: ");
          Serial.println(value);

          // Call the LED pattern function based on the value
          if (functionName.equalsIgnoreCase("adiosMf") && value == 1) {
            adiosMf(); // Start the LED pattern
          } else if (functionName.equalsIgnoreCase("adiosMf") && value == 0) {
            // Stop the LED pattern
            isLedAnimationRunning = false;
          } else {
            Serial.println("Unknown function name");
          }
        } else {
          Serial.println("Invalid LED_OPERATION format");
        }
      }

      // Extract pump number/angle and run time from subsequent brackets
      int index = secondBracketIndex + 1;
      while (index < command.length()) {
        int nextBracketIndex = command.indexOf('(', index);
        int endIndex = command.indexOf(')', nextBracketIndex + 1);

        if (nextBracketIndex != -1 && endIndex != -1) {
          String inputData = command.substring(nextBracketIndex + 1, endIndex);

          int commaIndex = inputData.indexOf(separator);
          if (commaIndex != -1) {
            int param1 = inputData.substring(0, commaIndex).toInt();
            int param2 = inputData.substring(commaIndex + 1).toInt();

            Serial.print(motorType);
            Serial.print(": Param1: ");
            Serial.print(param1);
            Serial.print(", Param2: ");
            Serial.println(param2);

            if (motorType == "PUMPMOTOR_OPERATION") runPumps(param1, param2);
            else if (motorType == "SERVOMOTOR_OPERATION") runServo(param1, param2);
            else if (motorType == "STEPPERMOTOR_OPERATION") runStepper(param1, param2);
            else Serial.println("Unknown motor type");
          } else {
            Serial.println("Invalid pump data format");
          }

          index = endIndex + 1;
        } else {
          break;
        }
      }
    } else {
      Serial.println("Invalid motor type and SID format");
    }
  } else {
    Serial.println("Invalid command format");
  }
}

void loop() {
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');
    processCommand(data);
  }
}
