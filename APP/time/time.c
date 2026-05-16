/*
 * time.c
 */

#include "time.h"

static volatile Uint32 s_tim0_tick = 0UL;

void TIM0_Init(float Freq, float Period)
{
    s_tim0_tick = 0UL;

    EALLOW;
    SysCtrlRegs.PCLKCR3.bit.CPUTIMER0ENCLK = 1;
    EDIS;

    CpuTimer0.RegsAddr = &CpuTimer0Regs;
    CpuTimer0Regs.PRD.all = 0xFFFFFFFF;
    CpuTimer0Regs.TPR.all = 0;
    CpuTimer0Regs.TPRH.all = 0;
    CpuTimer0Regs.TCR.bit.TSS = 1;
    CpuTimer0Regs.TCR.bit.TRB = 1;
    CpuTimer0.InterruptCount = 0;

    ConfigCpuTimer(&CpuTimer0, Freq, Period);
    CpuTimer0Regs.TCR.bit.TIE = 0;
    CpuTimer0Regs.TCR.bit.TIF = 1;
    CpuTimer0Regs.TCR.bit.TRB = 1;
    CpuTimer0Regs.TCR.bit.TSS = 0;
}

interrupt void TIM0_IRQn(void)
{
    s_tim0_tick++;
    CpuTimer0Regs.TCR.bit.TIF = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

Uint32 TIM0_GetTick(void)
{
    Uint32 tick;

    if (CpuTimer0Regs.TCR.bit.TIF != 0U) {
        s_tim0_tick++;
        CpuTimer0Regs.TCR.bit.TIF = 1;
    }

    tick = s_tim0_tick;

    return tick;
}
