/*
 * app_ctrl.c
 */

#include "app_ctrl.h"
#include "adc.h"
#include "beep.h"
#include "dc_motor.h"
#include "key.h"
#include "leds.h"
#include "relay.h"

#define APP_ADC_ALARM_SET_MV    2500U
#define APP_ADC_ALARM_CLEAR_MV  2400U
#define APP_ALARM_PAUSE_MS      10000UL

static DspStatus s_status;
static DspStatus s_report_status;
static Uint32 s_alarm_pause_until = 0UL;

static Uint16 App_Clamp01(Uint16 value)
{
    return (value != 0U) ? 1U : 0U;
}

static Uint16 App_ClampMotorDir(Uint16 dir)
{
    if ((dir == MOTOR_DIR_FORWARD) || (dir == MOTOR_DIR_REVERSE)) {
        return dir;
    }
    return MOTOR_DIR_FORWARD;
}

static Uint16 App_ClampMotorSpeed(Uint16 speed)
{
    if (speed > MOTOR_SPEED_MAX) {
        return MOTOR_SPEED_MAX;
    }
    return speed;
}

static Uint16 App_IsAlarmPaused(Uint32 now)
{
    if (s_alarm_pause_until == 0UL) {
        return 0U;
    }

    if ((Uint32)(s_alarm_pause_until - now) < 0x80000000UL) {
        return 1U;
    }

    s_alarm_pause_until = 0UL;
    return 0U;
}

static void App_ApplyLed(void)
{
    if (s_status.led != 0U) {
        LED1_ON;
    } else {
        LED1_OFF;
    }
}

static void App_ApplyBeep(void)
{
    if ((s_status.beep == 0U) && (s_status.alarm == 0U)) {
        BEEP_OFF;
    }
}

static void App_ApplyRelay(void)
{
    if (s_status.relay != 0U) {
        RELAY_ON;
    } else {
        RELAY_OFF;
    }
}

static void App_ApplyAlarmLed(void)
{
    if (s_status.alarm != 0U) {
        LED2_ON;
    } else {
        LED2_OFF;
    }
}

static void App_ApplyMotor(void)
{
    DCMotor_Set(
        s_status.motor_enable,
        s_status.motor_dir,
        s_status.motor_speed);
}

static void App_ApplyOutputs(void)
{
    App_ApplyLed();
    App_ApplyBeep();
    App_ApplyRelay();
    App_ApplyAlarmLed();
    App_ApplyMotor();
}

void App_Init(void)
{
    s_status.led = 0U;
    s_status.beep = 0U;
    s_status.relay = 0U;
    s_status.motor_enable = 0U;
    s_status.motor_dir = MOTOR_DIR_FORWARD;
    s_status.motor_speed = 0U;
    s_status.adc_mv = 0U;
    s_status.alarm = 0U;

    App_ApplyOutputs();
}

void App_KeyTask(Uint32 now)
{
    char key = KEY_Scan(0);

    switch (key) {
    case KEY1_PRESS:
        s_status.led = (s_status.led == 0U) ? 1U : 0U;
        App_ApplyLed();
        break;
    case KEY2_PRESS:
        s_status.beep = (s_status.beep == 0U) ? 1U : 0U;
        App_ApplyBeep();
        break;
    case KEY3_PRESS:
        s_status.relay = (s_status.relay == 0U) ? 1U : 0U;
        App_ApplyRelay();
        break;
    case KEY4_PRESS:
        s_status.motor_enable = (s_status.motor_enable == 0U) ? 1U : 0U;
        if ((s_status.motor_enable != 0U) && (s_status.motor_speed == 0U)) {
            s_status.motor_speed = 1U;
        }
        App_ApplyMotor();
        break;
    case KEY5_PRESS:
        if (s_status.motor_speed < MOTOR_SPEED_MAX) {
            s_status.motor_speed++;
        }
        App_ApplyMotor();
        break;
    case KEY6_PRESS:
        if (s_status.motor_speed > 0U) {
            s_status.motor_speed--;
        }
        App_ApplyMotor();
        break;
    case KEY7_PRESS:
        s_status.motor_dir = MOTOR_DIR_FORWARD;
        App_ApplyMotor();
        break;
    case KEY8_PRESS:
        s_status.motor_dir = MOTOR_DIR_REVERSE;
        App_ApplyMotor();
        break;
    case KEY9_PRESS:
        s_alarm_pause_until = now + APP_ALARM_PAUSE_MS;
        s_status.alarm = 0U;
        App_ApplyAlarmLed();
        App_ApplyBeep();
        break;
    default:
        break;
    }
}

void App_BeepTask(Uint32 now)
{
    (void)now;

    if ((s_status.beep == 0U) && (s_status.alarm == 0U)) {
        BEEP_OFF;
        return;
    }

    BEEP_ON;
}

void App_AdcTask(Uint32 now)
{
    Uint16 adc_value = 0U;
    Uint32 adc = 0UL;
    Uint16 mv = 0U;

    if (ADC_ReadValue(&adc_value) == 0U) {
        return;
    }

    adc = (Uint32)adc_value;
    mv = (Uint16)((adc * 3000UL) / 4096UL);

    s_status.adc_mv = mv;

    if (mv >= APP_ADC_ALARM_SET_MV) {
        if (App_IsAlarmPaused(now) != 0U) {
            s_status.alarm = 0U;
            App_ApplyAlarmLed();
            App_ApplyBeep();
            return;
        }
        s_status.alarm = 1U;
        App_ApplyAlarmLed();
        App_ApplyBeep();
    } else if (mv <= APP_ADC_ALARM_CLEAR_MV) {
        s_alarm_pause_until = 0UL;
        s_status.alarm = 0U;
        App_ApplyAlarmLed();
        App_ApplyBeep();
    }
}

void App_ApplyCommand(const DspCommand *cmd, Uint32 now)
{
    if (cmd->led_valid != 0U) {
        s_status.led = App_Clamp01(cmd->led);
        App_ApplyLed();
    }
    if (cmd->beep_valid != 0U) {
        s_status.beep = App_Clamp01(cmd->beep);
        App_ApplyBeep();
    }
    if (cmd->relay_valid != 0U) {
        s_status.relay = App_Clamp01(cmd->relay);
        App_ApplyRelay();
    }
    if (cmd->motor_enable_valid != 0U) {
        s_status.motor_enable = App_Clamp01(cmd->motor_enable);
        if ((s_status.motor_enable != 0U) && (s_status.motor_speed == 0U)) {
            s_status.motor_speed = 1U;
        }
        App_ApplyMotor();
    }
    if (cmd->motor_dir_valid != 0U) {
        s_status.motor_dir = App_ClampMotorDir(cmd->motor_dir);
        App_ApplyMotor();
    }
    if (cmd->motor_speed_valid != 0U) {
        s_status.motor_speed = App_ClampMotorSpeed(cmd->motor_speed);
        if (s_status.motor_speed == 0U) {
            s_status.motor_enable = 0U;
        }
        App_ApplyMotor();
    }
    if ((cmd->alarm_clear_valid != 0U) && (cmd->alarm_clear != 0U)) {
        s_alarm_pause_until = now + APP_ALARM_PAUSE_MS;
        s_status.alarm = 0U;
        App_ApplyAlarmLed();
        App_ApplyBeep();
    }
}

const DspStatus *App_GetStatus(void)
{
    s_report_status = s_status;
    s_report_status.beep = ((s_status.beep != 0U) || (s_status.alarm != 0U)) ? 1U : 0U;
    return &s_report_status;
}
