/*
 * main.c
 */

#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#include "adc.h"
#include "app_ctrl.h"
#include "beep.h"
#include "dc_motor.h"
#include "key.h"
#include "lcd9648.h"
#include "leds.h"
#include "protocol.h"
#include "relay.h"
#include "smg.h"
#include "time.h"
#include "uart.h"

#define APP_UART_BAUD       9600UL
#define APP_ADC_PERIOD_MS   200UL
#define APP_REPORT_MS       1000UL

void main(void)
{
    ProtocolRxBuffer rx;
    DspCommand cmd;
    char rx_line[PROTOCOL_LINE_MAX];
    char tx_line[PROTOCOL_LINE_MAX];
    Uint32 now = 0UL;
    Uint32 last_adc = 0UL;
    Uint16 last_lcd_centivolt = 0xFFFFU;
    Uint32 last_report = 0UL;
    Uint16 lcd_centivolt = 0U;
    int ch = 0;

    InitSysCtrl();
    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

    LED_Init();
    BEEP_Init();
    Relay_Init();
    KEY_Init();
    ADC_Init();
    LCD9648_Init();
    UARTa_Init(APP_UART_BAUD);
    DCMotor_ePWM3_Init();
    App_Init();
    TIM0_Init(150, 1000);

    Protocol_RxBufferInit(&rx);
    UARTa_SendString("{\"boot\":1}\n");
    Protocol_FormatStatus(App_GetStatus(), tx_line, PROTOCOL_LINE_MAX);
    UARTa_SendString(tx_line);

    while (1) {
        now = TIM0_GetTick();

        while ((ch = UARTa_ReadByteNonBlocking()) >= 0) {
            if (Protocol_RxAppend(&rx, ch, rx_line, PROTOCOL_LINE_MAX) != 0U) {
                if (Protocol_ParseCommand(rx_line, &cmd) != 0U) {
                    App_ApplyCommand(&cmd, now);
                }
            }
        }

        App_KeyTask(now);
        App_BeepTask(now);

        if ((now - last_adc) >= APP_ADC_PERIOD_MS) {
            last_adc = now;
            App_AdcTask(now);
            lcd_centivolt = (Uint16)((App_GetStatus()->adc_mv + 5U) / 10U);
            if (lcd_centivolt != last_lcd_centivolt) {
                last_lcd_centivolt = lcd_centivolt;
                LCD9648_DisplayVoltageMv(App_GetStatus()->adc_mv);
            }
        }

        if ((now - last_report) >= APP_REPORT_MS) {
            last_report = now;
            Protocol_FormatStatus(App_GetStatus(), tx_line, PROTOCOL_LINE_MAX);
            UARTa_SendString(tx_line);
        }
    }
}
