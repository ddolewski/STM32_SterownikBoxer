/*
 * boxer_irrigation.c
 *
 *  Created on: 30 lip 2015
 *      Author: Doles
 */

#include "boxer_irrigation.h"
#include "boxer_timers.h"

static systime_t waterLvlCheckTimer = 0;
static systime_t softStartTimer = 0;
static systime_t soilMoistTimer = 0;
static uint32_t SecondIrr;
static uint8_t DayIrr;
static uint8_t pumpCounter;
static uint8_t flagTurnOnPump;
static uint8_t flagSoftStartOff;
static uint8_t WaterToSecond;
soil_moist_t soilMoisture = SOIL_UNKNOWN_STATE;
soil_moist_t lastSoilMoistState = SOIL_UNKNOWN_STATE;

static void Irrigation_WaterLevel(void);
static void Irrigation_PumpControll(void);
static void Irrigation_SoilMoisture_Handler(void);
static uint8_t Irrigation_PumpSoftStart(bool_t xStatus);
/////////////////////////////////////////////////////////////////////////////
irrigate_control_t irrigationControl;
void Irrigation_Handler(void)
{
//	Irrigation_PumpControll();
//	Irrigation_WaterLevel();
	Irrigation_SoilMoisture_Handler();
}
/////////////////////////////////////////////////////////////////////////////
static void Irrigation_PumpControll(void)
{
	if (flagTurnOnPump == 1)
	{
		if (pumpCounter == WaterToSecond)
		{
			flagSoftStartOff = 1;
			WaterToSecond = 0;
		}

		if (flagSoftStartOff)
		{
			uint8_t state = Irrigation_PumpSoftStart(FALSE);

			if (state)
			{
				flagTurnOnPump = 0;
				pumpCounter = 0;
				flagSoftStartOff = 0;
			}
		}
		else
		{
			Irrigation_PumpSoftStart(TRUE);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
static void Irrigation_WaterLevel(void)
{
	if (systimeTimeoutControl(&waterLvlCheckTimer, 250))
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
static void Irrigation_SoilMoisture_Handler(void)
{
	if (systimeTimeoutControl(&soilMoistTimer, 60000))
	{
		lastSoilMoistState = soilMoisture;
		if (GPIOx_ReadInputPin(SOIL_MOIST_PORT, SOIL_MOIST_PIN)) //stan wysoki
		{
			soilMoisture = SOIL_DRY;
		}
		else
		{
			soilMoisture = SOIL_WET;
		}
	}
//	else //todo oksydacja sondy
//	{
//
//	}
}
/////////////////////////////////////////////////////////////////////////////
void Irrigation_Core(void)
{
	if (flagTurnOnPump != 1)
	{
		switch (irrigationControl.mode)
		{
			case IRRIGATION_MODE_AUTO_TIME:
				SecondIrr++;			// increment seconds
				if(SecondIrr == 86400)	// 24 * 3600s = 1 day [s]
				{
					DayIrr++;			// increment days
					SecondIrr = 0;		// clear seconds
					if (irrigationControl.frequency == DayIrr)
					{
						WaterToSecond = (irrigationControl.water*10)/100; // iloœæ w ml / 100ml (1s) np.
						flagTurnOnPump = 1;
						flagSoftStartOff = 0;
						DayIrr = 0;	// clear days
					}
				}
			break;

//			case IRRIGATION_MODE_AUTO_SOIL:
//				if (soilMoisture == SOIL_WET || soilMoisture == SOIL_UNKNOWN_STATE) //stan wysoki
//				{
//					flagTurnOnPump = 0;
//					flagSoftStartOff = 0;
//				}
//				else
//				{
//					WaterToSecond = (irrigationControl.water*10)/100; //todo iloœæ w ml / 100ml (1s) np.
//					flagTurnOnPump = 1;
//				}
//			break;

			case IRRIGATION_MODE_MANUAL:
				flagTurnOnPump = 1;
				flagSoftStartOff = 0;
				irrigationControl.mode = 0;
			break;

			default:
				irrigationControl.mode = IRRIGATION_MODE_OFF;
				flagSoftStartOff = 0;
				flagTurnOnPump = 0;
				break;
		}
	}

	if (flagTurnOnPump == 1 && flagSoftStartOff != 1)
	{
		pumpCounter++;
	}
}
/////////////////////////////////////////////////////////////////////////////
static uint8_t Irrigation_PumpSoftStart(bool_t xStatus)
{
	uint8_t ret = 0;
	if (systimeTimeoutControl(&softStartTimer, 20))
	{
		if (xStatus)
		{
			ret = PWM_IncPercentTo(PWM_PUMP, 95);
		}
		else
		{
			ret = PWM_DecPercentTo(PWM_PUMP, 0);
		}
	}

	return ret;
}
