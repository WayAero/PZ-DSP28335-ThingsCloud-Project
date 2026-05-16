/*
 * dc_motor.c
 *
 * ePWM3A/ePWM3B drive the second motor bridge channel on GPIO4/GPIO5.
 */

#include "dc_motor.h"

#define EPWM3_TIMER_TBPRD  3750U
#define EPWM3_MAX_CMP      3500U

static Uint16 s_motor_enable = 0;
static Uint16 s_motor_dir = MOTOR_DIR_FORWARD;
static Uint16 s_motor_speed = 0;

static Uint16 DCMotor_SpeedToCompare(Uint16 speed)
{
    static const Uint16 table[MOTOR_SPEED_MAX + 1] = {
        0U, 500U, 1000U, 1500U, 2000U, 2500U, 3000U, EPWM3_MAX_CMP
    };

    if (speed > MOTOR_SPEED_MAX) {
        speed = MOTOR_SPEED_MAX;
    }

    return table[speed];
}

static void DCMotor_ApplyOutputs(void)
{
    Uint16 cmp = 0U;

    if (s_motor_enable != 0U) {
        cmp = DCMotor_SpeedToCompare(s_motor_speed);
    }

    if ((s_motor_enable == 0U) || (cmp == 0U) || (s_motor_dir == MOTOR_DIR_STOP)) {
        EPwm3Regs.CMPA.half.CMPA = 0U;
        EPwm3Regs.CMPB = 0U;
        EPwm3Regs.AQCTLA.bit.ZRO = AQ_CLEAR;
        EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
        EPwm3Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
        EPwm3Regs.AQCTLB.bit.CBU = AQ_CLEAR;
        return;
    }

    EPwm3Regs.CMPA.half.CMPA = 0U;
    EPwm3Regs.CMPB = 0U;

    if (s_motor_dir == MOTOR_DIR_REVERSE) {
        EPwm3Regs.AQCTLA.bit.ZRO = AQ_CLEAR;
        EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
        EPwm3Regs.AQCTLB.bit.ZRO = AQ_SET;
        EPwm3Regs.AQCTLB.bit.CBU = AQ_CLEAR;
        EPwm3Regs.CMPB = cmp;
    } else {
        EPwm3Regs.AQCTLA.bit.ZRO = AQ_SET;
        EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
        EPwm3Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
        EPwm3Regs.AQCTLB.bit.CBU = AQ_CLEAR;
        EPwm3Regs.CMPA.half.CMPA = cmp;
    }
}

void DC_Motor_Init(void)
{
    DCMotor_ePWM3_Init();
}

void DCMotor_ePWM3_Init(void)
{
    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    SysCtrlRegs.PCLKCR1.bit.EPWM3ENCLK = 1;
    EDIS;

    InitEPwm3Gpio();

    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm3Regs.TBPRD = EPWM3_TIMER_TBPRD;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm3Regs.TBPHS.half.TBPHS = 0x0000;
    EPwm3Regs.TBCTR = 0x0000;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV2;
    EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV2;

    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    EPwm3Regs.CMPA.half.CMPA = 0U;
    EPwm3Regs.CMPB = 0U;
    EPwm3Regs.AQCTLA.bit.ZRO = AQ_CLEAR;
    EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm3Regs.AQCTLB.bit.ZRO = AQ_CLEAR;
    EPwm3Regs.AQCTLB.bit.CBU = AQ_CLEAR;

    EPwm3Regs.ETSEL.bit.INTEN = 0;

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    DCMotor_Stop();
}

void DCMotor_Set(Uint16 enable, Uint16 dir, Uint16 speed)
{
    s_motor_enable = (enable != 0U) ? 1U : 0U;

    if ((dir == MOTOR_DIR_FORWARD) || (dir == MOTOR_DIR_REVERSE)) {
        s_motor_dir = dir;
    } else if (dir == MOTOR_DIR_STOP) {
        s_motor_enable = 0U;
    }

    if (speed > MOTOR_SPEED_MAX) {
        speed = MOTOR_SPEED_MAX;
    }
    s_motor_speed = speed;

    DCMotor_ApplyOutputs();
}

void DCMotor_Stop(void)
{
    s_motor_enable = 0U;
    s_motor_speed = 0U;
    DCMotor_ApplyOutputs();
}
