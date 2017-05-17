/*
 * boxer_light.c
 *
 *  Created on: 21 Apr 2017
 *      Author: dolewdam
 */

#include "boxer_light.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Lightning_Handler(void)
{
	lastLightState = xLightControl.lightingState;
	if (xLightControl.timeOnHours == 0 && xLightControl.timeOffHours == 24)
	{
		GPIOx_ResetPin(LAMP_PORT, LAMP_PIN);
		xLightControl.lightingState = LIGHT_OFF;
		xLightCounters.counterSeconds = 0;
		xLightCounters.counterHours = 0;
	}
	else if (xLightControl.timeOnHours == 24 && xLightControl.timeOffHours == 0)
	{
		GPIOx_SetPin(LAMP_PORT, LAMP_PIN);
		xLightControl.lightingState = LIGHT_ON;
		xLightCounters.counterSeconds = 0;
		xLightCounters.counterHours = 0;
	}
	else
	{
		xLightCounters.counterSeconds++;
		if (xLightCounters.counterSeconds == 3600)
		{
			xLightCounters.counterHours++;
			xLightCounters.counterSeconds = 0;
		}

		switch (xLightControl.lightingState)
		{
		case LIGHT_ON:
			if (xLightControl.timeOnHours != 0 && xLightControl.timeOnHours != 24)
			{
				GPIOx_SetPin(LAMP_PORT, LAMP_PIN);
				if (xLightControl.timeOnHours == xLightCounters.counterHours)
				{
					xLightControl.lightingState = LIGHT_OFF;
					xLightCounters.counterHours = 0;
					xLightCounters.counterSeconds = 0;
				}
			}
			break;

		case LIGHT_OFF:
			if (xLightControl.timeOffHours != 0 && xLightControl.timeOffHours != 24)
			{
				GPIOx_ResetPin(LAMP_PORT, LAMP_PIN);
				if (xLightControl.timeOffHours == xLightCounters.counterHours)
				{
					xLightControl.lightingState = LIGHT_ON;
					xLightCounters.counterHours = 0;
					xLightCounters.counterSeconds = 0;
				}
			}
			break;

		default:
			break;
		}
	}
}
