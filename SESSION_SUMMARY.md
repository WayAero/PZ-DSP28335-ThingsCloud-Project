# DSP28335 + ESP32 ThingsCloud 项目会话摘要

更新时间：2026-05-15 15:45

## 项目路径

- DSP 工程：`<DSP工程路径>`
- DSP 例程：`<DSP例程路径>`
- ESP32 工程：`<ESP32工程路径>`

## 当前目标

课程设计基于 TMS320F28335 DSP 开发板和 ESP32，完成工业物联网模型。

当前分工：

1. DSP 负责 ADC、LCD9648 显示、LED、蜂鸣器、继电器、电机、按键。
2. DSP 通过 SCI-A 串口和 ESP32 通信。
3. ESP32 负责 Wi-Fi、DHT22 温湿度、ThingsCloud MQTT、DSP 串口桥接。
4. ThingsCloud 使用设备属性查看和属性下发。

## 关键决策

- 云平台采用 ThingsCloud。
- ThingsCloud 使用属性上报和属性下发。
- 不使用命令下发 `command/send/+`。
- 不使用自定义数据流。
- ThingsCloud 属性使用扁平字段。
- DSP 和 ESP32 之间采用 JSON 行协议。
- DSP 串口 JSON 命令必须以 `\n` 结尾，否则 DSP 会缓存但不会解析。
- ThingsCloud 属性下发的 MQTT JSON 不需要 `\n`；ESP32 转发到 DSP 串口时会自动追加 `\n`。
- 电机已从 ePWM2 改为 ePWM3，使用 `GPIO4/GPIO5`，接 TC1508S 第二通道 `INC/IND`。
- LCD9648 已合入主工程，显示 ADC 电压。
- LCD9648 初始化已改为逐位配置 GPIO，避免整口写 `GPAMUX1.all` / `GPADIR.all` 破坏 ePWM3。
- 数码管暂时停用，但 `APP/smg` 文件和工程项保留。
- 蜂鸣器已改为 ePWM4A / GPIO6 输出，避免 LCD9648 刷新导致声音断点。

## 串口接口

DSP 和 ESP32 物理连接：

```text
DSP GPIO35 / SCITXDA -> ESP32 GPIO16 / RX2
DSP GPIO36 / SCIRXDA <- ESP32 GPIO17 / TX2
GND                  <-> GND
```

串口参数：

```text
9600 8N1
```

DSP 到 ESP32，JSON 行，以 `\n` 结尾：

```json
{"led":0,"beep":0,"relay":0,"motor_enable":0,"motor_dir":1,"motor_speed":0,"adc_mv":1450,"alarm":0}
```

ESP32 到 DSP，JSON 行，以 `\n` 结尾：

```json
{"led":1}
{"beep":0}
{"relay":1}
{"motor_enable":1}
{"motor_dir":2}
{"motor_speed":3}
{"alarm_clear":1}
```

串口助手发送时要开启“发送新行 / 回车换行”，或用 HEX 追加 `0A`。

## ThingsCloud 属性字段

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

ESP32 端已验证：

- ThingsCloud 设备属性里可以看到 `temperature`。
- ThingsCloud 设备属性里可以看到 `humidity`。
- 温湿度会更新。
- ThingsCloud 设备属性里可以看到 DSP 状态字段。
- ThingsCloud 属性下发可以控制 DSP 外设。
- ESP32 + DSP + ThingsCloud 闭环已实测通过。

## DSP 当前模块

DSP 工程当前 APP 模块：

```text
APP/adc
APP/app_ctrl
APP/beep
APP/dc_motor
APP/epwm
APP/key
APP/lcd9648
APP/leds
APP/protocol
APP/relay
APP/smg
APP/time
APP/uart
```

说明：

- `APP/lcd9648`：当前实际使用，显示 ADC 电压。
- `APP/smg`：保留但当前主循环不调用。

DSP 库目录已补入：

```text
DSP2833x_Libraries/DSP2833x_Adc.c
DSP2833x_Libraries/DSP2833x_CpuTimers.c
DSP2833x_Libraries/DSP2833x_EPwm.c
DSP2833x_Libraries/DSP2833x_Sci.c
```

## DSP 当前行为

`User/main.c` 当前为主循环调度结构：

- 初始化系统、PIE、LED、蜂鸣器、继电器、按键、ADC、LCD9648、UART、ePWM3、电机状态、Timer0。
- 串口启动后发送 `{"boot":1}`。
- 随后立即发送一帧完整状态 JSON。
- 主循环轮询接收 JSON 行。
- 收到完整命令后调用 `App_ApplyCommand(&cmd, now)`。
- 周期执行按键任务、蜂鸣器任务、ADC 任务、LCD9648 更新、状态上报。
- ADC 周期：200 ms。
- 状态上报周期：1000 ms。
- 串口波特率：9600。
- LCD9648 只在 ADC 显示值变化到 0.01V 级别时刷新。

## DSP 已修复问题

### 链接内存不足

原错误：

```text
#10099-D program will not fit into available memory
```

修复：

- `DSP2833x_Libraries/28335_RAM_lnk.cmd` 中把 `RAML1/RAML2/RAML3` 合并为连续 `RAML1L3`。
- `.text` 和 `IQmath` 放到 `RAML1L3`。

### 串口只输出 `{"boot":1}`

已修复：

- `APP/time/time.c`：Timer0 改为轮询 `TIF` 标志累加 tick，不再依赖 PIE 中断。
- `APP/adc/adc.c`：新增 `ADC_ReadValue()`，ADC 等待加入超时。
- `APP/app_ctrl/app_ctrl.c`：ADC 超时时跳过本次更新，不阻塞主循环。
- `APP/protocol/protocol.c`：状态 JSON 格式化去掉 `sprintf`，改为手动拼接。
- `User/main.c`：`{"boot":1}` 后立即发送完整状态帧。

### 蜂鸣器报警时 `beep` 仍为 0

原因：

- 内部 `s_status.beep` 原本表示手动蜂鸣器开关。
- 实际蜂鸣器输出是 `s_status.beep || s_status.alarm`。

修复：

- `App_GetStatus()` 上报实际蜂鸣器状态。
- 报警响时串口 JSON 中 `beep=1`。

### 蜂鸣器声音有断点

原因：

- 原先蜂鸣器靠主循环 `BEEP_TOGGLE`。
- LCD9648 软件模拟时序刷新会占用主循环，导致蜂鸣器翻转不均匀。

修复：

- `APP/beep/beep.c` 改为 ePWM4A / GPIO6 输出。
- `BEEP_ON` / `BEEP_OFF` 改为控制 ePWM4A 输出。
- 蜂鸣器不再依赖主循环高频翻转。

### KEY9 / alarm_clear 无法暂停高 ADC 报警

原因：

- KEY9 清除后，如果 ADC 仍大于 2500mV，下一次 ADC 任务会马上再次置位报警。

修复：

- KEY9 改为暂停报警 10 秒。
- `alarm_clear=1` 改为暂停报警 10 秒。
- 10 秒内 ADC 仍高于阈值，也不会重新报警。
- 10 秒后 ADC 仍高于阈值，会再次报警。

## DSP 当前控制规则

### 报警

```text
adc_mv >= 2500：alarm=1，LED2 亮，蜂鸣器响
adc_mv <= 2400：alarm=0，LED2 灭；如果手动蜂鸣器未开，蜂鸣器停
2400 < adc_mv < 2500：保持原报警状态
KEY9：暂停报警 10 秒
alarm_clear=1：暂停报警 10 秒
```

### 蜂鸣器

```text
GPIO6 / ePWM4A -> 蜂鸣器
```

当前实现：

- 手动 `beep=1` 或 `alarm=1` 时输出 ePWM4A 方波。
- 手动 `beep=0` 且 `alarm=0` 时关闭。
- 串口 JSON 的 `beep` 表示实际蜂鸣器输出状态。

### 电机

```text
motor_enable: 0/1
motor_dir: 1 正转，2 反转
motor_speed: 0-7
```

当前电机通道：

```text
GPIO4 / ePWM3A -> TC1508S INC
GPIO5 / ePWM3B -> TC1508S IND
```

PWM 档位表：

```text
0, 500, 1000, 1500, 2000, 2500, 3000, 3500
```

实测现象：

- 初始时 1-6 档可能不转。
- 7 档启动后，后续低档可正常转。
- 判断更像电机静摩擦 / 启动扭矩问题，当前程序未做强制高档启动。

### LCD9648

当前显示：

```text
ADC VOLT
2.48V
```

引脚：

```text
GPIO0  -> SCL
GPIO60 -> SDA
GPIO1  -> RS
GPIO2  -> CS
GPIO3  -> RSET
```

注意：

- LCD9648 驱动只逐位配置 GPIO0、GPIO1、GPIO2、GPIO3、GPIO60。
- 不再整口配置 GPIO0-GPIO15。
- 因此不会破坏 GPIO4/GPIO5 的 ePWM3 电机输出。
- LCD9648 模块 VCC 未从本地资料最终确认。F28335 GPIO 是 3.3V 逻辑，外接前优先查模块丝印或资料。

### 数码管

- `APP/smg` 代码保留。
- 当前 `User/main.c` 不调用 `SMG_Init()`。
- 当前 `User/main.c` 不调用 `SMG_DisplayVoltageMv()`。
- 现阶段数码管不显示。

## DSP 单板测试结果

用户已实测通过：

1. 每秒稳定输出 JSON。
2. 串口发送 JSON 能控制外设。
3. 按键能控制外设。
4. ADC 电压能变化。
5. LCD9648 可显示 ADC 电压。
6. ADC 超过阈值能报警。
7. `alarm_clear` / KEY9 能清除或暂停报警。
8. 蜂鸣器响时，串口 JSON 的 `beep` 能正确反映实际状态。
9. 数码管已停用，其他功能正常。
10. 蜂鸣器改 ePWM4A 后无已知问题。
11. `alarm_clear=1` 在 ADC 仍高于阈值时，能像 KEY9 一样暂停报警约 10 秒。

结论：

```text
DSP 单板功能已完成。
```

## 当前最新 DSP 构建

已在本机使用 CCS 7.4.0 生成的 makefile 执行：

```text
gmake clean all
gmake all
```

构建结果：

```text
编译通过
链接通过
```

最新输出文件：

```text
<DSP工程路径>\Debug\DSP2833x_Course Project.out
```

最新观察：

```text
时间：2026-05-15 15:43:13
大小：222928 bytes
.text：0x1325
RAML1L3 剩余：0x1cdb
```

## ESP32 当前状态

ESP32 工程：

```text
<ESP32工程路径>
```

当前 `src/main.cpp` 已加入：

- `Serial2` DSP 串口桥接。
- `Serial2` 参数：`9600 8N1`。
- `Serial2` 引脚：`RX2=GPIO16`，`TX2=GPIO17`。
- DSP JSON 行读取。
- DSP 状态字段解析：
  - `led`
  - `beep`
  - `relay`
  - `motor_enable`
  - `motor_dir`
  - `motor_speed`
  - `adc_mv`
  - `alarm`
- ThingsCloud 属性上报。
- ThingsCloud 属性下发转 DSP JSON。
- MQTT 连接后调用 `client.getAttributes()` 同步云端属性。
- DHT22 读取失败时只跳过温湿度，不清空 DSP 状态。

已通过本机 PlatformIO 编译。

ESP32 已实测通过：

- ThingsCloud 设备属性里能看到 `temperature`。
- ThingsCloud 设备属性里能看到 `humidity`。
- 温湿度会更新。
- ESP32 能通过 `Serial2` 收到 DSP 每秒 JSON。
- ESP32 能把 DSP 状态上报到 ThingsCloud。
- ESP32 能把 ThingsCloud 属性下发转发给 DSP。
- ESP32 转发到 DSP 串口时会追加换行符。

## ESP32 + DSP + ThingsCloud 联调结果

已验证接线：

```text
DSP GPIO35 / SCITXDA -> ESP32 GPIO16 / RX2
DSP GPIO36 / SCIRXDA <- ESP32 GPIO17 / TX2
GND                  <-> GND
```

已实测通过：

1. ESP32 串口监视器能看到 `DSP -> ESP32: {...}`。
2. ThingsCloud 能看到 `adc_voltage`。
3. ThingsCloud 能看到 DSP 的 `led/beep/relay/motor_enable/motor_dir/motor_speed/alarm`。
4. ThingsCloud 下发 `led/beep/relay/motor_enable/motor_dir/motor_speed/alarm_clear` 后，ESP32 日志能看到 `ESP32 -> DSP: {...}`。
5. DSP 外设动作与云端下发一致。
6. DSP 回传状态能继续刷新到 ThingsCloud。

注意：

- ESP32 不使用 RX0/TX0 连接 DSP。
- RX0/TX0 留给 USB 日志串口。
- DSP 桥接使用 ESP32 `GPIO16/GPIO17`。

### ThingsCloud 页面控件

建议创建或检查这些控件：

```text
temperature 显示
humidity 显示
adc_voltage 显示
led 开关
beep 开关
relay 开关
motor_enable 开关
motor_dir 枚举或数值
motor_speed 数值，范围 0-7
alarm 状态显示
alarm_clear 按钮或布尔下发
```

## 当前验收状态

项目主链路已完成：

1. DSP 单板功能完成。
2. ESP32 温湿度上报完成。
3. ESP32 接收 DSP 状态完成。
4. ThingsCloud 查看 DSP + ESP32 属性完成。
5. ThingsCloud 属性下发控制 DSP 完成。
6. 报警、清警、暂停报警完成。
7. LCD9648 显示 ADC 电压完成。
8. 本地 Git 留档已启用。

后续更适合进入课程报告、演示流程整理、接线图整理和最终答辩材料整理。

## Git 本地留档

DSP 工程已初始化本地 Git 仓库。

历史留档：

```text
b2af3ac Archive DSP course project state
```

当前这次留档包含：

- `alarm_clear=1` 与 KEY9 一样暂停报警 10 秒。
- `SESSION_SUMMARY.md` 更新到 ESP32 + DSP + ThingsCloud 闭环已通过。
- 最新 DSP `.out` 已重新构建。

## 参考资料

- 任务书：`DSP课程设计任务书_2026.docx`
- 原理图：`PZ-DSP28335-L开发板原理图.pdf`
- 开发攻略：`普中DSP28335开发攻略.pdf`
- 已验证例程清单：`例程.xlsx`
- ThingsCloud MQTT 文档：`https://www.thingscloud.xyz/docs/guide/connect-device/mqtt.html`

## 下次会话入口

```text
请继续从 SESSION_SUMMARY.md 接手，当前 DSP + ESP32 + ThingsCloud 主链路已完成，下一步整理课程报告和演示材料。
```
