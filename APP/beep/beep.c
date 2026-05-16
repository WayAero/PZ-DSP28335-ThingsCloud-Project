/*
 * beep.c
 */

#include "beep.h"

#define BEEP_EPWM_TBPRD     7500U
#define BEEP_EPWM_CMPA      3750U

static Uint16 s_beep_enabled = 0U;

void BEEP_Init(void)
{
    EALLOW;
    SysCtrlRegs.PCLKCR1.bit.EPWM4ENCLK = 1;
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;

    GpioCtrlRegs.GPAPUD.bit.GPIO6 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;

    EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
    EPwm4Regs.TBPRD = BEEP_EPWM_TBPRD;
    EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm4Regs.TBPHS.half.TBPHS = 0x0000;
    EPwm4Regs.TBCTR = 0x0000;
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = TB_DIV2;
    EPwm4Regs.TBCTL.bit.CLKDIV = TB_DIV2;

    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm4Regs.CMPA.half.CMPA = 0U;
    EPwm4Regs.AQCTLA.bit.ZRO = AQ_CLEAR;
    EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm4Regs.ETSEL.bit.INTEN = 0;

    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

    s_beep_enabled = 0U;
}

void BEEP_Set(Uint16 enable)
{
    if (enable != 0U) {
        if (s_beep_enabled == 0U) {
            EPwm4Regs.CMPA.half.CMPA = BEEP_EPWM_CMPA;
            EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;
            EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
            s_beep_enabled = 1U;
        }
    } else if (s_beep_enabled != 0U) {
        EPwm4Regs.CMPA.half.CMPA = 0U;
        EPwm4Regs.AQCTLA.bit.ZRO = AQ_CLEAR;
        EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;
        s_beep_enabled = 0U;
    }
}
