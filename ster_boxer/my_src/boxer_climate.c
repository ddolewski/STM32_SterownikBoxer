/*
 * boxer_climate.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_climate.h"
#include "boxer_timers.h"
#include "boxer_display.h"
#include "boxer_light.h"
#include "hardware/TSL2561/tsl2561.h"

static systime_t measureOwireTimer = 0;
static systime_t luxMeasureTimer = 0;
static systime_t shtMeasurementTimer = 0;
static bool_t oneWireResetDone = FALSE;

static ErrorStatus errorSht = SUCCESS;
static ErrorStatus errorTsl = SUCCESS;
static ErrorStatus errorDsUp = SUCCESS;
static ErrorStatus errorDsDown = SUCCESS;

float lastTempUp = 0;
float lastTempDown = 0;
float lastTempMiddle = 0;
float lastHumidity = 0;
uint32_t lastLux = 0;
/////////////////////////////////////////////////////////////////////////////
void Climate_SensorsHandler(void)
{
	if (oneWireResetDone == FALSE)
	{
	#ifndef OWIRE_OFF_MODE
			errorDsUp = initializeConversion(&sensorTempUp);
			errorDsDown = initializeConversion(&sensorTempDown);
	#endif
			oneWireResetDone = TRUE;

#ifdef ONE_WIRE_LOGS
//		if (errorDsUp == 0)
//		{
//			_error("ds up init error");
//		}
//		else
//		{
//			_printString("ds up init ok\r\n");
//		}

//		if (errorDsDown == 0)
//		{
//			_error("ds down init error");
//		}
//		else
//		{
//			_printString("ds dpwn init ok\r\n");
//		}
#endif
	}

	if (oneWireResetDone == TRUE)
	{
		if (systimeTimeoutControl(&measureOwireTimer, 760))
		{
#ifndef OWIRE_OFF_MODE
			errorDsUp = readTemperature(&sensorTempUp);
			lastTempUp = displayData.temp_up_t;
			displayData.temp_up_t = sensorTempUp.fTemp;

			errorDsDown = readTemperature(&sensorTempDown);
			lastTempDown = displayData.temp_down_t;
			displayData.temp_down_t = sensorTempDown.fTemp;
#endif

#ifdef ONE_WIRE_LOGS
			char tempString[5] = {0};
			if (errorDsUp == 0)
			{
				_error("ds up measure error");
			}
			else
			{
//				_printString("ds up measure ok\r\n");
				ftoa(displayData.temp_up_t, tempString, 1);
				_printString(tempString);
				_printString("\r\n");
			}

			if (errorDsDown == 0)
			{
				_error("ds down measure error");
			}
			else
			{
//				_printString("ds dpwn measure ok\r\n");
				ftoa(displayData.temp_down_t, tempString, 1);
				_printString(tempString);
				_printString("\r\n");
			}
#endif
			oneWireResetDone = FALSE;
		}
	}
////////////////////////////////////////////////////////////////////////////////////////////////////
	if (systimeTimeoutControl(&shtMeasurementTimer, 3000))
	{
#ifndef I2C_OFF_MODE
		errorSht = SHT21_SoftReset(I2C2, SHT21_ADDR);
#ifdef I2C2_LOGS
		if (errorSht == ERROR)
		{
			_error("SHT21 reset error (loop)");
		}
		else
		{
			_printString("SHT21 reset ok (loop)\r\n");
		}
#endif
		uint16_t tempWord = 0;
		uint16_t humWord = 0;

		systimeDelayMs(20);
		tempWord = SHT21_MeasureTempCommand(I2C2, SHT21_ADDR, &errorSht);
#ifdef I2C2_LOGS
		if (errorSht == ERROR)
		{
			_error("SHT21 meas temp error (loop)");
		}
		else
		{
			_printString("SHT21 meas temp ok (loop)\r\n");
		}
#endif
		humWord = SHT21_MeasureHumCommand(I2C2, SHT21_ADDR, &errorSht);
#ifdef I2C2_LOGS
		if (errorSht == ERROR)
		{
			_error("SHT21 meas hum error (loop)");
		}
		else
		{
			_printString("SHT21 meas hum ok (loop)\r\n");
		}
#endif
		humWord = ((uint16_t)(SHT_HumData.msb_lsb[0]) << 8) | SHT_HumData.msb_lsb[1];
		tempWord = ((uint16_t)(SHT_TempData.msb_lsb[0]) << 8) | SHT_TempData.msb_lsb[1];

		lastTempMiddle = displayData.temp_middle_t;
		displayData.temp_middle_t = SHT21_CalcTemp(tempWord);

		lastHumidity = displayData.humiditySHT2x;
		displayData.humiditySHT2x = SHT21_CalcRH(humWord);
#endif
	}
////////////////////////////////////////////////////////////////////////////////////////////////////
	if (systimeTimeoutControl(&luxMeasureTimer, 5000))
	{
#ifndef I2C_OFF_MODE
		lastLux = displayData.lux;
		displayData.lux = TSL2561_ReadLux(&errorTsl);
#ifdef I2C2_LOGS
		if (errorTsl == ERROR)
		{
			_error("TSL2561 read lux error (loop)");
		}
		else
		{
			_printString("TSL2561 read lux ok (loop)\r\n");
		}
#endif
#endif
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Climate_TempCtrl_Handler(void)
{
	if ((TIM3->CR1 & TIM_CR1_CEN) == SET)
	{
	    if (tempControl.tempCtrlMode == TEMP_AUTO) //sterowanie temperatura maksymalna
		{
	//    	_printParam(UC"userParam.tempControl", userParam.tempControl);
			if (xLightControl.lightingState == LIGHT_ON)
			{
	//			_printParam(UC"LightControl.LightingState", LightControl.LightingState);
				if (sensorTempUp.fTemp > (float)tempControl.userTemp)
				{
					//USARTx_SendString(USART_COMM, UC"fTemp > userTemp\n\r");
					PWM_IncPercentTo(PWM_FAN_PULL_AIR, 100);//, PWM_CHANGE_SLOW); 	//wyciagajacy
					lastPullPWM = 100;
					PWM_IncPercentTo(PWM_FAN_PUSH_AIR, 70);//, PWM_CHANGE_SLOW); 	//wciagajacy
					lastPushPWM = 70;
				}
				else
				{
					//USARTx_SendString(USART_COMM, UC"fTemp < userTemp\n\r");
					PWM_DecPercentTo(PWM_FAN_PULL_AIR, 60);//, PWM_CHANGE_SLOW);
					lastPullPWM = 60;
					PWM_DecPercentTo(PWM_FAN_PUSH_AIR, 30);//, PWM_CHANGE_SLOW);
					lastPushPWM = 30;
				}
			}
			else
			{
	//			_printParam(UC"LightControl.LightingState", LightControl.LightingState);

				if (lastPullPWM > 40)
				{
					if (PWM_DecPercentTo(PWM_FAN_PULL_AIR, 40))//, 40, PWM_CHANGE_SLOW) == 1)
					{
						lastPullPWM = 40;
					}
				}
				else
				{
					if (PWM_IncPercentTo(PWM_FAN_PULL_AIR, 40))//, 40, PWM_CHANGE_SLOW) == 1)
					{
						lastPullPWM = 40;
					}
				}

				if (lastPushPWM > 30)
				{
					if (PWM_DecPercentTo(PWM_FAN_PUSH_AIR, 30))//, 30, PWM_CHANGE_SLOW) == 1)
					{
						lastPushPWM = 30;
					}
				}
				else
				{
					if (PWM_IncPercentTo(PWM_FAN_PUSH_AIR, 30))//, 30, PWM_CHANGE_SLOW) == 1)
					{
						lastPushPWM = 30;
					}
				}
			}
		}
	    else if (tempControl.tempCtrlMode == TEMP_MANUAL)
	    {
	    	if (lastPullPWM != tempControl.fanPull)
			{
				if (lastPullPWM > tempControl.fanPull)
				{
					if (PWM_DecPercentTo(PWM_FAN_PULL_AIR, tempControl.fanPull))//, PWM_CHANGE_SLOW) == 1)
					{
						lastPullPWM = tempControl.fanPull;
					}
				}
				else
				{
					if (PWM_IncPercentTo(PWM_FAN_PULL_AIR, tempControl.fanPull))//, PWM_CHANGE_SLOW) == 1)
					{
						lastPullPWM = tempControl.fanPull;
					}
				}
			}

			if (lastPushPWM != tempControl.fanPush)
			{
				if (lastPushPWM > tempControl.fanPush)
				{
					if (PWM_DecPercentTo(PWM_FAN_PUSH_AIR, tempControl.fanPush))//, PWM_CHANGE_SLOW) == 1)
					{
						lastPushPWM = tempControl.fanPush;
					}
				}
				else
				{
					if (PWM_IncPercentTo(PWM_FAN_PUSH_AIR, tempControl.fanPush))//, PWM_CHANGE_SLOW) == 1)
					{
						lastPushPWM = tempControl.fanPush;
					}
				}
			}
	    }
	}
}
