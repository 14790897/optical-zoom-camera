#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "secrets.h" // 包含WiFi和Web服务器设置

// 创建Web服务器对象
AsyncWebServer server(80);

// A4988 步进电机驱动引脚定义
// 第一个步进电机
#define DIR_PIN_1 2     // 电机1方向控制引脚
#define STEP_PIN_1 3    // 电机1步进脉冲引脚

// 第二个步进电机
#define DIR_PIN_2 10    // 电机2方向控制引脚
#define STEP_PIN_2 6    // 电机2步进脉冲引脚

// 共用使能引脚（或可以分别控制）
#define EN_PIN_1 12        // 使能引脚（低电平有效）
#define EN_PIN_2 18        // 使能引脚（低电平有效）

// 步进电机参数
#define STEPS_PER_REV 2000   // 每圈步数（1.8度步距角电机）
#define STEP_DELAY 1000     // 步进间隔（微秒）- 控制转速

// 全局变量
volatile bool motor1_running = false;
volatile bool motor2_running = false;
volatile bool motor1_direction = true;
volatile bool motor2_direction = true;
volatile unsigned long motor1_stop_time = 0;
volatile unsigned long motor2_stop_time = 0;
volatile int current_step_delay = STEP_DELAY;

// 日志辅助
static inline const char* dirToStr(bool cw) { return cw ? "顺时针" : "逆时针"; }
static void logMotorState() {
  unsigned long now = millis();
  unsigned long m1_left = (motor1_running && motor1_stop_time > now) ? (motor1_stop_time - now) / 1000UL : 0;
  unsigned long m2_left = (motor2_running && motor2_stop_time > now) ? (motor2_stop_time - now) / 1000UL : 0;
  Serial.printf("[状态] delay=%dus | M1:%s%s 剩余%lus | M2:%s%s 剩余%lus\n",
                current_step_delay,
                motor1_running ? "运行" : "停止",
                motor1_running ? (motor1_direction ? "(顺)" : "(逆)") : "",
                m1_left,
                motor2_running ? "运行" : "停止",
                motor2_running ? (motor2_direction ? "(顺)" : "(逆)") : "",
                m2_left);
}

void stepMotor(int motorNum, int steps, bool clockwise) {
  int dirPin, stepPin;
  
  // 选择对应的电机引脚
  if (motorNum == 1) {
    dirPin = DIR_PIN_1;
    stepPin = STEP_PIN_1;
  } else if (motorNum == 2) {
    dirPin = DIR_PIN_2;
    stepPin = STEP_PIN_2;
  } else {
    return; // 无效的电机编号
  }
  
  // 设置方向
  digitalWrite(dirPin, clockwise ? HIGH : LOW);
  
  // 执行步进
  for (int i = 0; i < abs(steps); i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(current_step_delay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(current_step_delay);
  }
}

// 启动电机持续转动
void startMotor(int motorNum, bool clockwise, int duration_seconds) {
  Serial.printf("[电机%d] start %s, %ds\n", motorNum, dirToStr(clockwise), duration_seconds);
  if (motorNum == 1) {
    motor1_running = true;
    motor1_direction = clockwise;
    motor1_stop_time = millis() + (duration_seconds * 1000);
    digitalWrite(EN_PIN_1, LOW);
  } else if (motorNum == 2) {
    motor2_running = true;
    motor2_direction = clockwise;
    motor2_stop_time = millis() + (duration_seconds * 1000);
    digitalWrite(EN_PIN_2, LOW);
  }
}

// 停止电机
void stopMotor(int motorNum) {
  Serial.printf("[电机%d] stop\n", motorNum);
  if (motorNum == 1) {
    motor1_running = false;
  } else if (motorNum == 2) {
    motor2_running = false;
  }
}

// 更新电机状态（在loop中调用）
void updateMotors() {
  unsigned long currentTime = millis();
  
  // 检查电机1
  if (motor1_running) {
    if (currentTime >= motor1_stop_time) {
      motor1_running = false;
      Serial.println("[电机1] 到时自动停止");
    } else {
      stepMotor(1, 1, motor1_direction);
    }
  }
  
  // 检查电机2
  if (motor2_running) {
    if (currentTime >= motor2_stop_time) {
      motor2_running = false;
      Serial.println("[电机2] 到时自动停止");
    } else {
      stepMotor(2, 1, motor2_direction);
    }
  }
}

// 初始化WiFi
void initWiFi() {
  Serial.printf("连接WiFi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("连接WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("WiFi已连接! IP地址: ");
  Serial.println(WiFi.localIP());
}

// 初始化mDNS
void initMDNS() {
  if (!MDNS.begin(MDNS_HOST_NAME)) {
    Serial.println("mDNS启动失败!");
    return;
  }
  
  // 添加服务
  MDNS.addService("http", "tcp", 80);
  MDNS.addServiceTxt("http", "tcp", "version", "1.0");
  MDNS.addServiceTxt("http", "tcp", "device", "camera-motor-controller");
  
  Serial.println("mDNS已启动!");
  Serial.print("可通过以下地址访问: http://");
  Serial.print(MDNS_HOST_NAME);
  Serial.println(".local");
}

// 初始化OTA
void initOTA() {
  // 设置OTA主机名
  ArduinoOTA.setHostname(MDNS_HOST_NAME);
  
  // 设置OTA密码
  ArduinoOTA.setPassword(OTA_PASSWORD);
  
  // OTA开始回调
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("开始OTA更新: " + type);
    
    // 停止所有电机
    motor1_running = false;
    motor2_running = false;
    digitalWrite(EN_PIN_1, HIGH); // 禁用电机驱动
    digitalWrite(EN_PIN_2, HIGH); // 禁用电机驱动
  });
  
  // OTA结束回调
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA更新完成!");
  });
  
  // OTA进度回调
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("进度: %u%%\r", (progress / (total / 100)));
  });
  
  // OTA错误回调
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA错误[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("认证失败");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("开始失败");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("连接失败");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("接收失败");
    } else if (error == OTA_END_ERROR) {
      Serial.println("结束失败");
    }
    
    // 重新启用电机驱动
    digitalWrite(EN_PIN_1, LOW);
    digitalWrite(EN_PIN_2, LOW);
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA已启动!");
  Serial.println("现在可以通过WiFi无线更新固件");
}

// 初始化LittleFS
void initLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS挂载失败");
    return;
  }
  Serial.println("LittleFS挂载成功");
}

// 初始化Web服务器
void initWebServer() {
  // 允许跨域，便于开发环境通过 mDNS 或不同源访问
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
  // 提供静态文件
  // server.serveStatic("/", LittleFS, "/");

  // 电机1控制
  server.on("/motor1/cw/*", HTTP_GET, [](AsyncWebServerRequest *request){
    String duration = request->pathArg(0);
    int durationInt = duration.toInt();
    // 如果没有提供有效的持续时间，使用默认值5秒
    if (durationInt <= 0)
    {
      durationInt = 5;
      duration = "5";
    }
    Serial.printf("[HTTP] /motor1/cw/%s (实际: %d秒)\n", duration.c_str(), durationInt);
    startMotor(1, true, durationInt);
    request->send(200, "text/plain", "电机1顺时针转动" + String(durationInt) + "秒");
  });
  
  server.on("/motor1/ccw/*", HTTP_GET, [](AsyncWebServerRequest *request){
    String duration = request->pathArg(0);
    int durationInt = duration.toInt();
    // 如果没有提供有效的持续时间，使用默认值5秒
    if (durationInt <= 0)
    {
      durationInt = 5;
      duration = "5";
    }
    Serial.printf("[HTTP] /motor1/ccw/%s (实际: %d秒)\n", duration.c_str(), durationInt);
    startMotor(1, false, durationInt);
    request->send(200, "text/plain", "电机1逆时针转动" + String(durationInt) + "秒");
  });
  
  server.on("/motor1/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("[HTTP] /motor1/stop");
    stopMotor(1);
    request->send(200, "text/plain", "电机1已停止");
  });
  
  // 电机2控制
  server.on("/motor2/cw/*", HTTP_GET, [](AsyncWebServerRequest *request){
    String duration = request->pathArg(0);
    int durationInt = duration.toInt();
    // 如果没有提供有效的持续时间，使用默认值5秒
    if (durationInt <= 0)
    {
      durationInt = 5;
      duration = "5";
    }
    Serial.printf("[HTTP] /motor2/cw/%s (实际: %d秒)\n", duration.c_str(), durationInt);
    startMotor(2, true, durationInt);
    request->send(200, "text/plain", "电机2顺时针转动" + String(durationInt) + "秒");
  });
  
  server.on("/motor2/ccw/*", HTTP_GET, [](AsyncWebServerRequest *request){
    String duration = request->pathArg(0);
    int durationInt = duration.toInt();
    // 如果没有提供有效的持续时间，使用默认值5秒
    if (durationInt <= 0)
    {
      durationInt = 5;
      duration = "5";
    }
    Serial.printf("[HTTP] /motor2/ccw/%s (实际: %d秒)\n", duration.c_str(), durationInt);
    startMotor(2, false, durationInt);
    request->send(200, "text/plain", "电机2逆时针转动" + String(durationInt) + "秒");
  });
  
  server.on("/motor2/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("[HTTP] /motor2/stop");
    stopMotor(2);
    request->send(200, "text/plain", "电机2已停止");
  });
  
  // 双电机控制
  server.on("/both/*/*/*", HTTP_GET, [](AsyncWebServerRequest *request){
    String dir1 = request->pathArg(0);
    String dir2 = request->pathArg(1);
    String duration = request->pathArg(2);
    int durationInt = duration.toInt();
    // 如果没有提供有效的持续时间，使用默认值5秒
    if (durationInt <= 0)
    {
      durationInt = 5;
      duration = "5";
    }
    Serial.printf("[HTTP] /both/%s/%s/%s (实际: %d秒)\n", dir1.c_str(), dir2.c_str(), duration.c_str(), durationInt);

    bool motor1_cw = (dir1 == "cw");
    bool motor2_cw = (dir2 == "cw");

    startMotor(1, motor1_cw, durationInt);
    startMotor(2, motor2_cw, durationInt);

    request->send(200, "text/plain", "双电机已启动运行" + String(durationInt) + "秒");
  });
  
  // 停止所有电机
  server.on("/stop/all", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("[HTTP] /stop/all");
    stopMotor(1);
    stopMotor(2);
    request->send(200, "text/plain", "所有电机已停止");
  });
  
  // 设置速度
  server.on("/set/speed/*", HTTP_GET, [](AsyncWebServerRequest *request){
    String speed = request->pathArg(0);
    current_step_delay = speed.toInt();
    Serial.printf("[HTTP] /set/speed/%d\n", current_step_delay);
    request->send(200, "text/plain", "速度已设置为: " + speed);
  });
  
  // 状态查询
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"motor1\":\"" + String(motor1_running ? (motor1_direction ? "顺时针" : "逆时针") : "停止") + "\",";
    json += "\"motor2\":\"" + String(motor2_running ? (motor2_direction ? "顺时针" : "逆时针") : "停止") + "\"";
    json += "}";
    request->send(200, "application/json", json);
  });
  
  // 重启系统
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "系统将在3秒后重启...");
    delay(3000);
    ESP.restart();
  });
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html; charset=utf-8"); });

  // 兼容查询参数的简易接口，避免某些环境下 pathArg(*) 匹配异常
  server.on("/motor1/cw", HTTP_GET, [](AsyncWebServerRequest *request){
    int durationInt = 5;
    if (request->hasParam("t")) {
      durationInt = request->getParam("t")->value().toInt();
      if (durationInt <= 0) durationInt = 5;
    }
    Serial.printf("[HTTP] /motor1/cw?t=%d\n", durationInt);
    startMotor(1, true, durationInt);
    request->send(200, "text/plain; charset=utf-8", "电机1顺时针转动" + String(durationInt) + "秒");
  });

  server.on("/motor1/ccw", HTTP_GET, [](AsyncWebServerRequest *request){
    int durationInt = 5;
    if (request->hasParam("t")) {
      durationInt = request->getParam("t")->value().toInt();
      if (durationInt <= 0) durationInt = 5;
    }
    Serial.printf("[HTTP] /motor1/ccw?t=%d\n", durationInt);
    startMotor(1, false, durationInt);
    request->send(200, "text/plain; charset=utf-8", "电机1逆时针转动" + String(durationInt) + "秒");
  });

  server.on("/motor2/cw", HTTP_GET, [](AsyncWebServerRequest *request){
    int durationInt = 5;
    if (request->hasParam("t")) {
      durationInt = request->getParam("t")->value().toInt();
      if (durationInt <= 0) durationInt = 5;
    }
    Serial.printf("[HTTP] /motor2/cw?t=%d\n", durationInt);
    startMotor(2, true, durationInt);
    request->send(200, "text/plain; charset=utf-8", "电机2顺时针转动" + String(durationInt) + "秒");
  });

  server.on("/motor2/ccw", HTTP_GET, [](AsyncWebServerRequest *request){
    int durationInt = 5;
    if (request->hasParam("t")) {
      durationInt = request->getParam("t")->value().toInt();
      if (durationInt <= 0) durationInt = 5;
    }
    Serial.printf("[HTTP] /motor2/ccw?t=%d\n", durationInt);
    startMotor(2, false, durationInt);
    request->send(200, "text/plain; charset=utf-8", "电机2逆时针转动" + String(durationInt) + "秒");
  });

  server.on("/both", HTTP_GET, [](AsyncWebServerRequest *request){
    String d1 = request->hasParam("d1") ? request->getParam("d1")->value() : String("cw");
    String d2 = request->hasParam("d2") ? request->getParam("d2")->value() : String("cw");
    int durationInt = request->hasParam("t") ? request->getParam("t")->value().toInt() : 5;
    if (durationInt <= 0) durationInt = 5;
    bool motor1_cw = (d1 == "cw");
    bool motor2_cw = (d2 == "cw");
    Serial.printf("[HTTP] /both?d1=%s&d2=%s&t=%d\n", d1.c_str(), d2.c_str(), durationInt);
    startMotor(1, motor1_cw, durationInt);
    startMotor(2, motor2_cw, durationInt);
    request->send(200, "text/plain; charset=utf-8", "双电机已启动运行" + String(durationInt) + "秒");
  });
  server.begin();
  Serial.println("Web服务器已启动");
}

void setup() {
  Serial.begin(115200);
  Serial.println("相机步进电机Web控制系统初始化...");
  
  // 初始化第一个电机引脚
  pinMode(DIR_PIN_1, OUTPUT);
  pinMode(STEP_PIN_1, OUTPUT);
  
  // 初始化第二个电机引脚
  pinMode(DIR_PIN_2, OUTPUT);
  pinMode(STEP_PIN_2, OUTPUT);
  
  // 初始化使能引脚
  pinMode(EN_PIN_1, OUTPUT);
  pinMode(EN_PIN_2, OUTPUT);

  // 使能步进电机驱动（低电平有效）
  digitalWrite(EN_PIN_1, LOW);
  digitalWrite(EN_PIN_2, LOW);

  // 初始状态
  digitalWrite(DIR_PIN_1, LOW);
  digitalWrite(STEP_PIN_1, LOW);
  digitalWrite(DIR_PIN_2, LOW);
  digitalWrite(STEP_PIN_2, LOW);
  
  Serial.println("电机初始化完成！");
  
  // 初始化文件系统和网络
  initLittleFS();
  initWiFi();
  initMDNS();
  // initOTA();
  initWebServer();
  
  Serial.println("==================================");
  Serial.println("相机步进电机Web控制系统已启动！");
  Serial.print("IP地址访问: http://");
  Serial.println(WiFi.localIP());
  Serial.print("域名访问: http://");
  Serial.print(MDNS_HOST_NAME);
  Serial.println(".local");
  Serial.println("OTA更新: 主机名=" + String(MDNS_HOST_NAME) + ".local");
  Serial.println("==================================");
}

void loop() {
  // 处理OTA更新
  ArduinoOTA.handle();
  
  // 更新电机状态和控制
  updateMotors();
  
  // 短暂延迟，避免过度占用CPU
  static unsigned long lastBeat = 0;
  unsigned long now = millis();
  if (now - lastBeat >= 5000) { // 每500ms 打印一次状态心跳
    logMotorState();
    lastBeat = now;
  }
  delay(1);
}
