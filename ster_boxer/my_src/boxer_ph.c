/*
 * boxer_adc.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_ph.h"
#include "boxer_datastorage.h"
#include <stdio.h>
#include <math.h>
#include "string.h"

typedef struct adcRef_t
{
	float vRefValueADC;
	float refVoltage;
	float mVFactor;
}adcRef_t;

typedef struct ADC_value_t
{
	uint16_t soil;
	uint16_t water;
	soil_moist_t soilMoisture;
	uint16_t waterLevel;
} ADC_value_t;

typedef struct probe_t
{
	float tempSoil;
	float inAverageSoil;
	float inSoil;
	float tempWater;
	float inAverageWater;
	float inWater;
}probe_adc_t;

typedef struct
{
	float pH4;
	float pH7;
	float pH9;
}pHBufferVoltage_t;

typedef enum
{
	BUFFER_PH4 = 1,
	BUFFER_PH7 = 2,
	BUFFER_PH9 = 3
}buffer_type_t;

static ADC_value_t ADC_value = {0};
static adcRef_t referenceVoltage;
probe_adc_t probeData;
static volatile uint8_t adcAverageMeasCounter;
static uint16_t ADC_ConvertedData[3];
pHBufferVoltage_t pHBufferVoltage;

pH_t pH;

#define SOIL_PH_INPUT 	GPIO_Pin_5
#define WATER_PH_INPUT 	GPIO_Pin_6

#define V_REF_BUFF_INDEX 	0
#define SOIL_BUFF_INDEX  	1
#define WATER_BUFF_INDEX 	2

static void ADC_ReadCalcPh(void);
static void ADC_CalibrateProbes_BufferChooser(void);
static void ADC_CalibrateProbess_GetFactorsFromMeasurement(volatile pHBufferVoltage_t * xReferenceBufferVoltages, volatile ph_factors_t * xOutPhFactors, probe_type_t xProbeType);

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

	ADC_DeInit(ADC1);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);
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

	DMA_Cmd(DMA1_Channel1, ENABLE);

	ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);
	ADC_DMACmd(ADC1, ENABLE);

	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_ChannelConfig(ADC1, ADC_Channel_5 , ADC_SampleTime_55_5Cycles);
	ADC_ChannelConfig(ADC1, ADC_Channel_6 , ADC_SampleTime_55_5Cycles);
	ADC_ChannelConfig(ADC1, ADC_Channel_Vrefint , ADC_SampleTime_55_5Cycles); //Vref

	ADC_VrefintCmd(ENABLE);
	ADC_GetCalibrationFactor(ADC1);

	ADC_Cmd(ADC1, ENABLE);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY));

	/* ADC DMA request in circular mode */
	ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);

	/* Enable ADC_DMA */
	ADC_DMACmd(ADC1, ENABLE);

	/* Enable the ADC peripheral */
	ADC_Cmd(ADC1, ENABLE);

	/* Wait the ADRDY flag */
	uint32_t timeout = 0xFFFFFFF;
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY))
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void ADC_ReadCalcPh(void)
{
	uint32_t timeout = 0xFFFFFFF;
	while (DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET)
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
    /* Clear DMA TC flag */
    DMA_ClearFlag(DMA1_FLAG_TC1);

	referenceVoltage.vRefValueADC = ADC_ConvertedData[V_REF_BUFF_INDEX];	//Vref voltage
	ADC_value.soil  = ADC_ConvertedData[SOIL_BUFF_INDEX]; 		//PA5
	ADC_value.water = ADC_ConvertedData[WATER_BUFF_INDEX]; 		//PA6

	referenceVoltage.refVoltage  = ((float)VREFINT_CAL * (float)3.3)/referenceVoltage.vRefValueADC;
	referenceVoltage.mVFactor 	 = referenceVoltage.refVoltage / (float)4096;

	probeData.inSoil = (float)ADC_value.soil * referenceVoltage.mVFactor;
	probeData.tempSoil += probeData.inSoil;
	probeData.inWater = (float)ADC_value.water * referenceVoltage.mVFactor;
	probeData.tempWater += probeData.inWater;

	++adcAverageMeasCounter;
	if (adcAverageMeasCounter == 250)
	{
		probeData.inAverageSoil  = (probeData.tempSoil/250);
		probeData.inAverageWater = (probeData.tempWater/250);
		adcAverageMeasCounter = 0;
		probeData.tempSoil = 0;
		probeData.tempWater = 0;
		//////////////////////////////////////////////////////////////////////////
		if (calibrateFlags.processActive == FALSE)
		{
			xLastWaterPh = pH.water;
			xLastSoilPh = pH.soil;

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
				pH.water = 0;
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
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void ADC_CalibrateProbes_BufferChooser(void)
{
	if (calibrateFlags.measureVoltagePh == TRUE)
	{
		if (calibrateFlags.pH4Buffer == TRUE)
		{
			if (calibrateFlags.probeType == PROBE_SOIL)
			{
				pHBufferVoltage.pH4 = probeData.inAverageSoil;
			}
			else if (calibrateFlags.probeType == PROBE_WATER)
			{
				pHBufferVoltage.pH4 = probeData.inAverageWater;
			}
		}
		else if (calibrateFlags.pH7Buffer == TRUE)
		{
			if (calibrateFlags.probeType == PROBE_SOIL)
			{
				pHBufferVoltage.pH7 = probeData.inAverageSoil;
			}
			else if (calibrateFlags.probeType == PROBE_WATER)
			{
				pHBufferVoltage.pH7 = probeData.inAverageWater;
			}
		}
		else if (calibrateFlags.pH9Buffer == TRUE)
		{
			if (calibrateFlags.probeType == PROBE_SOIL)
			{
				pHBufferVoltage.pH9 = probeData.inAverageSoil;
			}
			else if (calibrateFlags.probeType == PROBE_WATER)
			{
				pHBufferVoltage.pH9 = probeData.inAverageWater;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ADC_CalibrateProbes_Handler(void)
{
	if (calibrateFlags.processActive == TRUE)
	{
		if (calibrateFlags.waitForNextBuffer == TRUE)
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
				calibrateFlags.waitForNextBuffer = FALSE;
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
			if (calibrateFlags.pHCounter == 5) // 5s
			{
				calibrateFlags.pHCounter = 0;
				calibrateFlags.measureVoltagePh = FALSE;
				calibrateFlags.waitForNextBuffer = TRUE;

				if (calibrateFlags.pH9Buffer == TRUE)
				{
					if ((pHBufferVoltage.pH4 != 0 && pHBufferVoltage.pH7 != 0 && pHBufferVoltage.pH9 != 0) ||
						(FactorsEquationpH.soilFactor_A == 0 && FactorsEquationpH.soilFactor_B == 0 &&
						 FactorsEquationpH.waterFactor_A == 0 && FactorsEquationpH.waterFactor_B == 0))
					{
						ADC_CalibrateProbess_GetFactorsFromMeasurement(
								&pHBufferVoltage,
								&FactorsEquationpH,
								calibrateFlags.probeType);

						FLASH_SaveConfiguration();
						GPIOx_SetPin(BUZZER_PORT, BUZZER_PIN);
					}

					memset(&calibrateFlags, 0, sizeof(calibrateFlags)); //skasowanie flag
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////
static void ADC_CalibrateProbess_GetFactorsFromMeasurement(volatile pHBufferVoltage_t * xReferenceBufferVoltages, volatile ph_factors_t * xOutPhFactors, probe_type_t xProbeType)
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
}
////////////////////////////////////////////////////////////////////////////
void PhProccess_Handler(void)
{
	ADC_ReadCalcPh();
	ADC_CalibrateProbes_BufferChooser();
}
