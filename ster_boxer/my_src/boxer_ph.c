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
static adcRef_t referenceVoltage = {0};
static probe_adc_t probeData = {0};
static uint16_t ADC_ConvertedData[3] = {0};
static pHBufferVoltage_t pHBufferVoltage = {0};

static volatile uint8_t adcDataReady = FALSE;
pH_t pH;

static systime_t adcMeasTimer = 0;
static uint32_t adcAverageMeasCounter = 0;

#define SOIL_PH_INPUT 	GPIO_Pin_5
#define WATER_PH_INPUT 	GPIO_Pin_6

#define V_REF_BUFF_INDEX 	0
#define SOIL_BUFF_INDEX  	1
#define WATER_BUFF_INDEX 	2

static void ADC_ReadCalcPh(void);
static void ADC_CalibrateProbes_BufferChooser(void);
static void ADC_CalibrateProbess_GetFactorsFromMeasurement(pHBufferVoltage_t * xReferenceBufferVoltages, volatile ph_factors_t * xOutPhFactors, volatile probe_type_t xProbeType);

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
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

	NVIC_SetPriority(DMA1_Channel1_IRQn, 2);
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);

	ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);
	ADC_DMACmd(ADC1, ENABLE);

	RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);

	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_ChannelConfig(ADC1, ADC_Channel_5 , ADC_SampleTime_239_5Cycles);
	ADC_ChannelConfig(ADC1, ADC_Channel_6 , ADC_SampleTime_239_5Cycles);
	ADC_ChannelConfig(ADC1, ADC_Channel_Vrefint , ADC_SampleTime_239_5Cycles); //Vref

	ADC_VrefintCmd(ENABLE);
	ADC_GetCalibrationFactor(ADC1);

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

		adcAverageMeasCounter++;
		if (calibrateFlags.processActive == FALSE)
		{
			if (adcAverageMeasCounter == 500)
			{
				adcAverageMeasCounter = 0;
				adcDataReady = TRUE;
				ADC_StopOfConversion(ADC1);
			}
		}
		else
		{
			if (adcAverageMeasCounter == 2500)
			{
				adcAverageMeasCounter = 0;
				adcDataReady = TRUE;
//				calibrateFlags.phBufferAvgDone = TRUE;
				ADC_StopOfConversion(ADC1);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void ADC_ReadCalcPh(void)
{
	if (calibrateFlags.processActive == FALSE)
	{
		if (systimeTimeoutControl(&adcMeasTimer, 60000)) //pomiary pH co 1 min
		{
			ADC_StartOfConversion(ADC1);
		}
	}

    if (adcDataReady == TRUE)
    {
        referenceVoltage.vRefValueADC = ADC_ConvertedData[V_REF_BUFF_INDEX];	//Vref voltage
        ADC_value.soil  = ADC_ConvertedData[SOIL_BUFF_INDEX]; 		//PA5
        ADC_value.water = ADC_ConvertedData[WATER_BUFF_INDEX]; 		//PA6

        referenceVoltage.refVoltage  = ((float)VREFINT_CAL * (float)3.3)/referenceVoltage.vRefValueADC;
        referenceVoltage.mVFactor 	 = referenceVoltage.refVoltage / (float)4096;

    	probeData.inAverageSoil  = (float)ADC_value.soil  * referenceVoltage.mVFactor;
    	probeData.inAverageWater = (float)ADC_value.water * referenceVoltage.mVFactor;

		//////////////////////////////////////////////////////////////////////////
		if (calibrateFlags.processActive == FALSE)
		{
			xLastWaterPh = pH.water;
			xLastSoilPh  = pH.soil;

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
			calibrateFlags.phBufferAvgDone = TRUE;
			pH.soil  = 0;
			pH.water = 0;
		}

        adcDataReady = FALSE;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void ADC_CalibrateProbes_BufferChooser(void)
{
	if (calibrateFlags.phBufferAvgDone == TRUE)
	{
		switch (calibrateFlags.refBuffType)
		{
		case REF_BUFF_PH4:
			switch (calibrateFlags.probeType)
			{
			case PROBE_SOIL:
				pHBufferVoltage.pH4 = probeData.inAverageSoil;
				break;

			case PROBE_WATER:
				pHBufferVoltage.pH4 = probeData.inAverageWater;
				break;

			default:
				break;
			}

			calibrateFlags.waitForBuffer = TRUE;
			break;

		case REF_BUFF_PH7:
			switch (calibrateFlags.probeType)
			{
			case PROBE_SOIL:
				pHBufferVoltage.pH4 = probeData.inAverageSoil;
				break;

			case PROBE_WATER:
				pHBufferVoltage.pH4 = probeData.inAverageWater;
				break;

			default:
				break;
			}

			calibrateFlags.waitForBuffer = TRUE;
			break;

		case REF_BUFF_PH9:
			switch (calibrateFlags.probeType)
			{
			case PROBE_SOIL:
				pHBufferVoltage.pH4 = probeData.inAverageSoil;
				break;

			case PROBE_WATER:
				pHBufferVoltage.pH4 = probeData.inAverageWater;
				break;

			default:
				break;
			}

			calibrateFlags.phBufferVoltageMeas = TRUE;
			break;

		default:
			break;
		}

		calibrateFlags.phBufferAvgDone = FALSE;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ADC_CalibrateProbes_Handler(void)
{
	if (calibrateFlags.processActive == TRUE)
	{
		if (calibrateFlags.waitForBuffer == TRUE)
		{
			//sygnalizacja buzzerem oczekiwania do przygotowania sondy do pomiaru
			//przygotowanie buforu referencyjnego, wypłukanie, wytarcie i umieszczenie
			//w buforze sondy pH
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
			if (calibrateFlags.buzzerCounter == 20) // 20s
			{
				calibrateFlags.buzzerCounter = 0;
				calibrateFlags.waitForBuffer = FALSE;
				GPIOx_SetPin(BUZZER_PORT, BUZZER_PIN);

				switch (calibrateFlags.refBuffType)
				{
				case REF_BUFF_NONE:
					calibrateFlags.refBuffType = REF_BUFF_PH4;
					ADC_StartOfConversion(ADC1);
					break;

				case REF_BUFF_PH4:
					calibrateFlags.refBuffType = REF_BUFF_PH7;
					ADC_StartOfConversion(ADC1);
					break;

				case REF_BUFF_PH7:
					calibrateFlags.refBuffType = REF_BUFF_PH9;
					ADC_StartOfConversion(ADC1);
					break;

				case REF_BUFF_PH9:
					calibrateFlags.refBuffType = REF_BUFF_NONE;
					break;

				default:
					break;
				}


			}
		}

		if (calibrateFlags.phBufferVoltageMeas == TRUE)
		{
			calibrateFlags.phBufferVoltageMeas = FALSE;

			ADC_CalibrateProbess_GetFactorsFromMeasurement(&pHBufferVoltage, &FactorsEquationpH, calibrateFlags.probeType);

			memset(&calibrateFlags, 0, sizeof(calibrateFlags)); //skasowanie flag
			FLASH_SaveConfiguration();
			GPIOx_SetPin(BUZZER_PORT, BUZZER_PIN);
		}
	}
}
////////////////////////////////////////////////////////////////////////////
static void ADC_CalibrateProbess_GetFactorsFromMeasurement(pHBufferVoltage_t * xReferenceBufferVoltages, volatile ph_factors_t * xOutPhFactors, volatile probe_type_t xProbeType)
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
