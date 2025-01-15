#include <SoftwareSerial.h>
#include <Servo.h>

// Pin definitions
const int ULTRASONIC_TRIGGER_PIN = 19;
const int ULTRASONIC_ECHO_PIN = 18;

const int RIGHT_MOTOR_IN1 = 11;
const int RIGHT_MOTOR_IN2 = 9;
const int RIGHT_MOTOR_ENA = 6;

const int LEFT_MOTOR_IN3 = 8;
const int LEFT_MOTOR_IN4 = 7;
const int LEFT_MOTOR_ENB = 5;

const int LINE_FOLLOWING_SENSOR_LEFT = 2;
const int LINE_FOLLOWING_SENSOR_MIDDLE = 4;
const int LINE_FOLLOWING_SENSOR_RIGHT = 10;

Servo servo;
SoftwareSerial bluetooth(0, 1); 

enum Mode {
  BLUETOOTH_MODE,
  LINE_FOLLOWING_MODE,
  OBSTACLE_AVOIDANCE_MODE
};

enum CarState {
  FORWARD,
  TURN_RIGHT,
  TURN_LEFT,
  BACKWARD,
  STOP
};

Mode currentMode = BLUETOOTH_MODE;

void setup() {
  pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(RIGHT_MOTOR_IN1, OUTPUT);
  pinMode(RIGHT_MOTOR_IN2, OUTPUT);
  pinMode(RIGHT_MOTOR_ENA, OUTPUT);
  pinMode(LEFT_MOTOR_IN3, OUTPUT);
  pinMode(LEFT_MOTOR_IN4, OUTPUT);
  pinMode(LEFT_MOTOR_ENB, OUTPUT);
  pinMode(LINE_FOLLOWING_SENSOR_LEFT, INPUT);
  pinMode(LINE_FOLLOWING_SENSOR_MIDDLE, INPUT);
  pinMode(LINE_FOLLOWING_SENSOR_RIGHT, INPUT);

  Serial.begin(9600);
  bluetooth.begin(9600);
  servo.attach(3);
  setServoToCenter();
}

void loop() {
  if (bluetooth.available()) {
    char command = bluetooth.read();
    
    // Mode selection
    switch (command) {
      case '1':
        currentMode = BLUETOOTH_MODE;
        break;
      case '2':
        currentMode = LINE_FOLLOWING_MODE;
        break;
      case '3':
        currentMode = OBSTACLE_AVOIDANCE_MODE;
        break;
    }
    
    if (currentMode == BLUETOOTH_MODE) {
      handleBluetoothCommands(command);
    }
  }

  switch (currentMode) {
    case BLUETOOTH_MODE:
      // Commands handled in bluetooth.available() check
      break;
    
    case LINE_FOLLOWING_MODE:
      executeLineFollowing();
      break;
    
    case OBSTACLE_AVOIDANCE_MODE:
      executeObstacleAvoidance();
      break;
  }
}

void handleBluetoothCommands(char command) {
  switch (command) {
    case 'F':
      moveForwardWithSpeed(100);
      break;
    case 'B':
      moveBackwardWithSpeed(100);
      break;
    case 'L':
      turnLeftWithSpeed(50);
      break;
    case 'R':
      turnRightWithSpeed(50);
      break;
    default:
      stopCar();
      break;
  }
}

void executeLineFollowing() {
  int leftSensor = digitalRead(LINE_FOLLOWING_SENSOR_LEFT);
  int middleSensor = digitalRead(LINE_FOLLOWING_SENSOR_MIDDLE);
  int rightSensor = digitalRead(LINE_FOLLOWING_SENSOR_RIGHT);

  CarState state = determineState(leftSensor, middleSensor, rightSensor);
  executeState(state);
}

void executeObstacleAvoidance() {
  long distanceToObstacle = calculateUltrasonicDistance();

  if (distanceToObstacle > 30 || distanceToObstacle == 0) {
    moveForwardWithSpeed(200);
  } else {
    stopCar();
    delay(300);
    
    long distanceLeft = turnLeftAndMeasureDistance();
    long distanceRight = turnRightAndMeasureDistance();

    if (distanceLeft < 15 && distanceRight < 15) {
      moveBackwardWithSpeed(200);
      delay(300);
      stopCar();
    } else {
      if (distanceRight >= distanceLeft) {
        delay(300);
        stopCar();
        delay(300);
        turnLeftWithSpeed(200);
        delay(350);
        stopCar();
      } else {
        delay(300);
        stopCar();
        delay(300);
        turnRightWithSpeed(200);
        delay(350);
        stopCar();
      }
    }
  }
}

CarState determineState(int leftSensor, int middleSensor, int rightSensor) {
  if (middleSensor == LOW)
    return FORWARD;
  if (rightSensor == LOW && leftSensor != LOW)
    return TURN_RIGHT;
  if (leftSensor == LOW && rightSensor != LOW)
    return TURN_LEFT;
  return BACKWARD;
}

void executeState(CarState state) {
  switch (state) {
    case FORWARD:
      moveForwardWithSpeed(100);
      break;
    case TURN_RIGHT:
      turnRightWithSpeed(100);
      break;
    case TURN_LEFT:
      turnLeftWithSpeed(100);
      break;
    case BACKWARD:
      moveBackwardWithSpeed(100);
      break;
    case STOP:
      stopCar();
      break;
  }
}

long calculateUltrasonicDistance() {
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);

  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 20000);
  return (duration == 0) ? 999 : duration * 0.034 / 2;
}

long turnLeftAndMeasureDistance() {
  servo.write(0);
  delay(500);
  long distance = calculateUltrasonicDistance();
  setServoToCenter();
  return distance;
}

long turnRightAndMeasureDistance() {
  servo.write(180);
  delay(500);
  long distance = calculateUltrasonicDistance();
  setServoToCenter();
  return distance;
}

void setServoToCenter() {
  servo.write(90);
}

void moveForwardWithSpeed(int speed) {
  digitalWrite(RIGHT_MOTOR_IN1, HIGH);
  digitalWrite(RIGHT_MOTOR_IN2, LOW);
  analogWrite(RIGHT_MOTOR_ENA, speed);

  digitalWrite(LEFT_MOTOR_IN3, LOW);
  digitalWrite(LEFT_MOTOR_IN4, HIGH);
  analogWrite(LEFT_MOTOR_ENB, speed);
}

void moveBackwardWithSpeed(int speed) {
  digitalWrite(RIGHT_MOTOR_IN1, LOW);
  digitalWrite(RIGHT_MOTOR_IN2, HIGH);
  analogWrite(RIGHT_MOTOR_ENA, speed);

  digitalWrite(LEFT_MOTOR_IN3, HIGH);
  digitalWrite(LEFT_MOTOR_IN4, LOW);
  analogWrite(LEFT_MOTOR_ENB, speed);
}

void turnLeftWithSpeed(int speed) {
  digitalWrite(RIGHT_MOTOR_IN1, HIGH);
  digitalWrite(RIGHT_MOTOR_IN2, LOW);
  analogWrite(RIGHT_MOTOR_ENA, speed);

  digitalWrite(LEFT_MOTOR_IN3, HIGH);
  digitalWrite(LEFT_MOTOR_IN4, LOW);
  analogWrite(LEFT_MOTOR_ENB, speed);
}

void turnRightWithSpeed(int speed) {
  digitalWrite(RIGHT_MOTOR_IN1, LOW);
  digitalWrite(RIGHT_MOTOR_IN2, HIGH);
  analogWrite(RIGHT_MOTOR_ENA, speed);

  digitalWrite(LEFT_MOTOR_IN3, LOW);
  digitalWrite(LEFT_MOTOR_IN4, HIGH);
  analogWrite(LEFT_MOTOR_ENB, speed);
}

void stopCar() {
  digitalWrite(RIGHT_MOTOR_IN1, LOW);
  digitalWrite(RIGHT_MOTOR_IN2, LOW);
  analogWrite(RIGHT_MOTOR_ENA, 0);

  digitalWrite(LEFT_MOTOR_IN3, LOW);
  digitalWrite(LEFT_MOTOR_IN4, LOW);
  analogWrite(LEFT_MOTOR_ENB, 0);
}