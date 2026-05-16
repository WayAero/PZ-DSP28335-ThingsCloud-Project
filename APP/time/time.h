/*
 * time.h
 */

#ifndef TIME_H_
#define TIME_H_

#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

void TIM0_Init(float Freq, float Period);
interrupt void TIM0_IRQn(void);
Uint32 TIM0_GetTick(void);

#endif
