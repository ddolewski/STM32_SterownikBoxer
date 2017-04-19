/*
 * boxer_climate.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_climate.h"
#include "boxer_timers.h"

static systime_t measureOwireTimer = 0;
static systime_t measureI2cTimer = 0;
static systime_t shtInitTimer = 0;
static systime_t oWireInitTimer = 0;

static ErrorStatus errorSht = SUCCESS;
static ErrorStatus errorTsl = SUCCESS;
static ErrorStatus errorDs1 = SUCCESS;
static ErrorStatus errorDs2 = SUCCESS;
/////////////////////////////////////////////////////////////////////////////
void Climate_SensorsHandler(void)
{
	if (systimeTimeoutControl(&oWireInitTimer, 500))
	{
#ifndef OWIRE_OFF_MODE
		errorDs1 = initializeConversion(&ds18b20_1);
		errorDs2 = initializeConversion(&ds18b20_2);
#endif
	}

	if (systimeTimeoutControl(&measureOwireTimer, 800))
	{
#ifndef OWIRE_OFF_MODE
		errorDs1 = readTemperature(&ds18b20_1);
		displayData.tempDS18B20_1_t = ds18b20_1.fTemp;
		errorDs2 = readTemperature(&ds18b20_2);
		displayData.tempDS18B20_2_t = ds18b20_2.fTemp;
#endif
	}

	if (systimeTimeoutControl(&shtInitTimer, 3000))
	{
#ifndef I2C_OFF_MODE
    	errorSht = SHT21_SoftReset(I2C2, SHT21_ADDR);
#endif
	}

	if (systimeTimeoutControl(&measureI2cTimer, 5000))
	{
#ifndef I2C_OFF_MODE
		displayData.lux = TSL2561_ReadLux(&errorTsl);

        uint16_t tempWord = 0;
        uint16_t humWord = 0;

    	tempWord = SHT21_MeasureTempCommand(I2C2, SHT21_ADDR, &errorSht);
    	humWord = SHT21_MeasureHumCommand(I2C2, SHT21_ADDR, &errorSht);

    	humWord = ((uint16_t)(SHT_HumData.msb_lsb[0]) << 8) | SHT_HumData.msb_lsb[1];
    	tempWord = ((uint16_t)(SHT_TempData.msb_lsb[0]) << 8) | SHT_TempData.msb_lsb[1];

    	displayData.tempSHT2x = SHT21_CalcTemp(tempWord);
    	displayData.humiditySHT2x = SHT21_CalcRH(humWord);
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
				if (ds18b20_1.fTemp > (float)tempControl.userTemp)
				{
					//USARTx_SendString(USART_COMM, UC"fTemp > userTemp\n\r");
					PWM_IncPercentTo(PWM_FAN_PULL_AIR, 100, PWM_CHANGE_SLOW); 	//wyciagajacy
					lastPullPWM = 100;
					PWM_IncPercentTo(PWM_FAN_PUSH_AIR, 70, PWM_CHANGE_SLOW); 	//wciagajacy
					lastPushPWM = 70;
				}
				else
				{
					//USARTx_SendString(USART_COMM, UC"fTemp < userTemp\n\r");
					PWM_DecPercentTo(PWM_FAN_PULL_AIR, 60, PWM_CHANGE_SLOW);
					lastPullPWM = 60;
					PWM_DecPercentTo(PWM_FAN_PUSH_AIR, 30, PWM_CHANGE_SLOW);
					lastPushPWM = 30;
				}
			}
			else
			{
	//			_printParam(UC"LightControl.LightingState", LightControl.LightingState);

				if (lastPullPWM > 40)
				{
					if (PWM_DecPercentTo(PWM_FAN_PULL_AIR, 40, PWM_CHANGE_SLOW) == 1)
					{
						lastPullPWM = 40;
					}
				}
				else
				{
					if (PWM_IncPercentTo(PWM_FAN_PULL_AIR, 40, PWM_CHANGE_SLOW) == 1)
					{
						lastPullPWM = 40;
					}
				}

				if (lastPushPWM > 30)
				{
					if (PWM_DecPercentTo(PWM_FAN_PUSH_AIR, 30, PWM_CHANGE_SLOW) == 1)
					{
						lastPushPWM = 30;
					}
				}
				else
				{
					if (PWM_IncPercentTo(PWM_FAN_PUSH_AIR, 30, PWM_CHANGE_SLOW) == 1)
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
					if (PWM_DecPercentTo(PWM_FAN_PULL_AIR, tempControl.fanPull, PWM_CHANGE_SLOW) == 1)
					{
						lastPullPWM = tempControl.fanPull;
					}
				}
				else
				{
					if (PWM_IncPercentTo(PWM_FAN_PULL_AIR, tempControl.fanPull, PWM_CHANGE_SLOW) == 1)
					{
						lastPullPWM = tempControl.fanPull;
					}
				}
			}

			if (lastPushPWM != tempControl.fanPush)
			{
				if (lastPushPWM > tempControl.fanPush)
				{
					if (PWM_DecPercentTo(PWM_FAN_PUSH_AIR, tempControl.fanPush, PWM_CHANGE_SLOW) == 1)
					{
						lastPushPWM = tempControl.fanPush;
					}
				}
				else
				{
					if (PWM_IncPercentTo(PWM_FAN_PUSH_AIR, tempControl.fanPush, PWM_CHANGE_SLOW) == 1)
					{
						lastPushPWM = tempControl.fanPush;
					}
				}
			}
	    }
	}
}
