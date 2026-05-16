/*
 * uart.h
 */

#ifndef UART_H_
#define UART_H_

#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

void UARTa_Init(Uint32 baud);
void UARTa_SendByte(int a);
void UARTa_SendString(char *msg);
int UARTa_ReadByteNonBlocking(void);
Uint16 UARTa_RxAvailable(void);

#endif
