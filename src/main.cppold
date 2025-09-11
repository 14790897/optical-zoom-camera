#include <Arduino.h>

// A4988 步进电机驱动引脚定义
#define DIR_PIN 2     // 方向控制引脚
#define STEP_PIN 3    // 步进脉冲引脚
#define EN_PIN 12      // 使能引脚（低电平有效）

// 步进电机参数
#define STEPS_PER_REV 2000   // 每圈步数（1.8度步距角电机）
#define STEP_DELAY 1000     // 步进间隔（微秒）- 控制转速

void stepMotor(int steps, bool clockwise) {
  // 设置方向
  digitalWrite(DIR_PIN, clockwise ? HIGH : LOW);
  
  // 执行步进
  for (int i = 0; i < abs(steps); i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(STEP_DELAY);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(STEP_DELAY);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("A4988 步进电机控制初始化...");
  
  // 初始化引脚
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  
  // 使能步进电机驱动（低电平有效）
  digitalWrite(EN_PIN, LOW);
  
  // 初始状态
  digitalWrite(DIR_PIN, LOW);
  digitalWrite(STEP_PIN, LOW);
  
  Serial.println("初始化完成！");
}

void loop() {
  Serial.println("顺时针转动10秒...");
  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {  // 转动10秒
    stepMotor(1, true);  // 每次转1步，顺时针
  }
  
  Serial.println("逆时针转动10秒...");
  startTime = millis();
  while (millis() - startTime < 10000) {  // 转动10秒
    stepMotor(1, false);  // 每次转1步，逆时针
  }
  
  Serial.println("一个循环完成，暂停2秒...");
  delay(2000);
}