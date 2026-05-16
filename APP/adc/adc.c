/*
 * adc.c
 *
 *  Created on: 2018-1-29
 *      Author: Administrator
 */

#include "adc.h"




void ADC_Init(void)
{
	// Specific clock setting for this example:
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;    // ADC
	EDIS;

	// Specific clock setting for this example:
	EALLOW;
	SysCtrlRegs.HISPCP.all = ADC_MODCLK;	// HSPCLK = SYSCLKOUT/(2*ADC_MODCLK)
	EDIS;

	InitAdc();  // For this example, init the ADC

	// Specific ADC setup for this example:
	AdcRegs.ADCTRL1.bit.ACQ_PS = ADC_SHCLK;
	AdcRegs.ADCTRL3.bit.ADCCLKPS = ADC_CKPS;
	AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;        // 1  Cascaded mode
	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x0;
	AdcRegs.ADCTRL1.bit.CONT_RUN = 1;       // Setup continuous run
	AdcRegs.ADCMAXCONV.bit.MAX_CONV1 = 0x0;
	AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;
	AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;
	AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 1;

}

static Uint16 s_last_adc_value = 0U;

Uint16 ADC_ReadValue(Uint16 *value)
{
	Uint32 timeout = 60000UL;

	while ((AdcRegs.ADCST.bit.INT_SEQ1 == 0U) && (timeout > 0UL)) {
		timeout--;
	}

	if (timeout == 0UL) {
		AdcRegs.ADCTRL2.bit.RST_SEQ1 = 1;
		AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;
		AdcRegs.ADCTRL2.bit.SOC_SEQ1 = 1;
		*value = s_last_adc_value;
		return 0U;
	}

	AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;
	s_last_adc_value = AdcRegs.ADCRESULT0 >> 4;
	*value = s_last_adc_value;
	return 1U;
}

Uint16 Read_ADCValue(void)
{
	Uint16 value = 0U;
	ADC_ReadValue(&value);
	return value;
}

