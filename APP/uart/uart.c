/*
 * uart.c
 */

#include "uart.h"

void UARTa_Init(Uint32 baud)
{
    unsigned char scihbaud = 0U;
    unsigned char scilbaud = 0U;
    Uint16 scibaud = 0U;

    scibaud = (Uint16)(37500000UL / (8UL * baud) - 1UL);
    scihbaud = (unsigned char)(scibaud >> 8);
    scilbaud = (unsigned char)(scibaud & 0xffU);

    EALLOW;
    SysCtrlRegs.PCLKCR0.bit.SCIAENCLK = 1;
    EDIS;

    InitSciaGpio();

    SciaRegs.SCIFFTX.all = 0xE040;
    SciaRegs.SCIFFRX.all = 0x204f;
    SciaRegs.SCIFFCT.all = 0x0;

    SciaRegs.SCICCR.all = 0x0007;
    SciaRegs.SCICTL1.all = 0x0003;
    SciaRegs.SCICTL2.all = 0x0003;
    SciaRegs.SCICTL2.bit.TXINTENA = 0;
    SciaRegs.SCICTL2.bit.RXBKINTENA = 0;
    SciaRegs.SCIHBAUD = scihbaud;
    SciaRegs.SCILBAUD = scilbaud;
    SciaRegs.SCICTL1.all = 0x0023;
}

void UARTa_SendByte(int a)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {
    }
    SciaRegs.SCITXBUF = a;
}

void UARTa_SendString(char *msg)
{
    Uint16 i = 0U;

    while (msg[i] != '\0') {
        UARTa_SendByte(msg[i]);
        i++;
    }
}

Uint16 UARTa_RxAvailable(void)
{
    return SciaRegs.SCIFFRX.bit.RXFFST;
}

int UARTa_ReadByteNonBlocking(void)
{
    int data;

    if (SciaRegs.SCIFFRX.bit.RXFFOVF != 0U) {
        SciaRegs.SCIFFRX.bit.RXFFOVRCLR = 1;
    }

    if (SciaRegs.SCIFFRX.bit.RXFFST == 0U) {
        return -1;
    }

    data = (int)(SciaRegs.SCIRXBUF.all & 0x00FF);
    return data;
}
