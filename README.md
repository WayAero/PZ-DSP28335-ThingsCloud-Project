# PZ-DSP28335 ThingsCloud Project

基于 PZ-DSP28335 开发板和 ESP32 的工业物联网课程设计。DSP 负责本地采集、显示和外设控制，ESP32 负责 Wi-Fi、ThingsCloud MQTT 和串口桥接。

## 功能概览

- DSP28335 每秒输出设备状态 JSON。
- ESP32 通过 `Serial2` 接收 DSP 状态并上报 ThingsCloud。
- ThingsCloud 属性下发可控制 DSP 外设。
- ADC 电压超过阈值自动报警。
- LCD9648 显示 ADC 电压。
- 支持 LED、蜂鸣器、继电器、电机、按键控制。

## 硬件连接

DSP 与 ESP32 串口连接：

```text
DSP GPIO35 / SCITXDA -> ESP32 GPIO16 / RX2
DSP GPIO36 / SCIRXDA <- ESP32 GPIO17 / TX2
DSP GND              <-> ESP32 GND
```

电机驱动使用 TC1508S 第二通道：

```text
DSP GPIO4 / ePWM3A -> TC1508S INC
DSP GPIO5 / ePWM3B -> TC1508S IND
```

蜂鸣器：

```text
DSP GPIO6 / ePWM4A -> 蜂鸣器
```

## DSP 串口协议

串口参数：

```text
9600 8N1
```

DSP 状态上报示例：

```json
{"led":0,"beep":0,"relay":0,"motor_enable":0,"motor_dir":1,"motor_speed":0,"adc_mv":2200,"alarm":0}
```

DSP 控制命令示例：

```json
{"led":1}
{"relay":1}
{"beep":0}
{"motor_enable":1}
{"motor_speed":7}
{"motor_dir":2}
{"alarm_clear":1}
```

DSP 串口命令必须以换行符 `\n` 结尾。

## ThingsCloud 属性

上报字段：

```text
temperature
humidity
adc_voltage
led
beep
relay
motor_enable
motor_dir
motor_speed
alarm
```

属性下发示例：

```json
{"led":1}
```

```json
{"motor_enable":1,"motor_speed":5,"motor_dir":1}
```

```json
{"alarm_clear":1}
```

ThingsCloud 下发的 MQTT JSON 不需要手动追加 `\n`。ESP32 转发到 DSP 串口时会自动追加换行。

## 报警规则

```text
adc_mv >= 2500：报警
adc_mv <= 2400：自动解除报警
KEY9：暂停报警约 10 秒
alarm_clear=1：暂停报警约 10 秒
```

报警时 `alarm=1`，蜂鸣器响，LED2 亮。暂停 10 秒后，如果 ADC 仍高于阈值，会再次报警。

## 工程说明

主要目录：

```text
APP/adc        ADC 采集
APP/app_ctrl   应用状态和控制逻辑
APP/beep       蜂鸣器 ePWM4A 输出
APP/dc_motor   电机 ePWM3 控制
APP/key        按键扫描
APP/lcd9648    LCD9648 显示
APP/protocol   JSON 行协议
APP/time       Timer0 轮询 tick
APP/uart       SCI-A 串口
User/main.c    主循环
```

更多复现步骤见：

- `使用说明.md`
- `SESSION_SUMMARY.md`

## 隐私说明

本仓库不包含真实 Wi-Fi 密码或 ThingsCloud 设备密钥。

ESP32 工程中的 Wi-Fi 和 ThingsCloud 参数使用占位符。实际使用时需要在 ESP32 工程中填写自己的参数。
