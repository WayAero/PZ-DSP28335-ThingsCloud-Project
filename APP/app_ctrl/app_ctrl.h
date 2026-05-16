/*
 * app_ctrl.h
 */

#ifndef APP_CTRL_H_
#define APP_CTRL_H_

#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include "protocol.h"

void App_Init(void);
void App_KeyTask(Uint32 now);
void App_BeepTask(Uint32 now);
void App_AdcTask(Uint32 now);
void App_ApplyCommand(const DspCommand *cmd, Uint32 now);
const DspStatus *App_GetStatus(void);

#endif
