/*
 * boxer_string.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_string.h"

static char TempString[20] = {0};

void PrepareUdpString(uint32_t Lux, float humSht, float tempSht, float ds18b20Temp1, float ds18b20Temp2, char * xOutString)
{
	// STA BOXER humSht tempSht tempDs1 tempDs2 lux soilPh waterpH soilMoist lightState LightHouCnt END
	memset(xOutString, 0, 100);

	strcat(xOutString, "STA BOXER ");

	itoa((int)humSht, TempString);
	strcat(xOutString, TempString);
	strcat(xOutString, " ");

	memset(TempString, 0, 20);
	ftoa(tempSht, TempString, 1);
	strcat(xOutString, TempString);
	strcat(xOutString, " ");

	memset(TempString, 0, 20);
	ftoa(ds18b20Temp1, TempString, 1);
	strcat(xOutString, TempString);
	strcat(xOutString, " ");

	memset(TempString, 0, 20);
	ftoa(ds18b20Temp2, TempString, 1);
	strcat(xOutString, TempString);
	strcat(xOutString, " ");

	memset(TempString, 0, 20);
	itoa(Lux, TempString);
	strcat(xOutString, TempString);
	strcat(xOutString, " ");

	memset(TempString, 0, 20);
	ftoa(pH.soil, TempString, 2);
	strcat(xOutString, TempString);
	strcat(xOutString, " ");

	memset(TempString, 0, 20);
	ftoa(pH.water, TempString, 2);
	strcat(xOutString, TempString);
	strcat(xOutString, " ");

	memset(TempString, 0, 20);
	itoa(soilMoisture, TempString);
	strcat(xOutString, TempString);
	strcat(xOutString, " ");

	memset(TempString, 0, 20);
	itoa(lightControl.lightingState, TempString);
	strcat(xOutString, TempString);
	strcat(xOutString, " ");

	memset(TempString, 0, 20);
	itoa(lightCounters.counterHours, TempString);
	strcat(xOutString, TempString);

	strcat(xOutString, " END");
}



void MakeDateString(char * TimeString, date_t * rtc_time)
{
    char houstr[5] = {0};
    char minstr[5] = {0};
    char secstr[5] = {0};

    memset(TimeString, 0, 20);

    itoa(rtc_time->hour, houstr);
    itoa(rtc_time->min, minstr);
    itoa(rtc_time->sec, secstr);

    if(rtc_time->hour <= 9 && rtc_time->min <= 9 && rtc_time->sec <= 9)
    {
		strcat(TimeString, "0");
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString, "0");
		strcat(TimeString,  minstr);
		strcat(TimeString, ":");
		strcat(TimeString, "0");
		strcat(TimeString,  secstr);
    }
    else if(rtc_time->hour <= 9 && rtc_time->min <= 9 && rtc_time->sec > 9)
    {
		strcat(TimeString, "0");
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString, "0");
		strcat(TimeString,  minstr);
		strcat(TimeString, ":");
		strcat(TimeString,  secstr);
    }
    else if(rtc_time->hour <= 9 && rtc_time->min > 9 && rtc_time->sec <= 9)
    {
		strcat(TimeString, "0");
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString,  minstr);
		strcat(TimeString, ":");
		strcat(TimeString, "0");
		strcat(TimeString,  secstr);
    }
    else if(rtc_time->hour <= 9 && rtc_time->min > 9 && rtc_time->sec > 9)
    {
		strcat(TimeString, "0");
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString,  minstr);
		strcat(TimeString, ":");
		strcat(TimeString,  secstr);
    }
    else if(rtc_time->hour > 9 && rtc_time->min <= 9 && rtc_time->sec <= 9)
    {
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString, "0");
		strcat(TimeString,  minstr);
		strcat(TimeString, ":");
		strcat(TimeString, "0");
		strcat(TimeString,  secstr);
    }
    else if(rtc_time->hour > 9 && rtc_time->min <= 9 && rtc_time->sec > 9)
    {
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString, "0");
		strcat(TimeString,  minstr);
		strcat(TimeString, ":");
		strcat(TimeString,  secstr);
    }
    else if(rtc_time->hour > 9 && rtc_time->min > 9 && rtc_time->sec <= 9)
    {
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString,  minstr);
		strcat(TimeString, ":");
		strcat(TimeString, "0");
		strcat(TimeString,  secstr);
    }
    else if(rtc_time->hour > 9 && rtc_time->min > 9 && rtc_time->sec > 9)
    {
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString,  minstr);
		strcat(TimeString, ":");
		strcat(TimeString,  secstr);
    }
}
