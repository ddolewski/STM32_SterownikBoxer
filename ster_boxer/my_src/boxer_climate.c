/*
 * boxer_climate.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_climate.h"
#include "boxer_timers.h"

static systime_t speedTimer = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClimateTempControl_Handler(DS18B20Sensor_t * ds18b20_2)
{
	if (systimeTimeoutControl(&speedTimer, 100))
	{
		flagsGlobal.increaseSpeedFlag = TRUE;
	}

    if ((tempControl.tempControl == TEMP_AUTO) && (gFansSoftStartFlag == FALSE)) //sterowanie temperatura maksymalna
	{
//    	_printParam(UC"userParam.tempControl", userParam.tempControl);
		if (xLightControl.lightingState == LIGHT_ON)
		{
//			_printParam(UC"LightControl.LightingState", LightControl.LightingState);
			if (flagsGlobal.increaseSpeedFlag == TRUE) //powoduje zwiekszanie PWM'a co sekunde
			{
				if (ds18b20_2->fTemp > (float)tempControl.userTemp)
				{
					//USARTx_SendString(USART_COMM, UC"fTemp > userTemp\n\r");

					PWM_IncPercentTo(PWM_FAN_PULL_AIR, 95); //wyciagajacy
					PWM_IncPercentTo(PWM_FAN_PUSH_AIR, 70); //wciagajacy
				}
				else
				{
					//USARTx_SendString(USART_COMM, UC"fTemp < userTemp\n\r");
					PWM_DecPercentTo(PWM_FAN_PULL_AIR, 60);
					PWM_DecPercentTo(PWM_FAN_PUSH_AIR, 40);
				}

				flagsGlobal.increaseSpeedFlag = FALSE;
			}
		}
		else
		{
//			_printParam(UC"LightControl.LightingState", LightControl.LightingState);
			PWM_SetPercent(PWM_FAN_PULL_AIR, 50);
			PWM_SetPercent(PWM_FAN_PUSH_AIR, 25);
		}
	}
    else if (tempControl.tempControl == TEMP_MANUAL)
    {
    	//TODO manualne sterowanie
    }
}
