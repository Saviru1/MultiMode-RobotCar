#include <Servo.h>
#include <NewPing.h>

// Operation Modes
#define OBSTACLE_AVOIDING_MODE 1
#define LINE_FOLLOWER_MODE 2
#define LINE_OBSTACLE_MODE 3
#define OBJECT_FOLLOWING_MODE 4

int currentMode = LINE_FOLLOWER_MODE;  // Default mode

// Pin Definitions
#define SERVO_PIN 3
#define ULTRASONIC_SENSOR_TRIG 11
#define ULTRASONIC_SENSOR_ECHO 12

// IR Sensor Pins
#define BOTTOM_IR_SENSOR_RIGHT A0
#define BOTTOM_IR_SENSOR_RIGHT_2 A4  // Additional right sensor
#define BOTTOM_IR_SENSOR_LEFT A1
#define BOTTOM_IR_SENSOR_LEFT_2 A5   // Additional left sensor
#define TOP_IR_SENSOR_RIGHT A3
#define TOP_IR_SENSOR_LEFT A2

// Motor Control Pins
int enableRightMotor = 5;
int rightMotorPin1 = 7;
int rightMotorPin2 = 8;
int enableLeftMotor = 6;
int leftMotorPin1 = 9;
int leftMotorPin2 = 10;

NewPing mySensor(ULTRASONIC_SENSOR_TRIG, ULTRASONIC_SENSOR_ECHO, 400);
Servo myServo;

void setup() {
  Serial.begin(9600); // Same baud rate as ESP8266
  pinMode(13, OUTPUT);  // Debugging LED on

  // Initialize motor control pins
  pinMode(enableRightMotor, OUTPUT);
  pinMode(rightMotorPin1, OUTPUT);
  pinMode(rightMotorPin2, OUTPUT);
  pinMode(enableLeftMotor, OUTPUT);
  pinMode(leftMotorPin1, OUTPUT);
  pinMode(leftMotorPin2, OUTPUT);

  // Initialize servo
  myServo.attach(SERVO_PIN);
  myServo.write(90);

  // Initialize IR sensors
  pinMode(BOTTOM_IR_SENSOR_RIGHT, INPUT);
  pinMode(BOTTOM_IR_SENSOR_LEFT, INPUT);
  pinMode(BOTTOM_IR_SENSOR_RIGHT_2, INPUT);
  pinMode(BOTTOM_IR_SENSOR_LEFT_2, INPUT);
  pinMode(TOP_IR_SENSOR_RIGHT, INPUT);
  pinMode(TOP_IR_SENSOR_LEFT, INPUT);
  
  // Stop motors initially
  rotateMotor(0, 0);   
}

void loop() {
  // Check for serial input from ESP8266
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    int mode = input.toInt();
    if (mode >= 1 && mode <= 4) {
      currentMode = mode;
      Serial.print("Mode changed to: ");
      Serial.println(currentMode);
    }
  }

  // Call the corresponding function based on the selected mode
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
  }
}

void rotateMotor(int rightMotorSpeed, int leftMotorSpeed) {
  // Right motor control
  if (rightMotorSpeed < 0) {
    digitalWrite(rightMotorPin1, LOW);
    digitalWrite(rightMotorPin2, HIGH);    
  } 
  else if (rightMotorSpeed >= 0) {
    digitalWrite(rightMotorPin1, HIGH);
    digitalWrite(rightMotorPin2, LOW);      
  }

  // Left motor control
  if (leftMotorSpeed < 0) {
    digitalWrite(leftMotorPin1, LOW);
    digitalWrite(leftMotorPin2, HIGH);    
  } 
  else if (leftMotorSpeed >= 0) {
    digitalWrite(leftMotorPin1, HIGH);
    digitalWrite(leftMotorPin2, LOW);      
  }

  // Set motor speeds
  analogWrite(enableRightMotor, abs(rightMotorSpeed));
  analogWrite(enableLeftMotor, abs(leftMotorSpeed));    
}

void obstacleAvoiding() {
  digitalWrite(13, HIGH);  // Debug LED on
  static int MAX_REGULAR_MOTOR_SPEED = 150;
  static int MAX_MOTOR_TURN_SPEED = 200;
  static int DISTANCE_TO_CHECK = 30;
  
  int distance = mySensor.ping_cm();

  if (distance > 0 && distance < DISTANCE_TO_CHECK) {
    // Obstacle detected - avoidance maneuver
    rotateMotor(0, 0);
    delay(500);  
    rotateMotor(-MAX_MOTOR_TURN_SPEED, -MAX_MOTOR_TURN_SPEED);        
    delay(200);
    rotateMotor(0, 0);
    delay(500);
    
    // Check left side
    myServo.write(180);
    delay(500);
    int distanceLeft = mySensor.ping_cm();    

    // Check right side
    myServo.write(0);    
    delay(500);    
    int distanceRight = mySensor.ping_cm();

    // Return to center
    myServo.write(90); 
    delay(500);        
    
    // Decide turning direction
    if (distanceLeft == 0) {
      rotateMotor(MAX_MOTOR_TURN_SPEED, -MAX_MOTOR_TURN_SPEED);
      delay(500);
    }
    else if (distanceRight == 0) {
      rotateMotor(-MAX_MOTOR_TURN_SPEED, MAX_MOTOR_TURN_SPEED);
      delay(500);
    }
    else if (distanceLeft >= distanceRight) {
      rotateMotor(MAX_MOTOR_TURN_SPEED, -MAX_MOTOR_TURN_SPEED);
      delay(500);
    }
    else {
      rotateMotor(-MAX_MOTOR_TURN_SPEED, MAX_MOTOR_TURN_SPEED);
      delay(500);      
    }
    rotateMotor(0, 0);    
    delay(200);     
  }
  else {
    // No obstacle - move forward
    rotateMotor(MAX_REGULAR_MOTOR_SPEED, MAX_REGULAR_MOTOR_SPEED);
  }  
}

void lineFollower() {
  digitalWrite(13, LOW);  // Debug LED off
  static int MOTOR_SPEED = 100;
  static int TURNING_MOTOR_SPEED = 150;
  
  // Read all four IR sensors
  int right1 = digitalRead(BOTTOM_IR_SENSOR_RIGHT);
  int right2 = digitalRead(BOTTOM_IR_SENSOR_RIGHT_2);
  int left1 = digitalRead(BOTTOM_IR_SENSOR_LEFT);
  int left2 = digitalRead(BOTTOM_IR_SENSOR_LEFT_2);

  // Line following logic with 4 sensors
  if (right1 == LOW && right2 == LOW && left1 == LOW && left2 == LOW) {
    // No line detected - go straight
    rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
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
  }
  else {
    // For other combinations, continue straight
    rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
  }
}

void lineWithObstacle() {
  digitalWrite(13, HIGH);  // Debug LED on
  static int MOTOR_SPEED = 100;
  static int TURNING_MOTOR_SPEED = 200;
  static int DISTANCE_TO_CHECK = 20;
    
  // Read all four IR sensors
  int right1 = digitalRead(BOTTOM_IR_SENSOR_RIGHT);
  int right2 = digitalRead(BOTTOM_IR_SENSOR_RIGHT_2);
  int left1 = digitalRead(BOTTOM_IR_SENSOR_LEFT);
  int left2 = digitalRead(BOTTOM_IR_SENSOR_LEFT_2);

  if (right1 == LOW && right2 == LOW && left1 == LOW && left2 == LOW) {
    int distance = mySensor.ping_cm();
    if (distance > 0 && distance < DISTANCE_TO_CHECK) {
      adjustRobotToAvoidObjectWhileFollowingLine();
    }
    else {
      rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
    }
  }
  else if ((right1 == HIGH || right2 == HIGH) && (left1 == LOW && left2 == LOW)) {
    rotateMotor(-TURNING_MOTOR_SPEED, TURNING_MOTOR_SPEED);
  }
  else if ((left1 == HIGH || left2 == HIGH) && (right1 == LOW && right2 == LOW)) {
    rotateMotor(TURNING_MOTOR_SPEED, -TURNING_MOTOR_SPEED);
  }
  else if (right1 == HIGH && right2 == HIGH && left1 == HIGH && left2 == HIGH) {
    rotateMotor(0, 0);
  }
  else {
    rotateMotor(MOTOR_SPEED, MOTOR_SPEED);
  }
}

void objectFollowing() {
  digitalWrite(13, LOW);  // Debug LED off
  // Implement object following code here
}