#include "boxer_light.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////
void Lightning_Handler(void)
{
	lastLightState = xLightControl.lightingState;

	if ((xLightControl.timeOnHours  != 0 && xLightControl.timeOnHours  != 24) &&
		(xLightControl.timeOffHours != 0 && xLightControl.timeOffHours != 24))
	{
		xLightControl.counterSeconds++;
		if (xLightControl.counterSeconds == 3600)
		{
			xLightControl.counterHours++;
			xLightControl.counterSeconds = 0;
		}

		switch (xLightControl.lightingState)
		{
		case LIGHT_ON:
			LAMP_TURNON();
			if (xLightControl.timeOnHours == xLightControl.counterHours)
			{
				xLightControl.lightingState  = LIGHT_OFF;
				xLightControl.counterHours	 = 0;
				xLightControl.counterSeconds = 0;
			}
			break;

		case LIGHT_OFF:
			LAMP_TURNOFF();
			if (xLightControl.timeOffHours == xLightControl.counterHours)
			{
				xLightControl.lightingState  = LIGHT_ON;
				xLightControl.counterHours 	 = 0;
				xLightControl.counterSeconds = 0;
			}
			break;

		default:
			break;
		}
	}
}
