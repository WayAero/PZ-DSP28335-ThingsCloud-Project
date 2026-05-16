/*
 * lcd9648.h
 */

#ifndef LCD9648_H_
#define LCD9648_H_

#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#define LCD9648_SCL_SETH    (GpioDataRegs.GPASET.bit.GPIO0 = 1)
#define LCD9648_SCL_SETL    (GpioDataRegs.GPACLEAR.bit.GPIO0 = 1)

#define LCD9648_SDA_SETH    (GpioDataRegs.GPBSET.bit.GPIO60 = 1)
#define LCD9648_SDA_SETL    (GpioDataRegs.GPBCLEAR.bit.GPIO60 = 1)

#define LCD9648_RS_SETH     (GpioDataRegs.GPASET.bit.GPIO1 = 1)
#define LCD9648_RS_SETL     (GpioDataRegs.GPACLEAR.bit.GPIO1 = 1)

#define LCD9648_CS_SETH     (GpioDataRegs.GPASET.bit.GPIO2 = 1)
#define LCD9648_CS_SETL     (GpioDataRegs.GPACLEAR.bit.GPIO2 = 1)

#define LCD9648_RSET_SETH   (GpioDataRegs.GPASET.bit.GPIO3 = 1)
#define LCD9648_RSET_SETL   (GpioDataRegs.GPACLEAR.bit.GPIO3 = 1)

void LCD9648_Init(void);
void LCD9648_Clear(void);
void LCD9648_DisplayVoltageMv(Uint16 mv);

#endif
