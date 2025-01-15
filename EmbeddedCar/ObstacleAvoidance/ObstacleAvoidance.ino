#include <Servo.h>

const int ULTRASONIC_TRIGGER_PIN = 19;
const int ULTRASONIC_ECHO_PIN = 18;

const int RIGHT_MOTOR_IN1 = 11;
const int RIGHT_MOTOR_IN2 = 9;
const int RIGHT_MOTOR_ENA = 6;

const int LEFT_MOTOR_IN3 = 8;
const int LEFT_MOTOR_IN4 = 7;
const int LEFT_MOTOR_ENB = 5;

Servo servo;

void setup() {
  pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(RIGHT_MOTOR_IN1, OUTPUT);
  pinMode(RIGHT_MOTOR_IN2, OUTPUT);
  pinMode(RIGHT_MOTOR_ENA, OUTPUT);
  pinMode(LEFT_MOTOR_IN3, OUTPUT);
  pinMode(LEFT_MOTOR_IN4, OUTPUT);
  pinMode(LEFT_MOTOR_ENB, OUTPUT);

  Serial.begin(9600);
  servo.attach(3);
  setServoToCenter();
}

void loop() {
  long distanceToObstacle = calculateUltrasonicDistance();
  Serial.print("Distance to Obstacle: ");
  Serial.println(distanceToObstacle);

  if (distanceToObstacle > 30 || distanceToObstacle == 0) {
    moveForwardWithSpeed(200);
    Serial.println("Moving forward");
  } else {
    stopCar(); 
    delay(300);
    long distanceLeft = turnLeftAndMeasureDistance();
    long distanceRight = turnRightAndMeasureDistance();

    if (distanceLeft < 15 && distanceRight < 15) {
      moveBackwardWithSpeed(200);
      delay(300);
      stopCar();
      Serial.println("Moving backward");
    } else {
      if (distanceRight >= distanceLeft) {
        // moveBackwardWithSpeed(200);
        delay(300);
        stopCar();
        delay(300);
        turnLeftWithSpeed(200);
        delay(350);
        stopCar();
        delay(300);
        Serial.println("Turning right");
      } else if (distanceRight < distanceLeft) {
        // moveBackwardWithSpeed(200);
        delay(300);
        stopCar();
        delay(300);
        turnRightWithSpeed(200);
        delay(350);
        stopCar();
        delay(300);
        Serial.println("Turning left");
      }
    }
  }

  delay(100);
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
