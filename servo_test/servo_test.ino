#include <ESP32Servo.h>

Servo myServo;

void setup() {
  myServo.attach(26);  // 接到 PWM 引腳 D9
}

void loop() {
  myServo.write(175);    // 轉到 0 度
  delay(10000);         // 停 1 秒
  myServo.write(135);   // 轉到 90 度
  delay(5000);         // 停 1 秒
  // myServo.write(90);  // 轉到 180 度
  // delay(1000);         // 停 1 秒
}
