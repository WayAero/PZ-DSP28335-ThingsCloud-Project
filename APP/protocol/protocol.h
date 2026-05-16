/*
 * protocol.h
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#define PROTOCOL_LINE_MAX  160U

typedef struct {
    Uint16 led;
    Uint16 beep;
    Uint16 relay;
    Uint16 motor_enable;
    Uint16 motor_dir;
    Uint16 motor_speed;
    Uint16 adc_mv;
    Uint16 alarm;
} DspStatus;

typedef struct {
    Uint16 led_valid;
    Uint16 led;
    Uint16 beep_valid;
    Uint16 beep;
    Uint16 relay_valid;
    Uint16 relay;
    Uint16 motor_enable_valid;
    Uint16 motor_enable;
    Uint16 motor_dir_valid;
    Uint16 motor_dir;
    Uint16 motor_speed_valid;
    Uint16 motor_speed;
    Uint16 alarm_clear_valid;
    Uint16 alarm_clear;
} DspCommand;

typedef struct {
    char buffer[PROTOCOL_LINE_MAX];
    Uint16 index;
} ProtocolRxBuffer;

void Protocol_ResetCommand(DspCommand *cmd);
Uint16 Protocol_ParseCommand(char *line, DspCommand *cmd);
Uint16 Protocol_FormatStatus(const DspStatus *status, char *buffer, Uint16 size);
void Protocol_RxBufferInit(ProtocolRxBuffer *rx);
Uint16 Protocol_RxAppend(ProtocolRxBuffer *rx, int ch, char *line, Uint16 size);

#endif
