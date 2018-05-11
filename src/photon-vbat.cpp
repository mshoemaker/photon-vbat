#include "photon-vbat.h"

#include "adc_hal.h"
#include "gpio_hal.h"

PhotonVBAT::PhotonVBAT(int sparePin, float vbatCal, int tempCal) : sparePin(sparePin), vbatCal(vbatCal), tempCal(tempCal) {

}

PhotonVBAT::~PhotonVBAT() {

}

float PhotonVBAT::readVBAT() {
	static bool enabled = false;
	if (!enabled) {
		// If you don't analogRead once, before doing readChannel, you'll sometimes get inaccurate values.
		// for readVBAT and I can't figure out why.
		analogRead(sparePin);
		enabled = true;
		ADC_VBATCmd(ENABLE);
	}

	uint16_t value = readChannel(ADC_Channel_18);
	// Serial.printlnf("value=%d", value);

	// Note: VBAT input goes to 3.6, not 3.3.
	return (float)value * vbatCal / 4095.0;
}

float PhotonVBAT::readTempC() {
	static bool enabled = false;
	if (!enabled) {
		analogRead(sparePin);
		enabled = true;
		ADC_TempSensorVrefintCmd(ENABLE);
	}

	// ADC_Channel_16
	// Temperature (in °C) = {(VSENSE – V25) / Avg_Slope} + 25
	// Where:
	// – V25 = VSENSE value for 25° C
	// – Avg_Slope = average slope of the temperature vs. VSENSE curve (given in mV/°C or μV/°C)

	// From the STM32F20x data sheet, section 6.3.22, p. 129
	// Avg_Slope = 2.5 mV/deg C
	// V25 = 0.76V = 943 ADC value (0.76*4095/3.3)

	uint16_t vsense = readChannel(ADC_Channel_16);
	return (((float)vsense - (float)tempCal) / 2.5) + 25.0;
}


uint16_t PhotonVBAT::readChannel(uint8_t channel) {
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	// Enable ADC1 clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, ENABLE);

	//ADC_DeInit();

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	// ADC1 configuration
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Init(ADC2, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_480Cycles);

	// Enable ADC1
	ADC_Cmd(ADC1, ENABLE);

	ADC_SoftwareStartConv(ADC1);

	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

	uint16_t value = ADC_GetConversionValue(ADC1);

	// Serial.printlnf("read channel=%d value=%d", channel, value);

	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

	ADC_Cmd(ADC1, DISABLE);

	// This is not really desirable, but system firmware only sets these setting once,
	// and we override some settings, which will break analogRead()
	// Code from adc_hal.c:
	restoreSystemSettings();

	// We need to read a pin other than the one that the actual code is using because
	// system firmware doesn't reset all of the settings if you read the same pin you
	// last read and you'll get inaccurate readings.
	analogRead(sparePin);

	return value;
}


void PhotonVBAT::restoreSystemSettings() {

	// This is more or less copied from
	// https://github.com/particle-iot/firmware/blob/develop/hal/src/stm32f2xx/adc_hal.c#L167

	// The problem is that once these settings are made by the system firmware they don't
	// expect them to change and are not set again. This when we mess with them in order to
	// get the sensor reading (which are only available on ADC1 so we can't used dual mode
	// to read), it screws up the ability to use analogRead() unless we reset the values.

	// Also, don't forget to read a different pin with analogRead() before you read the one
	// you want (if you only read one pin) because that's cached too and you'll get inaccurate
	// values unless you do a fake read first.

	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef ADC_InitStructure;

	// ADC Common Init
	ADC_CommonInitStructure.ADC_Mode = ADC_DualMode_RegSimult;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	// ADC1 and ADC2 configuration
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_Init(ADC2, &ADC_InitStructure);

}
