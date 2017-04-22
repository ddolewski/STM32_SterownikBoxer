/*
 * boxer_irrigation.c
 *
 *  Created on: 30 lip 2015
 *      Author: Doles
 */

#include "boxer_irrigation.h"
#include "boxer_timers.h"
#include "boxer_bool.h"

const uint32_t SOIL_MEASURE_TIMEOUT = 300000;
const uint32_t DAY_IN_SEC = 86400;

static systime_t waterLvlCheckTimer = 0;
static systime_t soilMoistTimer = 0;
static uint32_t SecondIrr;
static uint8_t DayIrr;


static bool_t pumpOn = FALSE;


soil_moist_t soilMoisture = SOIL_UNKNOWN_STATE;
soil_moist_t lastSoilMoistState = SOIL_UNKNOWN_STATE;

static void Irrigation_WaterLevel(void);
static void Irrigation_SoilMoisture(void);
static uint8_t Irrigation_PumpEnable(bool_t xStatus);
/////////////////////////////////////////////////////////////////////////////
irrigate_control_t irrigationControl;
void Irrigation_Handler(void)
{
	Irrigation_PumpControll();
	Irrigation_WaterLevel();
	Irrigation_SoilMoisture();
	Irrigation_Core();
}
/////////////////////////////////////////////////////////////////////////////
void Irrigation_PumpControll(void)
{
	if (pumpOn == TRUE)
	{
		Irrigation_PumpEnable(TRUE); //slowly turn on the pump
	}
	else
	{
		Irrigation_PumpEnable(FALSE); //slowly turn off the pump
	}
}
/////////////////////////////////////////////////////////////////////////////
static void Irrigation_WaterLevel(void)
{
	if (systimeTimeoutControl(&waterLvlCheckTimer, 1000))
	{
		if (GPIOx_ReadInputPin(WATER_LEVEL_PORT, WATER_LEVEL_PIN))
		{
//			USARTx_SendString(USART_COMM, UC"poziom wody odpowiedni\n\r");
			GPIOx_ResetPin(SOLENOID_PORT, SOLENOID_PIN);
		}
		else
		{
//			USARTx_SendString(USART_COMM, UC"dolewanie wody\n\r");
			GPIOx_SetPin(SOLENOID_PORT, SOLENOID_PIN);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
static void Irrigation_SoilMoisture(void)
{
	if (systimeTimeoutControl(&soilMoistTimer, SOIL_MEASURE_TIMEOUT)) //co 5 min
	{
		Irrigation_CheckSoilMoisture();
	}
}
/////////////////////////////////////////////////////////////////////////////
void Irrigation_Core(void)
{
	if (pumpOn == FALSE)
	{
		switch (irrigationControl.mode)
		{
			case IRRIGATION_MODE_AUTO_TIME:
				if (irrigationControl.frequency != 0)
				{
					SecondIrr++;
					if(SecondIrr == DAY_IN_SEC)	// 24 * 3600s = 1 day [s]
					{
						DayIrr++;
						SecondIrr = 0;
						if (irrigationControl.frequency == DayIrr)
						{
							pumpOn = TRUE;
							DayIrr = 0;
							SecondIrr = 0;
						}
					}
				}
			break;

			case IRRIGATION_MODE_AUTO_SOIL:
				if (soilMoisture == SOIL_WET || soilMoisture == SOIL_UNKNOWN_STATE) //stan wysoki
				{
					pumpOn = FALSE;
				}
				else
				{
					pumpOn = TRUE;
				}
			break;

			case IRRIGATION_MODE_MANUAL:
				pumpOn = TRUE;
				irrigationControl.mode = IRRIGATION_MODE_OFF;
			break;

			default:
				irrigationControl.mode = IRRIGATION_MODE_OFF;
				break;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
static uint8_t Irrigation_PumpEnable(bool_t xStatus)
{
	uint8_t ret = 0;

	if (xStatus)
	{
		ret = PWM_IncPercentTo(PWM_PUMP, 100);//, PWM_CHANGE_FAST);
	}
	else
	{
		ret = PWM_DecPercentTo(PWM_PUMP, 0);//, PWM_CHANGE_FAST);
	}

	return ret;
}

void Irrigation_CheckSoilMoisture(void)
{
	//wlaczam na chwile +5V na sonde i robie pomiar wilgotnosci
	//po wykonanym pomiarze wylaczam sonde aby nie rdzewiala
	GPIOx_SetPin(SOIL_MOIST_EN, SOIL_MOIST_EN_PIN);
	systimeDelayMs(10);
	lastSoilMoistState = soilMoisture;
	if (GPIOx_ReadInputPin(SOIL_MOIST_PORT, SOIL_MOIST_PIN)) //stan wysoki
	{
		soilMoisture = SOIL_DRY;
	}
	else
	{
		soilMoisture = SOIL_WET;
	}

	GPIOx_ResetPin(SOIL_MOIST_EN, SOIL_MOIST_EN_PIN);
}
