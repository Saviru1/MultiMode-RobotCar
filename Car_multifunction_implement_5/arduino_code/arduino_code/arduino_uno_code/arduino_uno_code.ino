#include <Servo.h>
#include <NewPing.h>

// Operation Modes
#define OBSTACLE_AVOIDING_MODE 1
#define LINE_FOLLOWER_MODE 2
#define LINE_OBSTACLE_MODE 3
#define OBJECT_FOLLOWING_MODE 4
#define REMOTE_CONTROL_MODE 5
#define BATTERY_MODE 6

int currentMode = LINE_FOLLOWER_MODE;
int currentSpeed = 150;
float batteryPercentage = 0;

// Pin Definitions
#define SERVO_PIN 3
#define ULTRASONIC_SENSOR_TRIG 11
#define ULTRASONIC_SENSOR_ECHO 12

// IR Sensor Pins
#define BOTTOM_IR_SENSOR_RIGHT A0
#define BOTTOM_IR_SENSOR_RIGHT_2 A4
#define BOTTOM_IR_SENSOR_LEFT A1
#define BOTTOM_IR_SENSOR_LEFT_2 A5
#define TOP_IR_SENSOR_RIGHT A3
#define TOP_IR_SENSOR_LEFT A2

// Motor Control Pins
int enableRightMotor = 5;
int rightMotorPin1 = 7;
int rightMotorPin2 = 8;
int enableLeftMotor = 6;
int leftMotorPin1 = 9;
int leftMotorPin2 = 10;

String remoteCommand = "stop";
NewPing mySensor(ULTRASONIC_SENSOR_TRIG, ULTRASONIC_SENSOR_ECHO, 400);
Servo myServo;

unsigned long lastLineDetectedTime = 0;
const unsigned long LINE_LOST_TIMEOUT = 1000;
unsigned long lastBatteryRequestTime = 0;
const unsigned long BATTERY_REQUEST_INTERVAL = 2000;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);

  // Motor pins
  pinMode(enableRightMotor, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);
  pinMode(enableLeftMotor, OUTPUT);
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);

  // Servo
  myServo.attach(SERVO_PIN);
  myServo.write(90);

  // IR sensors
  pinMode(BOTTOM_IR_SENSOR_RIGHT, INPUT);
  pinMode(BOTTOM_IR_SENSOR_RIGHT_2, INPUT);
  pinMode(BOTTOM_IR_SENSOR_LEFT, INPUT);
  pinMode(BOTTOM_IR_SENSOR_LEFT_2, INPUT);
  pinMode(TOP_IR_SENSOR_RIGHT, INPUT);
  pinMode(TOP_IR_SENSOR_LEFT, INPUT);

  rotateMotor(0, 0);
}

void loop() {
  updateBatteryStatus();
  handleSerialCommands();
  
  switch (currentMode) {
    case OBSTACLE_AVOIDING_MODE:
      obstacleAvoiding();
      break;
    case LINE_FOLLOWER_MODE:
      lineFollower();
      break;
    case LINE_OBSTACLE_MODE:
      lineWithObstacle();
      break;
    case OBJECT_FOLLOWING_MODE:
      objectFollowing();
      break;
    case REMOTE_CONTROL_MODE:
      remoteControl();
      break;
    case BATTERY_MODE:
      batteryPerformance();
      break;
    default:
      rotateMotor(0, 0);
      break;
  }
}

void handleSerialCommands() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input.startsWith("mode:")) {
      int mode = input.substring(5).toInt();
      if (mode >= 1 && mode <= 6) {
        currentMode = mode;
        Serial.print("Mode changed to: ");
        Serial.println(currentMode);
        if (currentMode != REMOTE_CONTROL_MODE) {
          remoteCommand = "stop";
          rotateMotor(0, 0);
        }
      }
    }
    else if (input.startsWith("remote:")) {
      if (currentMode == REMOTE_CONTROL_MODE) {
        String command = input.substring(7);
        if (command == "forward" || command == "backward" || 
            command == "left" || command == "right" || 
            command == "stop") {
          remoteCommand = command;
          Serial.print("Remote command: ");
          Serial.println(remoteCommand);
        }
      }
    }
    else if (input.startsWith("speed:")) {
      int speed = input.substring(6).toInt();
      if (speed >= 0 && speed <= 255) {
        currentSpeed = speed;
        Serial.print("Speed set to: ");
        Serial.println(currentSpeed);
      }
    }
    else if (input == "getBattery") {
      Serial.print("battery:");
      Serial.println(batteryPercentage);
    }
  }
}

void updateBatteryStatus() {
  if (millis() - lastBatteryRequestTime > BATTERY_REQUEST_INTERVAL) {
    Serial.println("getBattery");
    lastBatteryRequestTime = millis();
  }
}

void batteryPerformance() {
  digitalWrite(13, HIGH);
  rotateMotor(0, 0);
  
  static unsigned long lastDisplay = 0;
  if (millis() - lastDisplay > 1000) {
    Serial.print("Battery: ");
    Serial.print(batteryPercentage);
    Serial.println("%");
    lastDisplay = millis();
  }
}

void remoteControl() {
  digitalWrite(13, HIGH);
  
  if (remoteCommand == "forward") {
    rotateMotor(currentSpeed, currentSpeed);
  } 
  else if (remoteCommand == "backward") {
    rotateMotor(-currentSpeed, -currentSpeed);
  } 
  else if (remoteCommand == "left") {
    // Left turn - right motor forward, left motor backward
    rotateMotor(currentSpeed, -currentSpeed);
  } 
  else if (remoteCommand == "right") {
    // Right turn - left motor forward, right motor backward
    rotateMotor(-currentSpeed, currentSpeed);
  } 
  else { // "stop"
    rotateMotor(0, 0);
  }
}

void rotateMotor(int rightMotorSpeed, int leftMotorSpeed) {
  // Constrain speeds to valid range
  rightMotorSpeed = constrain(rightMotorSpeed, -255, 255);
  leftMotorSpeed = constrain(leftMotorSpeed, -255, 255);
  
  // Right motor control
  if (rightMotorSpeed > 0) {
    digitalWrite(rightMotorPin1, HIGH);
    digitalWrite(rightMotorPin2, LOW);
  } else if (rightMotorSpeed < 0) {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, HIGH);
  } else {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, LOW);
  }

  // Left motor control
  if (leftMotorSpeed > 0) {
    digitalWrite(leftMotorPin1, HIGH);
    digitalWrite(leftMotorPin2, LOW);
  } else if (leftMotorSpeed < 0) {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, HIGH);
  } else {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, LOW);
  }

  analogWrite(enableRightMotor, abs(rightMotorSpeed));
  analogWrite(enableLeftMotor, abs(leftMotorSpeed));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void obstacleAvoiding() {
  digitalWrite(13, HIGH);  // Debug LED on
  static int MAX_REGULAR_MOTOR_SPEED = 150;
  static int MAX_MOTOR_TURN_SPEED = 200;
  static int DISTANCE_TO_CHECK = 30;
  
  int distance = mySensor.ping_cm();

  if (distance > 0 && distance < DISTANCE_TO_CHECK) {
    // Obstacle detected - avoidance maneuver
    rotateMotor(0, 0);
    delay(300);  
    rotateMotor(-MAX_MOTOR_TURN_SPEED, -MAX_MOTOR_TURN_SPEED);        
    delay(300);
    rotateMotor(0, 0);
    delay(300);
    
    // Check left side distance
    myServo.write(160);  // Not full 180 to prevent mechanical stress
    delay(500);
    int distanceLeft = mySensor.ping_cm();    

    // Check right side distance
    myServo.write(20);  // Not full 0 to prevent mechanical stress   
    delay(500);    
    int distanceRight = mySensor.ping_cm();

    // Return to center
    myServo.write(90); 
    delay(300);        
    
    // Decide turning direction with better logic
    if (distanceLeft == 0 && distanceRight == 0) {
      // Both sides blocked - turn around
      rotateMotor(MAX_MOTOR_TURN_SPEED, -MAX_MOTOR_TURN_SPEED);
      delay(1000);
    }
    else if (distanceLeft == 0) {
      // Left blocked - turn right
      rotateMotor(-MAX_MOTOR_TURN_SPEED, MAX_MOTOR_TURN_SPEED);
      delay(500);
    }
    else if (distanceRight == 0) {
      // Right blocked - turn left
      rotateMotor(MAX_MOTOR_TURN_SPEED, -MAX_MOTOR_TURN_SPEED);
      delay(500);
    }
    else if (distanceLeft >= distanceRight) {
      // More space on left - turn left
      rotateMotor(MAX_MOTOR_TURN_SPEED, -MAX_MOTOR_TURN_SPEED);
      delay(500);
    }
    else {
      // More space on right - turn right
      rotateMotor(-MAX_MOTOR_TURN_SPEED, MAX_MOTOR_TURN_SPEED);
      delay(500);      
    }
    
    // Move forward after turning
    rotateMotor(MAX_REGULAR_MOTOR_SPEED, MAX_REGULAR_MOTOR_SPEED);
    delay(300);     
  }
  else {
    // No obstacle - move forward
    rotateMotor(MAX_REGULAR_MOTOR_SPEED, MAX_REGULAR_MOTOR_SPEED);
  }  
}

void lineFollower() {
  digitalWrite(13, LOW);  // Debug LED off
  static int BASE_MOTOR_SPEED = 150;
  static int TURNING_MOTOR_SPEED = 200;
  static int LINE_LOST_SPEED = 100; // Slower speed when searching for line
  
  // Read all four IR sensors
  int right1 = digitalRead(BOTTOM_IR_SENSOR_RIGHT);
  int right2 = digitalRead(BOTTOM_IR_SENSOR_RIGHT_2);
  int left1 = digitalRead(BOTTOM_IR_SENSOR_LEFT);
  int left2 = digitalRead(BOTTOM_IR_SENSOR_LEFT_2);

  // Line following logic with 4 sensors
  if (right1 == HIGH || right2 == HIGH || left1 == HIGH || left2 == HIGH) {
    lastLineDetectedTime = millis(); // Update last line detection time
  }

  // If all sensors don't see the line but we saw it recently, continue with last action
  if (right1 == LOW && right2 == LOW && left1 == LOW && left2 == LOW) {
    if (millis() - lastLineDetectedTime < LINE_LOST_TIMEOUT) {
      // Continue with last action (forward)
      rotateMotor(LINE_LOST_SPEED, LINE_LOST_SPEED);
      return;
    }
  }

  // Main line following logic
  if (right1 == LOW && right2 == LOW && left1 == LOW && left2 == LOW) {
    // No line detected - go straight (or search pattern if lost for too long)
    rotateMotor(BASE_MOTOR_SPEED, BASE_MOTOR_SPEED);
  }
  else if ((right1 == HIGH || right2 == HIGH) && (left1 == LOW && left2 == LOW)) {
    // Right sensors detect line - turn right
    rotateMotor(-TURNING_MOTOR_SPEED, TURNING_MOTOR_SPEED);
  }
  else if ((left1 == HIGH || left2 == HIGH) && (right1 == LOW && right2 == LOW)) {
    // Left sensors detect line - turn left
    rotateMotor(TURNING_MOTOR_SPEED, -TURNING_MOTOR_SPEED);
  }
  else if (right1 == HIGH && right2 == HIGH && left1 == HIGH && left2 == HIGH) {
    // All sensors detect line - stop (likely at an intersection)
    rotateMotor(0, 0);
    delay(500);
    // Decision point - could add logic here for intersections
    rotateMotor(BASE_MOTOR_SPEED, BASE_MOTOR_SPEED);
    delay(300);
  }
  else {
    // For other combinations (like both inner sensors), continue straight
    rotateMotor(BASE_MOTOR_SPEED, BASE_MOTOR_SPEED);
  }
}

void lineWithObstacle() {
  digitalWrite(13, HIGH);  // Debug LED on
  static int BASE_MOTOR_SPEED = 150;
  static int TURNING_MOTOR_SPEED = 200;
  static int OBSTACLE_DISTANCE = 20;
    
  // First check for obstacles
  int distance = mySensor.ping_cm();
  if (distance > 0 && distance < OBSTACLE_DISTANCE) {
    adjustRobotToAvoidObjectWhileFollowingLine();
    return;
  }

  // If no obstacle, proceed with line following
  lineFollower();
}

void objectFollowing() {
  digitalWrite(13, LOW);  // Debug LED off
  static int BASE_SPEED = 150;
  static int TURN_ADJUSTMENT = 100;
  static int MIN_DISTANCE = 10;
  static int MAX_DISTANCE = 30;
  static int TOO_CLOSE_DISTANCE = 5;
  
  int distance = mySensor.ping_cm();
  int rightIRSensorValue = digitalRead(TOP_IR_SENSOR_RIGHT);
  int leftIRSensorValue = digitalRead(TOP_IR_SENSOR_LEFT);

  // If object is too close, back up
  if (distance > 0 && distance < TOO_CLOSE_DISTANCE) {
    rotateMotor(-BASE_SPEED, -BASE_SPEED);
    delay(300);
    return;
  }

  // If right sensor detects hand, turn right
  if (rightIRSensorValue == LOW && leftIRSensorValue == HIGH) {
    rotateMotor(BASE_SPEED - TURN_ADJUSTMENT, BASE_SPEED + TURN_ADJUSTMENT); 
  }
  // If left sensor detects hand, turn left
  else if (rightIRSensorValue == HIGH && leftIRSensorValue == LOW) {
    rotateMotor(BASE_SPEED + TURN_ADJUSTMENT, BASE_SPEED - TURN_ADJUSTMENT); 
  }
  // If distance is between min and max, go straight
  else if (distance >= MIN_DISTANCE && distance <= MAX_DISTANCE) {
    rotateMotor(BASE_SPEED, BASE_SPEED);
  }
  // Otherwise stop
  else {
    rotateMotor(0, 0);
  }  
}

void adjustRobotToAvoidObjectWhileFollowingLine() {
  static int TURN_SPEED = 200;
  static int FORWARD_SPEED = 150;
  static int TURN_DURATION = 500;
  static int FORWARD_DURATION = 500;
  static int CORRECTION_DURATION = 800;

  // Stop motors
  rotateMotor(0, 0);
  delay(300);
    
  // Check left side distance
  myServo.write(160);
  delay(500);
  int distanceLeft = mySensor.ping_cm();    

  // Check right side distance
  myServo.write(20);    
  delay(500);    
  int distanceRight = mySensor.ping_cm();

  // Return to center
  myServo.write(90); 
  delay(300);        

  bool turnLeft = false;
  
  // Decision making for turn direction
  if (distanceLeft == 0 && distanceRight == 0) {
    // Both sides blocked - turn around
    rotateMotor(TURN_SPEED, -TURN_SPEED);
    delay(TURN_DURATION * 2);
    turnLeft = true;
  }
  else if (distanceLeft == 0 || distanceRight >= distanceLeft) {
    // Left blocked or more space on right - turn right
    rotateMotor(-TURN_SPEED, TURN_SPEED);
    delay(TURN_DURATION);
  }
  else {
    // Right blocked or more space on left - turn left
    rotateMotor(TURN_SPEED, -TURN_SPEED);
    delay(TURN_DURATION);
    turnLeft = true;
  }
  
  // Move forward after turning
  rotateMotor(FORWARD_SPEED, FORWARD_SPEED);    
  delay(FORWARD_DURATION); 

  // Correct path after obstacle
  if (turnLeft) {
    rotateMotor(-TURN_SPEED, TURN_SPEED);
  } else {
    rotateMotor(TURN_SPEED, -TURN_SPEED);
  }
  delay(CORRECTION_DURATION);
  
  // Try to find the line again
  rotateMotor(FORWARD_SPEED/2, FORWARD_SPEED/2);
  delay(300);
}