/*
 * boxer_ph.h
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_PH_H_
#define MY_INC_BOXER_PH_H_

#include "stm32f0xx_dma.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_gpio.h"
#include "system_gpio.h"
#include "boxer_struct.h"
#include "global.h"

#define ADC1_DR_Address 	0x40012440
#define VREFINT_CAL 		(*(uint16_t *)0x1ffff7ba)

extern ADC_value_t ADC_value;
extern pH_t pH;
volatile ph_factors_t FactorsEquationpH;

extern pHBufferVoltage_t pHBufferVoltage;
volatile calibrationProcess_t calibrateFlags;

void ADC_DMA_Init(void);
void ADC_CalibrateProbes_Handler(void);
void PhMeasurementCalibration_Handler(void);

#endif /* MY_INC_BOXER_PH_H_ */
