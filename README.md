# PZ-DSP28335 ThingsCloud Project

基于 PZ-DSP28335 开发板和 ESP32 的工业物联网课程设计。DSP 负责本地采集、显示和外设控制，ESP32 负责 Wi-Fi、ThingsCloud MQTT 和串口桥接。

## 学习参考声明

本项目仅供课程设计、嵌入式学习和工程复现参考。

PZ-DSP28335 相关公开教程和例程年代较早，直接组合使用时会遇到工程配置、内存链接、外设引脚冲突、阻塞式外设调用等问题。本仓库保留了一份已联调通过的工程状态，并把调试过程中遇到的问题整理在 README 和文档中，方便后续学习者少走弯路。

本项目不是官方 SDK，也不是工业现场可直接部署的产品代码。实际使用前请重新检查硬件接线、电源电压、外设驱动能力和云端凭据安全。

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

LCD9648：

```text
GPIO0  -> SCL
GPIO60 -> SDA
GPIO1  -> RS
GPIO2  -> CS
GPIO3  -> RSET
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

## 实践问题总结

### 1. 老例程直接拼接后容易链接失败

早期例程通常功能单一，代码量小。把 ADC、LCD、串口、PWM、电机、协议解析等功能合到一个工程后，`.text` 可能放不进原 RAM 链接配置。

本工程的处理方式：

- 在 `DSP2833x_Libraries/28335_RAM_lnk.cmd` 中合并连续 RAM 区域。
- 将 `.text` 和 `IQmath` 放入合并后的 `RAML1L3`。
- 当前公开工程保留了这个可链接配置。

### 2. 只输出 `{"boot":1}` 多数不是串口坏了

调试时遇到过 DSP 启动后只输出 `{"boot":1}`，后续状态 JSON 不再输出。

本工程对应修复：

- Timer0 改为轮询 `TIF` 标志累加 tick，不依赖中断链路。
- ADC 读取加入非阻塞和超时处理。
- 状态 JSON 手动拼接，减少 `sprintf` 带来的代码体积和运行风险。

### 3. LCD9648 不能照搬整口 GPIO 初始化

LCD9648 是本项目里最容易引发连锁问题的部分。

一些旧 LCD 例程会直接写整组 GPIO 寄存器，例如一次性配置 GPIO0-GPIO15。这样在单独跑 LCD 例程时没问题，但合入综合工程后会破坏其他外设引脚复用。

本项目里电机原本使用 ePWM2，后续为了规避 LCD9648 与电机控制的引脚冲突，电机改用 ePWM3：

```text
GPIO4 / ePWM3A -> TC1508S INC
GPIO5 / ePWM3B -> TC1508S IND
```

LCD9648 驱动也改成只配置自己使用的引脚：

```text
GPIO0, GPIO1, GPIO2, GPIO3, GPIO60
```

这样 LCD9648 刷新 ADC 电压时，不会破坏 GPIO4/GPIO5 的 ePWM3 输出。

### 4. LCD9648 刷新会影响主循环节奏

LCD9648 使用软件模拟时序，刷新时会占用主循环时间。早期蜂鸣器如果靠主循环翻转 GPIO 发声，LCD 刷新会让蜂鸣器声音出现断点。

本工程的处理方式：

- 蜂鸣器改为 `GPIO6 / ePWM4A` 输出。
- 主循环只负责开关蜂鸣器。
- 方波由 ePWM 硬件产生，不再依赖主循环高频翻转。

### 5. DSP 串口是 JSON 行协议

DSP 接收命令时以 `\n` 作为一条 JSON 的结束标志。

直接用串口助手测试时，必须开启“发送新行”或手动追加 `0A`。否则 DSP 会缓存命令，但不会解析执行。

ThingsCloud 下发的 MQTT JSON 不需要手动加换行。ESP32 转发给 DSP 串口时会自动追加 `\n`。

### 6. ESP32 不要用 RX0/TX0 接 DSP

ESP32 程序使用 `Serial2` 和 DSP 通信：

```text
RX2 = GPIO16
TX2 = GPIO17
```

RX0/TX0 留给 USB 串口日志。DSP 接到 RX0/TX0 时，程序不会从那里读取 DSP 状态。

### 7. 报警清除需要暂停窗口

ADC 超过 2500mV 时会持续触发报警。如果只是把 `alarm` 清零，下一次 ADC 采样又会立刻报警。

本工程中：

```text
KEY9：暂停报警约 10 秒
alarm_clear=1：暂停报警约 10 秒
```

10 秒后如果 ADC 仍高于阈值，会再次报警。

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
