/*
 * smg.c
 */

#include "smg.h"

#define SMG_DOT 0x80U

static const unsigned char s_smg_digits[10] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66,
    0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

static void SMG_SelectDigit(unsigned char index)
{
    switch (index) {
    case 0:
        SEG1_SETH;
        SEG2_SETL;
        SEG3_SETL;
        SEG4_SETL;
        break;
    case 1:
        SEG1_SETL;
        SEG2_SETH;
        SEG3_SETL;
        SEG4_SETL;
        break;
    case 2:
        SEG1_SETL;
        SEG2_SETL;
        SEG3_SETH;
        SEG4_SETL;
        break;
    default:
        SEG1_SETL;
        SEG2_SETL;
        SEG3_SETL;
        SEG4_SETH;
        break;
    }
}

static void HC164SendData(unsigned char dat)
{
    unsigned char i = 0U;

    for (i = 0U; i < 8U; i++) {
        SPICLKA_SETL;
        if ((dat & 0x80U) != 0U) {
            SPISIMOA_SETH;
        } else {
            SPISIMOA_SETL;
        }
        SPICLKA_SETH;
        dat <<= 1;
    }
}

void SMG_Init(void)
{
    EALLOW;
    SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1;

    GpioCtrlRegs.GPBMUX2.bit.GPIO56 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO56 = 1;
    GpioCtrlRegs.GPBPUD.bit.GPIO56 = 0;

    GpioCtrlRegs.GPBMUX2.bit.GPIO54 = 0;
    GpioCtrlRegs.GPBDIR.bit.GPIO54 = 1;
    GpioCtrlRegs.GPBPUD.bit.GPIO54 = 0;

    GpioCtrlRegs.GPCMUX1.bit.GPIO70 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO70 = 1;
    GpioCtrlRegs.GPCPUD.bit.GPIO70 = 0;

    GpioCtrlRegs.GPCMUX1.bit.GPIO71 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO71 = 1;
    GpioCtrlRegs.GPCPUD.bit.GPIO71 = 0;

    GpioCtrlRegs.GPCMUX1.bit.GPIO72 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO72 = 1;
    GpioCtrlRegs.GPCPUD.bit.GPIO72 = 0;

    GpioCtrlRegs.GPCMUX1.bit.GPIO73 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO73 = 1;
    GpioCtrlRegs.GPCPUD.bit.GPIO73 = 0;
    EDIS;

    SEG1_SETL;
    SEG2_SETL;
    SEG3_SETL;
    SEG4_SETL;
}

void SMG_DisplayVoltageMv(Uint16 mv)
{
    unsigned char buf[4];
    static unsigned char digit = 0U;
    Uint16 centivolt = 0U;

    if (mv > 9990U) {
        mv = 9990U;
    }

    centivolt = (Uint16)((mv + 5U) / 10U);

    buf[0] = s_smg_digits[centivolt / 100U] | SMG_DOT;
    buf[1] = s_smg_digits[(centivolt / 10U) % 10U];
    buf[2] = s_smg_digits[centivolt % 10U];
    buf[3] = 0x00U;

    HC164SendData(buf[digit]);
    SMG_SelectDigit(digit);
    DELAY_US(500);

    digit++;
    if (digit >= 4U) {
        digit = 0U;
    }
}
