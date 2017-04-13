/*
 * boxer_climate.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_climate.h"
#include "boxer_timers.h"

static uint8_t lastFanPull = 0;
static uint8_t lastFanPush = 0;

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
	if (softStartPWM == SOFT_START_NONE)
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
					PWM_IncPercentTo(PWM_FAN_PULL_AIR, 100); //wyciagajacy
					PWM_IncPercentTo(PWM_FAN_PUSH_AIR, 70); //wciagajacy
				}
				else
				{
					//USARTx_SendString(USART_COMM, UC"fTemp < userTemp\n\r");
					PWM_DecPercentTo(PWM_FAN_PULL_AIR, 60);
					PWM_DecPercentTo(PWM_FAN_PUSH_AIR, 30);
				}
			}
			else
			{
	//			_printParam(UC"LightControl.LightingState", LightControl.LightingState);
				PWM_SetPercent(PWM_FAN_PULL_AIR, 40);
				PWM_SetPercent(PWM_FAN_PUSH_AIR, 20);
			}
		}
	    else if (tempControl.tempCtrlMode == TEMP_MANUAL)
	    {
	    	if (lastFanPull != tempControl.fanPull)
			{
				PWM_SetPercent(PWM_FAN_PULL_AIR, tempControl.fanPull);
			}

			if (lastFanPush != tempControl.fanPush)
			{
				PWM_SetPercent(PWM_FAN_PUSH_AIR, tempControl.fanPush);
			}

	    	lastFanPull = tempControl.fanPull;
	    	lastFanPush = tempControl.fanPush;
	    }
	}
}
