#include <Servo.h>
#include <SoftwareSerial.h> // Thư viện giao tiếp Bluetooth

// Định nghĩa chân Bluetooth
const int BT_RX = 2; // RX của Arduino nối với TX của module Bluetooth
const int BT_TX = 3; // TX của Arduino nối với RX của module Bluetooth
SoftwareSerial bluetooth(BT_RX, BT_TX); // Tạo đối tượng SoftwareSerial

// Định nghĩa chân động cơ
const int RIGHT_MOTOR_IN1 = 11;
const int RIGHT_MOTOR_IN2 = 9;
const int RIGHT_MOTOR_ENA = 6;
const int LEFT_MOTOR_IN3 = 8;
const int LEFT_MOTOR_IN4 = 7;
const int LEFT_MOTOR_ENB = 5;

// Định nghĩa chân cảm biến siêu âm
const int ULTRASONIC_TRIGGER_PIN = 19;
const int ULTRASONIC_ECHO_PIN = 18;

// Định nghĩa chân cảm biến dò line
const int LINE_FOLLOWING_SENSOR_LEFT = A0;
const int LINE_FOLLOWING_SENSOR_MIDDLE = A1;
const int LINE_FOLLOWING_SENSOR_RIGHT = A2;

// Biến trạng thái
bool isLineFollowingEnabled = false;
bool isObstacleAvoidanceEnabled = true;
Servo servo;

// Thiết lập ban đầu
void setup() {
  // Cài đặt chế độ chân
  pinMode(RIGHT_MOTOR_IN1, OUTPUT);
  pinMode(RIGHT_MOTOR_IN2, OUTPUT);
  pinMode(RIGHT_MOTOR_ENA, OUTPUT);
  pinMode(LEFT_MOTOR_IN3, OUTPUT);
  pinMode(LEFT_MOTOR_IN4, OUTPUT);
  pinMode(LEFT_MOTOR_ENB, OUTPUT);
  pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(LINE_FOLLOWING_SENSOR_LEFT, INPUT);
  pinMode(LINE_FOLLOWING_SENSOR_MIDDLE, INPUT);
  pinMode(LINE_FOLLOWING_SENSOR_RIGHT, INPUT);

  // Khởi động giao tiếp Serial và Bluetooth
  Serial.begin(9600);
  bluetooth.begin(9600);

  // Gắn servo và đặt nó về vị trí trung tâm
  servo.attach(3);
  setServoToCenter();

  Serial.println("System initialized.");
}

// Vòng lặp chính
void loop() {
  long distanceToObstacle = calculateUltrasonicDistance();
  int leftLineFollowerSensorValue = digitalRead(LINE_FOLLOWING_SENSOR_LEFT);
  int middleLineFollowerSensorValue = digitalRead(LINE_FOLLOWING_SENSOR_MIDDLE);
  int rightLineFollowerSensorValue = digitalRead(LINE_FOLLOWING_SENSOR_RIGHT);

  // Xử lý chế độ dò line
  if (isLineFollowingEnabled) {
    followLine(leftLineFollowerSensorValue, middleLineFollowerSensorValue, rightLineFollowerSensorValue);
  }
  // Xử lý chế độ né vật cản
  else if (isObstacleAvoidanceEnabled && distanceToObstacle > 0 && distanceToObstacle < 20) {
    avoidObstacle();
  } else {
    setServoToCenter();
    moveForwardWithSpeed(255);
    Serial.println("Moving forward.");
  }

  // Xử lý lệnh Bluetooth
  if (bluetooth.available()) {
    String command = bluetooth.readStringUntil('\n'); // Đọc dữ liệu từ Bluetooth
    command.trim();                                   // Loại bỏ khoảng trắng hoặc ký tự xuống dòng
    handleCommand(command);                          // Xử lý lệnh nhận được
  }

  delay(100);
}

// Hàm xử lý lệnh từ Bluetooth
void handleCommand(String command) {
  Serial.print("Command received: ");
  Serial.println(command);

  if (command == "UP") {
    moveForwardWithSpeed(255);
  } else if (command == "DOWN") {
    moveBackwardWithSpeed(255);
  } else if (command == "LEFT") {
    turnLeftWithSpeed(200);
  } else if (command == "RIGHT") {
    turnRightWithSpeed(200);
  } else if (command.startsWith("SPEED:")) {
    int speed = command.substring(6).toInt(); // Lấy tốc độ từ lệnh
    moveForwardWithSpeed(constrain(speed, 0, 255));
  } else if (command == "DOLINE:ON") {
    isLineFollowingEnabled = true;
    Serial.println("Line following mode enabled.");
  } else if (command == "DOLINE:OFF") {
    isLineFollowingEnabled = false;
    Serial.println("Line following mode disabled.");
  } else if (command == "NEVATCAN:ON") {
    isObstacleAvoidanceEnabled = true;
    Serial.println("Obstacle avoidance mode enabled.");
  } else if (command == "NEVATCAN:OFF") {
    isObstacleAvoidanceEnabled = false;
    Serial.println("Obstacle avoidance mode disabled.");
  } else {
    Serial.println("Unknown command.");
    stopCar();
  }
}

// Hàm di chuyển tiến với tốc độ
void moveForwardWithSpeed(int speed) {
  digitalWrite(RIGHT_MOTOR_IN1, HIGH);
  digitalWrite(RIGHT_MOTOR_IN2, LOW);
  analogWrite(RIGHT_MOTOR_ENA, speed);

  digitalWrite(LEFT_MOTOR_IN3, HIGH);
  digitalWrite(LEFT_MOTOR_IN4, LOW);
  analogWrite(LEFT_MOTOR_ENB, speed);
}

// Hàm đo khoảng cách bằng cảm biến siêu âm
long calculateUltrasonicDistance() {
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);

  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return 400; // Không có vật cản
  }

  long distance = duration * 0.034 / 2; // Tính khoảng cách
  return distance;
}

// Hàm dò line
void followLine(int left, int middle, int right) {
  if (middle == 1) {
    moveForwardWithSpeed(150); // Đi thẳng
  } else if (left == 1) {
    turnLeftWithSpeed(150); // Quẹo trái
  } else if (right == 1) {
    turnRightWithSpeed(150); // Quẹo phải
  } else {
    stopCar();
  }
}

// Hàm tránh vật cản
void avoidObstacle() {
  stopCar();
  long distanceFront = checkDistanceAtAngle(90);
  long distanceLeft = checkDistanceAtAngle(45);
  long distanceRight = checkDistanceAtAngle(135);

  if (distanceFront > 20) {
    moveForwardWithSpeed(200);
  } else if (distanceLeft > distanceRight && distanceLeft > 15) {
    turnLeftWithSpeed(200);
  } else if (distanceRight > distanceLeft && distanceRight > 15) {
    turnRightWithSpeed(200);
  } else {
    moveBackwardWithSpeed(200);
    turnLeftWithSpeed(200);
  }
  stopCar();
}

// Các hàm hỗ trợ khác
void setServoToCenter() {
  servo.write(90);
}
long checkDistanceAtAngle(int angle) {
  servo.write(angle);
  delay(500);
  return calculateUltrasonicDistance();
}
void stopCar() {
  analogWrite(RIGHT_MOTOR_ENA, 0);
  analogWrite(LEFT_MOTOR_ENB, 0);
}
