/*
 * lcd9648.c
 */

#include "lcd9648.h"

#define LCD9648_WIDTH       96U
#define LCD9648_PAGES       6U
#define LCD9648_GLYPH_W     6U

static void LCD9648_SendData(unsigned char dat)
{
    unsigned char i;

    for (i = 0U; i < 8U; i++) {
        if ((dat & 0x80U) != 0U) {
            LCD9648_SDA_SETH;
        } else {
            LCD9648_SDA_SETL;
        }

        dat <<= 1;

        LCD9648_SCL_SETL;
        DELAY_US(1);
        LCD9648_SCL_SETH;
        DELAY_US(1);
    }
}

static void LCD9648_WriteCmd(unsigned char cmd)
{
    LCD9648_CS_SETL;
    LCD9648_RS_SETL;
    DELAY_US(2);
    LCD9648_SendData(cmd);
    DELAY_US(2);
    LCD9648_CS_SETH;
}

static void LCD9648_WriteData(unsigned char data)
{
    LCD9648_CS_SETL;
    LCD9648_RS_SETH;
    DELAY_US(2);
    LCD9648_SendData(data);
    DELAY_US(2);
    LCD9648_CS_SETH;
}

static void LCD9648_SetPos(Uint16 x, Uint16 page)
{
    LCD9648_WriteCmd((unsigned char)(0xB0U + page));
    LCD9648_WriteCmd((unsigned char)(0x10U + ((x >> 4U) & 0x0FU)));
    LCD9648_WriteCmd((unsigned char)(x & 0x0FU));
}

static const unsigned char *LCD9648_Glyph(char ch)
{
    static const unsigned char glyph_space[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    static const unsigned char glyph_0[5] = {0x3E, 0x51, 0x49, 0x45, 0x3E};
    static const unsigned char glyph_1[5] = {0x00, 0x42, 0x7F, 0x40, 0x00};
    static const unsigned char glyph_2[5] = {0x42, 0x61, 0x51, 0x49, 0x46};
    static const unsigned char glyph_3[5] = {0x21, 0x41, 0x45, 0x4B, 0x31};
    static const unsigned char glyph_4[5] = {0x18, 0x14, 0x12, 0x7F, 0x10};
    static const unsigned char glyph_5[5] = {0x27, 0x45, 0x45, 0x45, 0x39};
    static const unsigned char glyph_6[5] = {0x3C, 0x4A, 0x49, 0x49, 0x30};
    static const unsigned char glyph_7[5] = {0x01, 0x71, 0x09, 0x05, 0x03};
    static const unsigned char glyph_8[5] = {0x36, 0x49, 0x49, 0x49, 0x36};
    static const unsigned char glyph_9[5] = {0x06, 0x49, 0x49, 0x29, 0x1E};
    static const unsigned char glyph_dot[5] = {0x00, 0x60, 0x60, 0x00, 0x00};
    static const unsigned char glyph_colon[5] = {0x00, 0x36, 0x36, 0x00, 0x00};
    static const unsigned char glyph_a[5] = {0x7E, 0x11, 0x11, 0x11, 0x7E};
    static const unsigned char glyph_c[5] = {0x3E, 0x41, 0x41, 0x41, 0x22};
    static const unsigned char glyph_d[5] = {0x7F, 0x41, 0x41, 0x22, 0x1C};
    static const unsigned char glyph_l[5] = {0x7F, 0x40, 0x40, 0x40, 0x40};
    static const unsigned char glyph_m[5] = {0x7E, 0x02, 0x0C, 0x02, 0x7E};
    static const unsigned char glyph_o[5] = {0x3E, 0x41, 0x41, 0x41, 0x3E};
    static const unsigned char glyph_t[5] = {0x01, 0x01, 0x7F, 0x01, 0x01};
    static const unsigned char glyph_v[5] = {0x1F, 0x20, 0x40, 0x20, 0x1F};

    switch (ch) {
    case '0': return glyph_0;
    case '1': return glyph_1;
    case '2': return glyph_2;
    case '3': return glyph_3;
    case '4': return glyph_4;
    case '5': return glyph_5;
    case '6': return glyph_6;
    case '7': return glyph_7;
    case '8': return glyph_8;
    case '9': return glyph_9;
    case '.': return glyph_dot;
    case ':': return glyph_colon;
    case 'A': return glyph_a;
    case 'C': return glyph_c;
    case 'D': return glyph_d;
    case 'L': return glyph_l;
    case 'M': return glyph_m;
    case 'O': return glyph_o;
    case 'T': return glyph_t;
    case 'V': return glyph_v;
    default: return glyph_space;
    }
}

static void LCD9648_WriteChar(char ch)
{
    const unsigned char *glyph = LCD9648_Glyph(ch);
    unsigned char i;

    for (i = 0U; i < 5U; i++) {
        LCD9648_WriteData(glyph[i]);
    }
    LCD9648_WriteData(0x00);
}

static void LCD9648_WriteLine(Uint16 x, Uint16 page, const char *text)
{
    Uint16 col = x;

    if (page >= LCD9648_PAGES) {
        return;
    }

    LCD9648_SetPos(x, page);
    while ((*text != '\0') && ((col + LCD9648_GLYPH_W) <= LCD9648_WIDTH)) {
        LCD9648_WriteChar(*text);
        text++;
        col += LCD9648_GLYPH_W;
    }
}

static void LCD9648_GPIO_Init(void)
{
    EALLOW;
    SysCtrlRegs.PCLKCR3.bit.GPIOINENCLK = 1;

    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO3 = 0;
    GpioCtrlRegs.GPBPUD.bit.GPIO60 = 0;

    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 0;
    GpioCtrlRegs.GPBMUX2.bit.GPIO60 = 0;

    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO60 = 1;
    EDIS;

    LCD9648_SCL_SETH;
    LCD9648_SDA_SETH;
    LCD9648_RS_SETL;
    LCD9648_CS_SETH;
    LCD9648_RSET_SETH;
}

void LCD9648_Init(void)
{
    LCD9648_GPIO_Init();

    LCD9648_RSET_SETH;
    DELAY_US(10000);
    LCD9648_RSET_SETL;
    DELAY_US(10000);
    LCD9648_RSET_SETH;
    DELAY_US(10000);

    LCD9648_WriteCmd(0xE2);
    LCD9648_WriteCmd(0xC8);
    LCD9648_WriteCmd(0xA0);
    LCD9648_WriteCmd(0x2F);
    LCD9648_WriteCmd(0x26);
    LCD9648_WriteCmd(0x81);
    LCD9648_WriteCmd(0x10);
    LCD9648_WriteCmd(0xAF);

    LCD9648_Clear();
}

void LCD9648_Clear(void)
{
    Uint16 page;
    Uint16 x;

    for (page = 0U; page < LCD9648_PAGES; page++) {
        LCD9648_WriteCmd(0x40);
        LCD9648_SetPos(0U, page);
        for (x = 0U; x < LCD9648_WIDTH; x++) {
            LCD9648_WriteData(0x00);
        }
    }
}

void LCD9648_DisplayVoltageMv(Uint16 mv)
{
    char value[9];
    Uint16 volt = mv / 1000U;
    Uint16 dec = (Uint16)((mv % 1000U) / 10U);

    if (mv > 9990U) {
        mv = 9990U;
        volt = 9U;
        dec = 99U;
    }

    value[0] = (char)('0' + volt);
    value[1] = '.';
    value[2] = (char)('0' + (dec / 10U));
    value[3] = (char)('0' + (dec % 10U));
    value[4] = 'V';
    value[5] = ' ';
    value[6] = ' ';
    value[7] = ' ';
    value[8] = '\0';

    LCD9648_WriteLine(6U, 1U, "ADC VOLT");
    LCD9648_WriteLine(24U, 3U, value);
}
