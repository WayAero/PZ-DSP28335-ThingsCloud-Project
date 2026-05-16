/*
 * protocol.c
 */

#include "protocol.h"
#include <stdlib.h>
#include <string.h>

static Uint16 Protocol_AppendChar(char *buffer, Uint16 size, Uint16 *index, char ch)
{
    if (*index >= (size - 1U)) {
        return 0U;
    }

    buffer[*index] = ch;
    (*index)++;
    buffer[*index] = '\0';
    return 1U;
}

static Uint16 Protocol_AppendString(char *buffer, Uint16 size, Uint16 *index, const char *text)
{
    Uint16 i = 0U;

    while (text[i] != '\0') {
        if (Protocol_AppendChar(buffer, size, index, text[i]) == 0U) {
            return 0U;
        }
        i++;
    }

    return 1U;
}

static Uint16 Protocol_AppendUint(char *buffer, Uint16 size, Uint16 *index, Uint16 value)
{
    char digits[5];
    Uint16 count = 0U;

    if (value == 0U) {
        return Protocol_AppendChar(buffer, size, index, '0');
    }

    while ((value > 0U) && (count < sizeof(digits))) {
        digits[count] = (char)('0' + (value % 10U));
        value = value / 10U;
        count++;
    }

    while (count > 0U) {
        count--;
        if (Protocol_AppendChar(buffer, size, index, digits[count]) == 0U) {
            return 0U;
        }
    }

    return 1U;
}

static Uint16 Protocol_FindInt(char *line, const char *key, int *value)
{
    char pattern[32];
    char *p;

    pattern[0] = '"';
    pattern[1] = '\0';
    strncat(pattern, key, sizeof(pattern) - 3U);
    strcat(pattern, "\"");

    p = strstr(line, pattern);
    if (p == 0) {
        return 0U;
    }

    p = strchr(p, ':');
    if (p == 0) {
        return 0U;
    }
    p++;

    while ((*p == ' ') || (*p == '\t')) {
        p++;
    }

    if (strncmp(p, "true", 4) == 0) {
        *value = 1;
        return 1U;
    }
    if (strncmp(p, "false", 5) == 0) {
        *value = 0;
        return 1U;
    }

    *value = (int)strtol(p, 0, 10);
    return 1U;
}

void Protocol_ResetCommand(DspCommand *cmd)
{
    cmd->led_valid = 0U;
    cmd->led = 0U;
    cmd->beep_valid = 0U;
    cmd->beep = 0U;
    cmd->relay_valid = 0U;
    cmd->relay = 0U;
    cmd->motor_enable_valid = 0U;
    cmd->motor_enable = 0U;
    cmd->motor_dir_valid = 0U;
    cmd->motor_dir = 0U;
    cmd->motor_speed_valid = 0U;
    cmd->motor_speed = 0U;
    cmd->alarm_clear_valid = 0U;
    cmd->alarm_clear = 0U;
}

Uint16 Protocol_ParseCommand(char *line, DspCommand *cmd)
{
    int value = 0;
    Uint16 found = 0U;

    Protocol_ResetCommand(cmd);

    if (Protocol_FindInt(line, "led", &value) != 0U) {
        cmd->led_valid = 1U;
        cmd->led = (value != 0) ? 1U : 0U;
        found = 1U;
    }
    if (Protocol_FindInt(line, "beep", &value) != 0U) {
        cmd->beep_valid = 1U;
        cmd->beep = (value != 0) ? 1U : 0U;
        found = 1U;
    }
    if (Protocol_FindInt(line, "relay", &value) != 0U) {
        cmd->relay_valid = 1U;
        cmd->relay = (value != 0) ? 1U : 0U;
        found = 1U;
    }
    if (Protocol_FindInt(line, "motor_enable", &value) != 0U) {
        cmd->motor_enable_valid = 1U;
        cmd->motor_enable = (value != 0) ? 1U : 0U;
        found = 1U;
    }
    if (Protocol_FindInt(line, "motor_dir", &value) != 0U) {
        cmd->motor_dir_valid = 1U;
        cmd->motor_dir = (Uint16)value;
        found = 1U;
    }
    if (Protocol_FindInt(line, "motor_speed", &value) != 0U) {
        cmd->motor_speed_valid = 1U;
        cmd->motor_speed = (Uint16)value;
        found = 1U;
    }
    if (Protocol_FindInt(line, "alarm_clear", &value) != 0U) {
        cmd->alarm_clear_valid = 1U;
        cmd->alarm_clear = (value != 0) ? 1U : 0U;
        found = 1U;
    }

    return found;
}

Uint16 Protocol_FormatStatus(const DspStatus *status, char *buffer, Uint16 size)
{
    Uint16 index = 0U;

    if (size == 0U) {
        return 0U;
    }

#define APPEND_TEXT(text)   do { if (Protocol_AppendString(buffer, size, &index, (text)) == 0U) return index; } while (0)
#define APPEND_UINT(value)  do { if (Protocol_AppendUint(buffer, size, &index, (value)) == 0U) return index; } while (0)

    buffer[0] = '\0';

    APPEND_TEXT("{\"led\":");
    APPEND_UINT(status->led);
    APPEND_TEXT(",\"beep\":");
    APPEND_UINT(status->beep);
    APPEND_TEXT(",\"relay\":");
    APPEND_UINT(status->relay);
    APPEND_TEXT(",\"motor_enable\":");
    APPEND_UINT(status->motor_enable);
    APPEND_TEXT(",\"motor_dir\":");
    APPEND_UINT(status->motor_dir);
    APPEND_TEXT(",\"motor_speed\":");
    APPEND_UINT(status->motor_speed);
    APPEND_TEXT(",\"adc_mv\":");
    APPEND_UINT(status->adc_mv);
    APPEND_TEXT(",\"alarm\":");
    APPEND_UINT(status->alarm);
    APPEND_TEXT("}\n");

#undef APPEND_TEXT
#undef APPEND_UINT

    return index;
}

void Protocol_RxBufferInit(ProtocolRxBuffer *rx)
{
    rx->index = 0U;
    rx->buffer[0] = '\0';
}

Uint16 Protocol_RxAppend(ProtocolRxBuffer *rx, int ch, char *line, Uint16 size)
{
    Uint16 i;

    if (ch == '\r') {
        return 0U;
    }

    if (ch == '\n') {
        if (rx->index == 0U) {
            return 0U;
        }

        rx->buffer[rx->index] = '\0';
        for (i = 0U; (i < size - 1U) && (rx->buffer[i] != '\0'); i++) {
            line[i] = rx->buffer[i];
        }
        line[i] = '\0';
        rx->index = 0U;
        rx->buffer[0] = '\0';
        return 1U;
    }

    if ((ch < 0x20) || (ch > 0x7e)) {
        return 0U;
    }

    if (rx->index >= (PROTOCOL_LINE_MAX - 1U)) {
        rx->index = 0U;
        rx->buffer[0] = '\0';
        return 0U;
    }

    rx->buffer[rx->index] = (char)ch;
    rx->index++;
    rx->buffer[rx->index] = '\0';

    return 0U;
}
