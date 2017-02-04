/*
 * boxer_adc.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_ph.h"
#include <stdio.h>
#include <math.h>

static adcRef_t referenceVoltage;
static probe_adc_t probeData;
static uint8_t adcAverageMeasCounter;
static uint16_t ADC_ConvertedData[3];

ADC_value_t ADC_value;
pH_t pH;
pHBufferVoltage_t pHBufferVoltage;

static volatile uint8_t adcDataReady = FALSE;

#define SOIL_PH_INPUT 	GPIO_Pin_5
#define WATER_PH_INPUT 	GPIO_Pin_6

#define V_REF_BUFF_INDEX 	0
#define SOIL_BUFF_INDEX  	1
#define WATER_BUFF_INDEX 	2

static void ADC_ReadCalcPh(void);
static void ADC_CalibrateProbes_BufferChooser(void);
static void ADC_CalibrateProbess_GetFactorsFromMeasurement(pHBufferVoltage_t * xReferenceBufferVoltages, ph_factors_t * xOutPhFactors, probe_type_t xProbeType);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ADC_DMA_Init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = SOIL_PH_INPUT | WATER_PH_INPUT;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;

	/* ADC1 DeInit */
	ADC_DeInit(ADC1);

	/* ADC1 Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	/* DMA1 clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);

	/* DMA1 Channel1 Config */
	DMA_DeInit(DMA1_Channel1);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_ConvertedData;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 3;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	/* DMA1 Channel1 enable */
	DMA_Cmd(DMA1_Channel1, ENABLE);

	/* DMA1 Channel1 enable transfer complete interrupt */
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

	NVIC_SetPriority(DMA1_Channel1_IRQn, 4);
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);

	/* ADC DMA request in circular mode */
	ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);

	/* Enable ADC_DMA */
	ADC_DMACmd(ADC1, ENABLE);

	/* Initialize ADC structure */
	ADC_StructInit(&ADC_InitStructure);

	/* Configure the ADC1 in continous mode withe a resolutuion equal to 12 bits  */
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* Convert the ADC1 Vref  with 55.5 Cycles as sampling time */
	ADC_ChannelConfig(ADC1, ADC_Channel_5 , ADC_SampleTime_55_5Cycles);
	ADC_ChannelConfig(ADC1, ADC_Channel_6 , ADC_SampleTime_55_5Cycles);
	ADC_ChannelConfig(ADC1, ADC_Channel_Vrefint , ADC_SampleTime_55_5Cycles); //Vref

	ADC_VrefintCmd(ENABLE);

	/* ADC Calibration */
	ADC_GetCalibrationFactor(ADC1);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	uint32_t timeout = 0xFFFFFFF;
	/* Wait the ADCEN falg */
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN))
    {
    	if (timeout > 0)
    	{
    		timeout--;
    	}
    	else
    	{
    		break;
    	}
    }
	/* ADC1 regular Software Start Conv */
	ADC_StartOfConversion(ADC1);
}


void DMA1_Channel1_IRQHandler(void)
{
	/* Test on DMA Transfer Complete interrupt */
	if (DMA_GetITStatus(DMA1_IT_TC1) != RESET)
	{
		/* Clear DMA Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit(DMA1_IT_TC1);
		adcDataReady = TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void ADC_ReadCalcPh(void)
{
    if (adcDataReady == TRUE)
    {
        referenceVoltage.vRefValueADC = ADC_ConvertedData[V_REF_BUFF_INDEX];	//Vref voltage
        ADC_value.soil  = ADC_ConvertedData[SOIL_BUFF_INDEX]; 		//PA5
        ADC_value.water = ADC_ConvertedData[WATER_BUFF_INDEX]; 		//PA6

        referenceVoltage.refVoltage  = ((float)VREFINT_CAL * (float)3.3)/referenceVoltage.vRefValueADC;
        referenceVoltage.mVFactor 	 = referenceVoltage.refVoltage / (float)4096;

    	probeData.inSoil = (float)ADC_value.soil * referenceVoltage.mVFactor;
    	probeData.tempSoil += probeData.inSoil;
    	probeData.inWater = (float)ADC_value.water * referenceVoltage.mVFactor;
    	probeData.tempWater += probeData.inWater;

    	++adcAverageMeasCounter; //todo sprawdzic czy licznik measureADCCounter faktycznie osiaga wartosc 50 probek
    	if(adcAverageMeasCounter >= 250) //srednia z 50 pomiarow
    	{
    		probeData.inAverageSoil = (probeData.tempSoil/250);
    		probeData.inAverageWater = (probeData.tempWater/250);
    		adcAverageMeasCounter = 0;
    		probeData.tempSoil = 0;
    		probeData.tempWater = 0;
    		//////////////////////////////////////////////////////////////////////////
    		if (calibrateFlags.processActive == 0)
    		{
    			if (FactorsEquationpH.soilFactor_A == 0 && FactorsEquationpH.soilFactor_B == 0)
    			{
    				pH.soil = 0;
    			}
    			else
    			{
        			pH.soil  = FactorsEquationpH.soilFactor_A  * probeData.inAverageSoil  + FactorsEquationpH.soilFactor_B;  	//soil pH equation
    			}

    			if (FactorsEquationpH.waterFactor_A == 0 && FactorsEquationpH.waterFactor_B == 0)
    			{

    			}
    			else
    			{
    				pH.water = FactorsEquationpH.waterFactor_A * probeData.inAverageWater + FactorsEquationpH.waterFactor_B;	//water pH equation
    			}
    		}
    		else
    		{
    			pH.soil = 0;
    			pH.water = 0;
    		}
    	}

        adcDataReady = FALSE;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void ADC_CalibrateProbes_BufferChooser(void)
{
	if (calibrateFlags.measureVoltagePh == 1)
	{
		if (calibrateFlags.pH4Buffer == 1)
		{
			if (calibrateFlags.probeType == 1)
			{
				pHBufferVoltage.pH4 = probeData.inAverageSoil;
			}
			else if (calibrateFlags.probeType == 2)
			{
				pHBufferVoltage.pH4 = probeData.inAverageWater;
			}
		}
		else if (calibrateFlags.pH7Buffer == 1)
		{
			if (calibrateFlags.probeType == 1)
			{
				pHBufferVoltage.pH7 = probeData.inAverageSoil;
			}
			else if (calibrateFlags.probeType == 2)
			{
				pHBufferVoltage.pH7 = probeData.inAverageWater;
			}
		}
		else if (calibrateFlags.pH9Buffer == 1)
		{
			if (calibrateFlags.probeType == 1)
			{
				pHBufferVoltage.pH9 = probeData.inAverageSoil;
			}
			else if (calibrateFlags.probeType == 2)
			{
				pHBufferVoltage.pH9 = probeData.inAverageWater;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ADC_CalibrateProbes_Core(void)
{
	if (calibrateFlags.processActive == TRUE)
	{
		if (calibrateFlags.turnOnBuzzer == TRUE)
		{
			if (calibrateFlags.toggleBuzzerState == TRUE)
			{
				GPIOx_ResetPin(BUZZER_PORT, BUZZER_PIN);
				calibrateFlags.toggleBuzzerState = FALSE;
			}
			else
			{
				GPIOx_SetPin(BUZZER_PORT, BUZZER_PIN);
				calibrateFlags.toggleBuzzerState = TRUE;
			}

			calibrateFlags.buzzerCounter++;
			if (calibrateFlags.buzzerCounter == 15) // 30s
			{
				calibrateFlags.buzzerCounter = FALSE;
				calibrateFlags.turnOnBuzzer = FALSE;
				GPIOx_SetPin(BUZZER_PORT, BUZZER_PIN);
				calibrateFlags.measureVoltagePh = TRUE;
				calibrateFlags.pHBufferChooser++;
				__NOP();__NOP();__NOP();

				if (calibrateFlags.pHBufferChooser == BUFFER_PH4)
				{
					calibrateFlags.pH4Buffer = TRUE;
					calibrateFlags.pH7Buffer = FALSE;
					calibrateFlags.pH9Buffer = FALSE;
				}
				else if (calibrateFlags.pHBufferChooser == BUFFER_PH7)
				{
					calibrateFlags.pH4Buffer = FALSE;
					calibrateFlags.pH7Buffer = TRUE;
					calibrateFlags.pH9Buffer = FALSE;
				}
				else if (calibrateFlags.pHBufferChooser == BUFFER_PH9)
				{
					calibrateFlags.pH4Buffer = FALSE;
					calibrateFlags.pH7Buffer = FALSE;
					calibrateFlags.pH9Buffer = TRUE;
				}
			}
		}

		if (calibrateFlags.measureVoltagePh == TRUE)
		{
			calibrateFlags.pHCounter++;
			if (calibrateFlags.pHCounter == 10) // 10s
			{
				calibrateFlags.pHCounter = 0;
				calibrateFlags.measureVoltagePh = FALSE;
				calibrateFlags.turnOnBuzzer = TRUE;
				if (calibrateFlags.pH9Buffer == TRUE)
				{
					ADC_CalibrateProbess_GetFactorsFromMeasurement(&pHBufferVoltage, &FactorsEquationpH, calibrateFlags.probeType); //todo regresja C
					calibrateFlags.calibrateDone = TRUE;

					memset(&calibrateFlags, 0, sizeof(calibrateFlags)); //skasowanie flag
					FLASH_SaveConfiguration();
					GPIOx_SetPin(BUZZER_PORT, BUZZER_PIN);
				}
			}
		}
	}
}

static void ADC_CalibrateProbess_GetFactorsFromMeasurement(pHBufferVoltage_t * xReferenceBufferVoltages, ph_factors_t * xOutPhFactors, probe_type_t xProbeType)
{
	float xx[3] = {0, 0, 0};
	float nr = 0, dr = 0;

	float bufferPhVoltages[3] = {0, 0, 0};
	bufferPhVoltages[0] = xReferenceBufferVoltages->pH4;
	bufferPhVoltages[1] = xReferenceBufferVoltages->pH7;
	bufferPhVoltages[2] = xReferenceBufferVoltages->pH9;

	float bufferPh[3] = {4, 7, 9}; //bufory wzorcowe pH

	double sum_y = 0, sum_xy = 0, sum_x = 0,sum_xx = 0, sum_x2 = 0, slope = 0, intercept = 0;//, reg;
	int i, n = 3;
	for (i = 0; i < n; i++)
	{
		xx[i] = bufferPhVoltages[i] * bufferPhVoltages[i];
	}

	for (i = 0; i < n; i++)
	{
		sum_x += bufferPhVoltages[i];
		sum_y += bufferPh[i];
		sum_xx += xx[i];
		sum_xy += bufferPhVoltages[i] * bufferPh[i];
	}

	nr = (n*sum_xy) - (sum_x*sum_y);
	sum_x2 = sum_x*sum_x;
	dr = (n*sum_xx) - sum_x2;
	slope = nr/dr;
	double fac = pow(10, 2);
	slope = round(slope*fac) / fac;
	intercept = (sum_y - slope * sum_x)/n;
	//reg = intercept + (slope*64);

	switch (xProbeType)
	{
		case PROBE_WATER:
			xOutPhFactors->waterFactor_A = (float)slope;
			xOutPhFactors->waterFactor_B = (float)intercept;
			break;

		case PROBE_SOIL:
			xOutPhFactors->soilFactor_A = (float)slope;
			xOutPhFactors->soilFactor_B = (float)intercept;
			break;

		default:
			break;
	}

//	printf("Slope: %.2f\nIntercept: %.3f\nRegression: %.3f",slope,intercept,reg);
}

void PhMeasurementCalibration_Handler(void)
{
	ADC_ReadCalcPh();
	ADC_CalibrateProbes_BufferChooser();
}
