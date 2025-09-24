# 📷 光学变焦相机控制系统 / Optical Zoom Camera Control System

基于 ESP32 的双步进电机控制系统，专为相机光学变焦和对焦功能设计。支持 Web 界面远程控制、OTA 无线更新和实时状态监控。

> 🌐 **English Version**: [README_EN.md](README_EN.md)

## ✨ 功能特性

- 🎯 **双步进电机控制** - 独立控制两个 A4988 步进电机驱动器
- 🌐 **Web 远程控制** - 响应式 Web 界面，支持手机和电脑访问
- 📡 **WiFi 连接** - 通过 WiFi 网络进行无线控制
- 🔄 **OTA 更新** - 支持无线固件更新，无需连接数据线
- ⚡ **实时控制** - 可调节转速、转向和持续时间
- 🖥️ **状态监控** - 实时显示电机运行状态和剩余时间
- 📱 **跨平台兼容** - 支持通过 mDNS 域名访问设备

## 🛠️ 硬件需求

### 主控制器
- **ESP32-C3** 开发板（推荐 AirM2M CORE ESP32C3）
- 内置 WiFi 和蓝牙功能

### 步进电机驱动
- **2 x A4988** 步进电机驱动器
- **2 x NEMA 17** 步进电机（1.8° 步距角）
- 12V/24V 电源适配器（根据电机规格）

### 接线说明

#### 电机1（变焦控制）
```
ESP32-C3    ->   A4988 #1
GPIO2       ->   DIR (方向)
GPIO3       ->   STEP (步进脉冲)
GPIO12      ->   EN (使能，低电平有效)
```

#### 电机2（对焦控制）
```
ESP32-C3    ->   A4988 #2
GPIO10      ->   DIR (方向)
GPIO6       ->   STEP (步进脉冲)
GPIO18      ->   EN (使能，低电平有效)
```

#### A4988 电源连接
```
VDD    -> 3.3V (逻辑电源)
VMOT   -> 12V/24V (电机电源)
GND    -> GND (公共地线)
```

## 🚀 快速开始

### 1. 环境准备

安装 [PlatformIO](https://platformio.org/) 开发环境：

```bash
# 使用 VSCode 插件（推荐）
# 或使用命令行安装
pip install platformio
```

### 2. 配置 WiFi

复制示例配置文件并修改 WiFi 信息：

```bash
cp src/secrets.h.example src/secrets.h
```

然后编辑 `src/secrets.h` 文件：

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi 配置
#define WIFI_SSID "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"

// mDNS 主机名
#define MDNS_HOST_NAME "camera-lens"

// OTA 更新密码
#define OTA_PASSWORD "admin123"

#endif
```

### 3. 编译和上传

```bash
# 编译项目
pio run

# 上传固件到设备
pio run -t upload

# 监控串口输出
pio device monitor
```

### 4. 访问 Web 界面

固件上传成功后，设备会自动连接 WiFi 并启动 Web 服务器：

- **IP 地址访问**：`http://设备IP地址`
- **域名访问**：`http://camera-lens.local`

在串口监视器中可以查看设备的 IP 地址。

## 🖥️ Web 界面使用

### 主要功能

1. **电机独立控制**
   - 顺时针/逆时针转动
   - 可调节持续时间（1-60秒）
   - 一键停止功能

2. **双电机联动**
   - 同时控制两个电机
   - 支持不同方向组合

3. **参数设置**
   - 调节步进速度（转速控制）
   - 实时生效

4. **系统管理**
   - 查看运行状态
   - OTA 更新信息
   - 系统重启

### API 接口

#### 电机控制
```
GET /motor1/cw?t=5    # 电机1顺时针转动5秒
GET /motor1/ccw?t=3   # 电机1逆时针转动3秒
GET /motor2/cw?t=10   # 电机2顺时针转动10秒
GET /motor2/ccw?t=8   # 电机2逆时针转动8秒
```

#### 停止控制
```
GET /motor1/stop      # 停止电机1
GET /motor2/stop      # 停止电机2
GET /stop/all         # 停止所有电机
```

#### 参数设置
```
GET /set/speed/800    # 设置步进间隔为800微秒
```

#### 状态查询
```
GET /status           # 获取电机状态JSON
```

## 🔧 开发指南

### 项目结构

```
optical-zoom-camera/
├── src/
│   └── main_web.cpp          # 主程序（Web版本）
├── data/
│   └── index.html            # Web界面
├── main.cpp                  # 基础版本（仅串口控制）
├── platformio.ini            # PlatformIO配置
├── airm2m.json              # AirM2M开发板定义
├── official.json            # ESP32-C3官方开发板定义
└── README.md                # 项目说明
```

### 编译选项

在 `platformio.ini` 中可以调整以下配置：

```ini
# 调试等级
build_flags = -DCORE_DEBUG_LEVEL=3

# 启用OTA功能
build_flags = -DENABLE_OTA=1

# 文件系统类型
board_build.filesystem = littlefs
```

### 自定义配置

可以在代码中修改以下参数：

```cpp
// 步进电机参数
#define STEPS_PER_REV 2000    // 每圈步数
#define STEP_DELAY 1000       // 步进间隔（微秒）

// 引脚定义
#define DIR_PIN_1 2           # 电机1方向引脚
#define STEP_PIN_1 3          # 电机1步进引脚
// ... 其他引脚配置
```

## 📡 OTA 无线更新

### 启用 OTA 更新

1. 在 `platformio.ini` 中取消注释 OTA 配置：

```ini
upload_protocol = espota
upload_port = camera-lens.local
upload_flags = --auth=admin123
```

2. 首次需要通过 USB 上传固件
3. 之后可以使用无线方式更新：

```bash
pio run -t upload
```

### 更新步骤

1. 确保设备在线且可访问
2. 运行 `pio run -t upload`
3. 输入 OTA 密码：`admin123`
4. 等待更新完成

## 🔍 故障排除

### 常见问题

1. **WiFi 连接失败**
   - 检查 `secrets.h` 中的 WiFi 配置
   - 确认 WiFi 信号强度和密码正确

2. **电机不转动**
   - 检查电源连接（12V/24V 供电）
   - 验证 A4988 驱动器接线
   - 确认使能引脚状态（低电平有效）

3. **Web 界面无法访问**
   - 检查设备 IP 地址（串口监视器输出）
   - 尝试使用 mDNS 域名：`camera-lens.local`
   - 确认防火墙设置

4. **OTA 更新失败**
   - 检查网络连接稳定性
   - 确认 OTA 密码正确
   - 重启设备后重试

### 调试信息

启用调试输出查看详细信息：

```bash
pio device monitor -b 115200
```

## 📋 技术规格

- **微控制器**：ESP32-C3 (160MHz, 4MB Flash)
- **步进电机**：双路 A4988 驱动器
- **电机参数**：NEMA 17, 1.8° 步距角, 2000步/圈
- **WiFi**：802.11 b/g/n 2.4GHz
- **电源需求**：5V USB + 12V/24V 电机电源
- **控制精度**：0.18° 最小步距角
- **Web服务器**：ESPAsyncWebServer 异步处理

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

1. Fork 本项目
2. 创建特性分支：`git checkout -b feature/new-feature`
3. 提交更改：`git commit -m 'Add some feature'`
4. 推送分支：`git push origin feature/new-feature`
5. 提交 Pull Request

## 📄 开源协议

本项目采用 MIT 协议 - 查看 [LICENSE](LICENSE) 文件了解详情

## 🆘 支持

- 📧 邮件：[项目邮箱]
- 🐛 问题反馈：[GitHub Issues](https://github.com/14790897/optical-zoom-camera/issues)
- 💬 讨论：[GitHub Discussions](https://github.com/14790897/optical-zoom-camera/discussions)

---

*为相机光学系统提供精确的步进电机控制解决方案* 📸