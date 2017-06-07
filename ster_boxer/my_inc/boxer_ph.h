#ifndef MY_INC_BOXER_PH_H_
#define MY_INC_BOXER_PH_H_

#include "stm32f0xx_dma.h"
#include "stm32f0xx_adc.h"
#include "stm32f0xx_gpio.h"
#include "system_gpio.h"
#include "boxer_irrigation.h"

#define ADC1_DR_Address 	0x40012440
#define VREFINT_CAL 		(*(uint16_t *)0x1ffff7ba)

typedef struct pH_t
{
	float soil;
	float water;
}pH_t;

typedef struct
{
	float waterFactor_A;
	float waterFactor_B;
	float soilFactor_A;
	float soilFactor_B;
}ph_factors_t;

typedef enum
{
	PROBE_SOIL = 1,
	PROBE_WATER
}probe_type_t;

typedef enum
{
	BUFFER_PH_NONE = 0,
	BUFFER_PH4 = 1,
	BUFFER_PH7 = 2,
	BUFFER_PH9 = 3
}buffer_type_t;

typedef struct calibrationProcess_t
{
	uint8_t processActive;
	uint8_t waitForNextBuffer;
	uint8_t waitCounter;
	uint8_t measureVoltagePh;
	uint8_t pHBufferChooser;
	uint8_t pH4Buffer;
	uint8_t pH7Buffer;
	uint8_t	pH9Buffer;
	uint8_t meanpHCounter;
	probe_type_t probeType;
}calibrationProcess_t;

typedef struct probe_t
{
	float tempSoil;
	float inAverageSoil;
	float inSoil;
	float tempWater;
	float inAverageWater;
	float inWater;
}probe_adc_t;

extern pH_t pH;
extern uint8_t adcAverageMeasCounter;

volatile ph_factors_t xFactorsEquationpH;
volatile calibrationProcess_t calibrateFlags;
probe_adc_t probeData;

float xLastWaterPh;
float xLastSoilPh;

void ADC_DMA_Init(void);
void ADC_CalibrateProbes_Handler(void);
void PhProccess_Handler(void);

#endif /* MY_INC_BOXER_PH_H_ */
