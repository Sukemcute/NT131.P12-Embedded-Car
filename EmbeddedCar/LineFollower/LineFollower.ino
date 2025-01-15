#include <Servo.h>

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

enum CarState {
  FORWARD,
  TURN_RIGHT,
  TURN_LEFT,
  BACKWARD
};

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
  servo.attach(3);
  setServoToCenter();
}

void loop() {
  int leftSensor = digitalRead(LINE_FOLLOWING_SENSOR_LEFT);
  int middleSensor = digitalRead(LINE_FOLLOWING_SENSOR_MIDDLE);
  int rightSensor = digitalRead(LINE_FOLLOWING_SENSOR_RIGHT);

  Serial.print("Left Sensor: ");
  Serial.print(leftSensor);
  Serial.print(" | Middle Sensor: ");
  Serial.print(middleSensor);
  Serial.print(" | Right Sensor: ");
  Serial.println(rightSensor);

  stopCar();

  CarState state = determineState(leftSensor, middleSensor, rightSensor);
  executeState(state);

  delay(100);
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
  }
}

long calculateUltrasonicDistance() {
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);

  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 20000);

  if (duration == 0) {
    Serial.println("No obstacle detected (timeout).");
    return 999;
  }

  return duration * 0.034 / 2;
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
