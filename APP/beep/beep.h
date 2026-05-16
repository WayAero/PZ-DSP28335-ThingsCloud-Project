/*
 * beep.h
 */

#ifndef BEEP_H_
#define BEEP_H_

#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#define BEEP_ON     BEEP_Set(1U)
#define BEEP_OFF    BEEP_Set(0U)

void BEEP_Init(void);
void BEEP_Set(Uint16 enable);

#endif
