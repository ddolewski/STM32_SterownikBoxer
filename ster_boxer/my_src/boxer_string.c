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
	itoa(xLightControl.lightingState, TempString);
	strcat(xOutString, TempString);
	strcat(xOutString, " ");

	memset(TempString, 0, 20);
	itoa(xLightCounters.counterHours, TempString);
	strcat(xOutString, TempString);

	strcat(xOutString, " END");
}



void MakeDateString(char * TimeString, time_complex_t * rtc_time)
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

//////////////////////////////////////////////////////////////////
void displayMakeTimeString(char * TimeString, time_complex_t * xRtcTime)
{
	char houstr[5] = {0};
	char minstr[5] = {0};

	itoa(xRtcTime->hour, houstr);
	itoa(xRtcTime->min, minstr);

	memset(TimeString, 0, SIZEOF_TAB(TimeString));

	if(xRtcTime->hour <= 9 && xRtcTime->min <= 9)
	{
		strcat(TimeString, "0");
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString, "0");
		strcat(TimeString,  minstr);
	}
	else if(xRtcTime->hour <= 9 && xRtcTime->min > 9)
	{
		strcat(TimeString, "0");
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString,  minstr);
	}
	else if(xRtcTime->hour > 9 && xRtcTime->min <= 9)
	{
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString, "0");
		strcat(TimeString,  minstr);
	}
	else if(xRtcTime->hour > 9 && xRtcTime->min > 9)
	{
		strcat(TimeString,  houstr);
		strcat(TimeString, ":");
		strcat(TimeString,  minstr);
	}
}
//////////////////////////////////////////////////////////////////
void displayMakeDateString(char * xDateString, time_complex_t * xRtcDate)
{
	char day[5] = {0};
	char month[5] = {0};
	char year[6] = {0};

	itoa(xRtcDate->mday, day);
	itoa(xRtcDate->month, month);
	itoa(xRtcDate->year, year);

	memset(xDateString, 0, SIZEOF_TAB(xDateString));

	if (xRtcDate->mday < 10 && xRtcDate->month < 10) // np. 05.07
	{
		strcat(xDateString, "0");
		strcat(xDateString,  day);
		strcat(xDateString, "-");
		strcat(xDateString, "0");
		strcat(xDateString,  month);
	}
	else if (xRtcDate->mday < 10 && xRtcDate->month >= 10) // np. 09.11
	{
		strcat(xDateString, "0");
		strcat(xDateString,  day);
		strcat(xDateString, "-");
		strcat(xDateString,  month);
	}
	else if (xRtcDate->mday >= 10 && xRtcDate->month < 10) // np. 12.06
	{
		strcat(xDateString,  day);
		strcat(xDateString, "-");
		strcat(xDateString, "0");
		strcat(xDateString,  month);
	}
	else if (xRtcDate->mday >= 10 && xRtcDate->month >= 10) // np. 12.11
	{
		strcat(xDateString,  day);
		strcat(xDateString, "-");
		strcat(xDateString,  month);
	}
	else
	{
//		UART_PutString(SC"bledna data!\n\r");
	}

	strcat(xDateString, "-");
	strcat(xDateString, year);
}
//////////////////////////////////////////////////////////////////
void displayWeekDayConvert(uint8_t xWeekDayNum, char * xStrDay)
{
	memset(xStrDay, 0, SIZEOF_TAB(xStrDay));

	switch (xWeekDayNum)
	{
		case 0: strcpy(xStrDay, "Poniedzialek");
		break;

		case 1: strcpy(xStrDay, "Wtorek");
		break;

		case 2: strcpy(xStrDay, "Sroda");
		break;

		case 3: strcpy(xStrDay, "Czwartek");
		break;

		case 4: strcpy(xStrDay, "Piatek");
		break;

		case 5: strcpy(xStrDay, "Sobota");
		break;

		case 6: strcpy(xStrDay, "Niedziela");
		break;

		default:
		break;
	}
}
