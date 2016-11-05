/*
 * boxer_display.c
 *
 *  Created on: 5 sie 2015
 *      Author: Doles
 */

#include "boxer_display.h"

static systime_t displayTimer = 0;
static float lastWaterPh = 0;
static float lastSoilPh = 0;

static void Display_ShowPage(lcdDisplayData_t * display);
static void Display_ChangePage(lcdDisplayData_t * display);

void Display_Handler(void)
{
	Display_ChangePage((lcdDisplayData_t*)&displayData);
	Display_ShowPage((lcdDisplayData_t*)&displayData);
}

static void Display_ShowPage(lcdDisplayData_t * display)
{
	if (!readDataFlag)
	{
		if (systimeTimeoutControl(&displayTimer, 500))
		{
			MakeDateString((char*)display->time, &rtcDate);
			char tempString[10] = {0};

			if (display->page == 1)
			{
				float tempSHT2x = display->tempSHT2x;
				float tempds18b20_1 = display->tempDS18B20_1_t;
				float tempds18b20_2 = display->tempDS18B20_2_t;
				GLCD_GoTo(0,0);
				GLCD_WriteString((uint8_t*)display->time);
				GLCD_GoTo(65,0);
				GLCD_WriteString((uint8_t*)"Boxer_v2.5");
				memset(display->time, 0, 20);

				GLCD_GoTo(0,1);
				GLCD_WriteString((uint8_t*)"=====================");
/////////////////////////////////////////////////////
				GLCD_GoTo(0,2);
				GLCD_WriteString((uint8_t*)"T. dolna [*C]:");
				GLCD_GoTo(90,2);
				ftoa(tempSHT2x, tempString, 1);
				GLCD_WriteString((uint8_t*)tempString);
				memset(tempString, 0, 10);
///////////////////////////////////////////////////////
				GLCD_GoTo(0,3);
				GLCD_WriteString((uint8_t*)"T. lampa [*C]:");

				GLCD_GoTo(90,3);
				ftoa(tempds18b20_1, tempString, 1);
				GLCD_WriteString((uint8_t*)tempString);
				memset(tempString, 0, 10);

/////////////////////////////////////////////////////
				GLCD_GoTo(0,4);
				GLCD_WriteString((uint8_t*)"T. srodek[*C]:");

				GLCD_GoTo(90,4);
				ftoa(tempds18b20_2, tempString, 1);
				GLCD_WriteString((uint8_t*)tempString);
				memset(tempString, 0, 10);

				itoa(display->humiditySHT2x, tempString);
				GLCD_GoTo(0,5);
				GLCD_WriteString((uint8_t*)"Wilg. [%RH]:");
				GLCD_GoTo(90,5);
				GLCD_WriteString((uint8_t*)tempString);
				memset(tempString, 0, 10);

				itoa(display->lux, tempString);
				GLCD_GoTo(0,6);
				GLCD_WriteString((uint8_t*)"Nat.lampy [lx]:");
				GLCD_GoTo(90,6);
				GLCD_WriteString((uint8_t*)tempString);
				memset(tempString, 0, 10);
			}
			else if (display->page == 2)
			{
				GLCD_GoTo(0,0);
				GLCD_WriteString((uint8_t*)display->time);
				GLCD_GoTo(65,0);
				GLCD_WriteString((uint8_t*)"Boxer_v2.5");
				memset(display->time, 0, 20);

				GLCD_GoTo(0,1);
				GLCD_WriteString((uint8_t*)"=====================");

				lastWaterPh = pH.water;
				lastSoilPh = pH.soil;

				GLCD_GoTo(0,2);
				GLCD_WriteString((uint8_t*)"pH wody");
				GLCD_GoTo(47,2);
				GLCD_WriteString((uint8_t*)"[pH]:");

				if (calibrateFlags.processActive == 1 && calibrateFlags.probeType == PROBE_WATER)
				{
					GLCD_GoTo(82,2);
					GLCD_WriteString((uint8_t*)"-CAL-");
				}
				else
				{
					if (lastWaterPh == pH.water)
					{
						if (pH.water <= 1 || pH.water >= 14)
						{
							GLCD_GoTo(82,2);
							GLCD_WriteString((uint8_t*)"------");
						}
						else
						{
							ftoa(pH.water, tempString, 2);
							GLCD_GoTo(82,2);
							GLCD_WriteString((uint8_t*)tempString);
						}
					}
					else
					{
						GLCD_GoTo(82,2);
						GLCD_WriteString((uint8_t*)"        ");
					}
				}

				memset(tempString, 0, 10);

				ftoa(pH.soil, tempString, 2);
				GLCD_GoTo(0,3);
				GLCD_WriteString((uint8_t*)"pH gleby");
				GLCD_GoTo(47,3);
				GLCD_WriteString((uint8_t*)"[pH]:");

				if (calibrateFlags.processActive == 1 && calibrateFlags.probeType == PROBE_SOIL)
				{
					GLCD_GoTo(82,3);
					GLCD_WriteString((uint8_t*)"-CAL-");
				}
				else
				{
					if (lastSoilPh == pH.soil)
					{
						if (pH.soil <= 1 || pH.soil >= 14)
						{
							GLCD_GoTo(82,3);
							GLCD_WriteString((uint8_t*)"------");
						}
						else
						{
							ftoa(pH.soil, tempString, 2);
							GLCD_GoTo(82,3);
							GLCD_WriteString((uint8_t*)tempString);
						}
					}
					else
					{
						GLCD_GoTo(82,3);
						GLCD_WriteString((uint8_t*)"        ");
					}
				}

				memset(tempString, 0, 10);
				GLCD_GoTo(0,4);
				GLCD_WriteString((uint8_t*)"Wilg. gleby: ");


				if (soilMoisture == SOIL_DRY)
				{
					if (lastSoilMoistState != soilMoisture)
					{
						GLCD_GoTo(82,4);
						GLCD_WriteString((uint8_t*)"       ");
					}
					strcat(tempString,  (char*)"SUCHA!");
				}
				else if(soilMoisture == SOIL_WET)
				{
					if (lastSoilMoistState != soilMoisture)
					{
						GLCD_GoTo(82,4);
						GLCD_WriteString((uint8_t*)"       ");
					}
					strcat(tempString,  (char*)"OK");
				}
				else if (soilMoisture == SOIL_UNKNOWN_STATE)
				{
					if (lastSoilMoistState != soilMoisture)
					{
						GLCD_GoTo(82,4);
						GLCD_WriteString((uint8_t*)"       ");
					}
					strcat(tempString,  (char*)"BLAD!");
				}


				GLCD_GoTo(82,4);
				GLCD_WriteString((uint8_t*)tempString);
				memset(tempString, 0, 10);
			}
			else if (display->page == 3)
			{
				GLCD_GoTo(0,0);
				GLCD_WriteString((uint8_t*)display->time);
				GLCD_GoTo(65,0);
				GLCD_WriteString((uint8_t*)"Boxer_v2.5");
				memset(display->time, 0, 20);

				GLCD_GoTo(0,1);
				GLCD_WriteString((uint8_t*)"=====================");

				GLCD_GoTo(0,2);
				GLCD_WriteString((uint8_t*)"Stan lampy:");

				if (lightControl.lightingState == LIGHT_ON)
				{
					if (lastLightState != lightControl.lightingState)
					{
						GLCD_GoTo(72,2);
						GLCD_WriteString((uint8_t*)"        ");
					}

					GLCD_GoTo(78,2);
					GLCD_WriteString((uint8_t*)"wlaczona");
				}
				else if (lightControl.lightingState == LIGHT_OFF)
				{
					if (lastLightState != lightControl.lightingState)
					{
						GLCD_GoTo(78,2);
						GLCD_WriteString((uint8_t*)"         ");
					}

					GLCD_GoTo(72,2);
					GLCD_WriteString((uint8_t*)"wylaczona");
				}

				memset(tempString, 0, 10);

				GLCD_GoTo(0,3);
				GLCD_WriteString((uint8_t*)"Licznik czasu [h]:");

				if (lightCounters.counterHours < 10 && lightCounters.counterHours != 1)
				{
					GLCD_GoTo(120,3);
				}
				else if (lightCounters.counterHours == 1)
				{
					GLCD_GoTo(121,3);
				}
				else if (lightCounters.counterHours >= 10 && lightCounters.counterHours != 11)
				{
					GLCD_GoTo(114,3);
				}
				else if (lightCounters.counterHours == 11)
				{
					GLCD_GoTo(115,3);
				}
				else if (lightCounters.counterHours > 19)
				{
					GLCD_GoTo(115,3);
				}

				itoa(lightCounters.counterHours, tempString);
				GLCD_WriteString((uint8_t*)tempString);
				memset(tempString, 0, 10);

				GLCD_GoTo(0,4);
				GLCD_WriteString((uint8_t*)"OFF/ON [h]:");

				memset(tempString, 0, 10);

				if ((lastTimeOffHour != lightControl.timeOffHours) && (lastTimeOnHour != lightControl.timeOnHours))
				{
					GLCD_GoTo(92,4);
					GLCD_WriteString((uint8_t*)"      ");

					lastTimeOffHour = lightControl.timeOffHours;
					lastTimeOnHour = lightControl.timeOnHours;
				}

				if (lightControl.timeOffHours < 10 && lightControl.timeOnHours > 10)
				{
					GLCD_GoTo(102,4);
					itoa(lightControl.timeOffHours, tempString);
					GLCD_WriteString((uint8_t*)tempString);
					GLCD_GoTo(108,4);
					GLCD_WriteString((uint8_t*)"/");
					GLCD_GoTo(114,4);
					itoa(lightControl.timeOnHours, tempString);
					GLCD_WriteString((uint8_t*)tempString);
				}
				else if (lightControl.timeOffHours > 10 && lightControl.timeOnHours < 10)
				{
					GLCD_GoTo(102,4);
					itoa(lightControl.timeOffHours, tempString);
					GLCD_WriteString((uint8_t*)tempString);
					GLCD_GoTo(114,4);
					GLCD_WriteString((uint8_t*)"/");
					GLCD_GoTo(120,4);
					itoa(lightControl.timeOnHours, tempString);
					GLCD_WriteString((uint8_t*)tempString);
				}
				else if ((lightControl.timeOffHours >= 10 && lightControl.timeOffHours <= 14) &&
						(lightControl.timeOnHours >= 10 && lightControl.timeOnHours <= 14))
				{
					GLCD_GoTo(95,4);
					itoa(lightControl.timeOffHours, tempString);
					GLCD_WriteString((uint8_t*)tempString);
					GLCD_GoTo(108,4);
					GLCD_WriteString((uint8_t*)"/");
					GLCD_GoTo(114,4);
					itoa(lightControl.timeOnHours, tempString);
					GLCD_WriteString((uint8_t*)tempString);
				}

				GLCD_GoTo(0,5);
				GLCD_WriteString((uint8_t*)"Temp. max [*C]: ");
				GLCD_GoTo(114,5);

				itoa(tempControl.userTemp, tempString);
				GLCD_WriteString((uint8_t*)tempString);
				memset(tempString, 0, 10);
			}
		}
	}
}

static void Display_ChangePage(lcdDisplayData_t * display)
{
	if (display->pageCounter == 10)
	{
		if (display->page == 1)
		{
			display->page = 2;
		}
		else if (display->page == 2)
		{
			display->page = 3;
		}
		else if (display->page == 3)
		{
			display->page = 1;
		}

		GLCD_ClearScreen();
		display->pageCounter = 0;
	}

#ifdef DISPLAY_PAGE1_TEST
		display->page = 1;
#endif

#ifdef DISPLAY_PAGE2_TEST
		display->page = 2;
#endif

#ifdef DISPLAY_PAGE3_TEST
		display->page = 3;
#endif
}
