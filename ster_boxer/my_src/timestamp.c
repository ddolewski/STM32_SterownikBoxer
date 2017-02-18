#include "timestamp.h"
//#include "mkuart.h"

#define TIME_SEC_IN_HOUR	3600

#define TIME_DAYS_OF_WEEK	7
#define TIME_NR_OF_MONTHS	12

static const uint8_t timeDaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // tablica liczby dni miesiecy dla roku nieprzestepnego

static const dst_date_t timeDstDates[21] = // tablica dni ostatnich niedziel w marcu (kolumna 1) i w pazdzierniku (kolumna 2) zgodnie z A0004-2
{
	{{2016,3,27,0,1,0,0}, {2016,10,30,0,1,0,0}}, // 2016
	{{2017,3,26,0,1,0,0}, {2017,10,29,0,1,0,0}}, // 2017
	{{2018,3,25,0,1,0,0}, {2018,10,28,0,1,0,0}}, // 2018
	{{2019,3,31,0,1,0,0}, {2019,10,27,0,1,0,0}}, // 2019
	{{2020,3,29,0,1,0,0}, {2020,10,25,0,1,0,0}}, // 2020
	{{2021,3,28,0,1,0,0}, {2021,10,31,0,1,0,0}}, // 2021
	{{2022,3,27,0,1,0,0}, {2022,10,30,0,1,0,0}}, // 2022
	{{2023,3,26,0,1,0,0}, {2023,10,29,0,1,0,0}}, // 2023
	{{2024,3,31,0,1,0,0}, {2024,10,27,0,1,0,0}}, // 2024
	{{2025,3,30,0,1,0,0}, {2025,10,26,0,1,0,0}}, // 2025
	{{2026,3,29,0,1,0,0}, {2026,10,25,0,1,0,0}}, // 2026
	{{2027,3,28,0,1,0,0}, {2027,10,31,0,1,0,0}}, // 2027
	{{2028,3,26,0,1,0,0}, {2028,10,29,0,1,0,0}}, // 2028
	{{2029,3,25,0,1,0,0}, {2029,10,28,0,1,0,0}}, // 2029
	{{2030,3,31,0,1,0,0}, {2030,10,27,0,1,0,0}}, // 2030
	{{2031,3,30,0,1,0,0}, {2031,10,26,0,1,0,0}}, // 2031
	{{2032,3,28,0,1,0,0}, {2032,10,31,0,1,0,0}}, // 2032
	{{2033,3,27,0,1,0,0}, {2033,10,30,0,1,0,0}}, // 2033
	{{2034,3,26,0,1,0,0}, {2034,10,29,0,1,0,0}}, // 2034
	{{2035,3,25,0,1,0,0}, {2035,10,28,0,1,0,0}}, // 2035
	{{2036,3,30,0,1,0,0}, {2036,10,26,0,1,0,0}}  // 2036
};


//-----------------------------------------------------------------------------
// Funkcja cykliczna obslugi synchronizacji
// xYearToTest/in: rok do przetestowania
// return: 	TRUE - rok przestepny
//-----------------------------------------------------------------------------
uint8_t timeLeapYearCheck(uint16_t xYearToTest)
{
	if ((xYearToTest % 4 == 0) && ((xYearToTest % 100 != 0) || (xYearToTest % 400 == 0))) // leap year
	{
		return TRUE;
	}
	else // normal year
	{
		return FALSE;
	}
}

//-----------------------------------------------------------------------------
// Funkcja konwersji stempla czasowego NTP(00:00:00 1 January 1900) do czasu zlozonego
// xTimestamp/in: stempel czasowy
// xTime/out: czas zlozony
// return: 	void
//-----------------------------------------------------------------------------
void timeTimestampToTime(uint32_t xTimestamp, time_complex_t * xTime)
{
	uint64_t secondsTemp = 0;
	uint8_t leapYear = 0;

	// year calculations
	xTime->year = (uint16_t)1970;
	while (1)
	{
		if (timeLeapYearCheck(xTime->year) == 1)
		{
			leapYear = 1;
			secondsTemp += (uint64_t)31622400; // += 366 days * 24 hours * 3600 seconds
		}
		else // normal year
		{
			leapYear = 0;
			secondsTemp += (uint64_t)31536000; // += 365 days * 24 hours * 3600 seconds
		}
		
		if (secondsTemp > xTimestamp)
		{
			switch (leapYear) // odejmuje sekundy powodujace >= xTimestamp
			{
				case 0: secondsTemp -= (uint64_t)31536000; break;
				case 1: secondsTemp -= (uint64_t)31622400; break;
			}
			
			break;
		}
		else
		{
			xTime->year++;
		}
	}

	xTimestamp -= (uint32_t)secondsTemp; // seconds left
	secondsTemp = 0;
	xTime->month = 0;
	
	while (1)
	{
		secondsTemp += (uint64_t)((uint32_t)timeDaysOfMonth[xTime->month] * (uint32_t)86400); // * 24 * 3600
		if ((xTime->month == 1) && (leapYear == 1)) secondsTemp += (uint32_t)86400;
		if (secondsTemp > xTimestamp)
		{
			secondsTemp -= (uint64_t)((uint32_t)timeDaysOfMonth[xTime->month] * (uint32_t)86400); // odejmuje sekundy powodujace >= xTimestamp
			if ((xTime->month == 1) && (leapYear == 1)) secondsTemp -= (uint32_t)86400;
			break;
		}
		else
		{
			xTime->month++;
		}
	}
	xTime->month += 1; // from range 0-11 to 1-12
	// day, hour, minute and second calculations
	xTimestamp -= secondsTemp; // seconds left
	xTime->mday = (uint32_t)(xTimestamp / (uint32_t)86400);
	xTime->mday += 1; // from range 0-30 to 1-31
	xTimestamp %= (uint32_t)86400; // seconds left
	xTime->hour = (uint32_t)(xTimestamp / (uint32_t)3600);
	xTimestamp %= (uint32_t)3600; // seconds left
	xTime->min = xTimestamp / 60;
	xTimestamp %= 60; // seconds left
	xTime->sec = xTimestamp;
	xTime->wday = timeCalcDayWeek(xTime);
}

//-----------------------------------------------------------------------------
// Funkcja konwersji czasu zlozonego do stempla czasowego NTP (00:00:00 1 January 1900)
// xTime/in: czas zlozony
// xTimestamp/out: wyjsciowy stempel czasowy
// return: 	void
//-----------------------------------------------------------------------------
void timeTimeToTimestamp(time_complex_t * xTime, uint32_t * xTimestamp)
{
	uint32_t timestamp = 0;
	uint8_t leapYear = 0;
	uint16_t agk;
	uint8_t temp;
	// year calculations
	for (agk = 1970; agk < xTime->year; agk++)
	{
		if (timeLeapYearCheck(agk) == 1)
		{
			leapYear = 1;
			timestamp += (uint32_t)31622400; // += 366 days * 24 hours * 3600 seconds
		}
		else
		{
			leapYear = 0;
			timestamp += (uint32_t)31536000; // += 365 days * 24 hours * 3600 seconds
		}
	}
	// month calculations
	leapYear = timeLeapYearCheck(xTime->year);
	temp = xTime->month - 1; //skalowanie do tablicy
	for (agk = 0; agk < temp; agk++)
	{
		timestamp += (uint32_t)(timeDaysOfMonth[agk] * (uint32_t)86400); // * 24 * 3600;
		if ((leapYear == 1) && (agk == 1)) 
			timestamp += (uint32_t)86400;
	}
	
	// day, hour, minute and second calculations
	timestamp += (uint32_t)((xTime->mday - 1) * (uint32_t)86400);
	timestamp += (uint32_t)(xTime->hour * (uint32_t)3600);
	timestamp += (uint32_t)(xTime->min * (uint8_t)60);
	timestamp += (uint32_t)xTime->sec;

	*xTimestamp = timestamp;
}

//-----------------------------------------------------------------------------
// Funkcja konwersji stempla czasowego do czasu zlozonego
// xTime/in: czas zlozony
// xTimestamp/out: wyjsciowy stempel czasowy
// return: 	dzien tygodnia 0 - poniedzialek, 6 - niedziela
//-----------------------------------------------------------------------------
uint8_t timeCalcDayWeek(time_complex_t * xTime)
{
	uint32_t Y,C,M,N,D;
	uint8_t tempDay	= 0;
	uint8_t dayOfWeek = 0;

	M = 1 + (9 + xTime->month) % 12;
	Y = ((uint32_t)2000 + (uint32_t)xTime->year) - (M > 10);
	C = Y / 100;
	D = Y % 100;

	N=((13*M-1)/5+D+D/4+C/4+5*C+xTime->mday)%7;
	tempDay = (7+N)%7;

	if (tempDay == 0)
	{
		dayOfWeek = 6;
	}
	else
	{
		dayOfWeek = tempDay - 1;
	}
	return dayOfWeek;
}
//-----------------------------------------------------------------------------
// Funkcja obliczjaca maksymalna ilosc dni w miesiacu
// xMonth/in: miesiac
// xYear/in: year
// return: 	iosc dni w nanym miesiacu
//-----------------------------------------------------------------------------
uint8_t timeMaxDayOfMonthGet(uint8_t xMonth, uint16_t xYear)
{
	uint8_t maxDayOfMonth = 31;
	if (2099 < xYear)
	{
//		USARTx_SendString(SC"e: timeMaxDayOfMonthGet - zbyt duza wartosc pola rok - dopuszczalne 0-99\r\n");

	}
	if ((0 < xMonth) && (xMonth <= TIME_NR_OF_MONTHS))
	{
		maxDayOfMonth = timeDaysOfMonth[xMonth - 1];
		
		if ((xMonth == 2) && timeLeapYearCheck(xYear))
		{
			maxDayOfMonth++;
		}
	}
	else
	{
//		USARTx_SendString(SC"e: timeMaxDayGet - nieprawidlowy nr miesiaca\r\n");
	}
	return maxDayOfMonth;
}

//-----------------------------------------------------------------------------
// Konwersja czasu UTC do czasu lokalnego
// xInUtcTime\in: wejsciowy czas UTC
// xOutLocalTime\out:wyjsciowy czas lokalny
// xLocaltimeZoneHours\in: strefa czasowa czesc godzin
// xLocaltimeZoneMinutes\in: strefa czasowa czesc minut
// xDstFlag\in: flaga czasu letniego
// return: 1 - error, 0 - no error
//-----------------------------------------------------------------------------
uint8_t timeUtcToLocalConv(time_complex_t * xInUtcTime, time_complex_t * xOutLocalTime)
{
	uint8_t localtimeZoneErr 	= FALSE;
	uint8_t dstFlagErr 			= FALSE;
	uint8_t yearScopeErr		= FALSE;
	time_complex_t timeUtc		= *xInUtcTime;
	uint32_t  timestamp			= 0;
	uint8_t summerTimeFlag 		= FALSE;
	uint8_t err					= FALSE;

	if ((timeUtc.year <  TIME_DST_BASE_YEAR) || (TIME_DST_MAX_YEAR <= timeUtc.year))
	{
//		UART_PutInt(timeUtc.year, 10);
		yearScopeErr = TRUE;
	}
	if ((!localtimeZoneErr)&& (!dstFlagErr))
	{
		if (!yearScopeErr)
		{
			timeTimeToTimestamp(&timeUtc, &timestamp);	
			timestamp += TIME_SEC_IN_HOUR;
			timeTimestampToTime(timestamp, xOutLocalTime);

			summerTimeFlag = timeCheckDstStatus(&timeUtc);

			if (summerTimeFlag)
			{
				timestamp += TIME_SEC_IN_HOUR;
			}
			
			timeTimestampToTime(timestamp, xOutLocalTime);
		}
		else
		{
//			USARTx_SendString(SC"i: timeUtcToLocalConv - przekroczony rok 2036 lub ponizej 2012, timestamp zawiera bledne dane\r\n");
			err	= TRUE;
		}
	}
	else
	{
//		USARTx_SendString(SC"e: timeUtcToLocalConv -  nieprawidlowy parametr\r\n");
		err	= TRUE;
	}

	return err;
}
//-----------------------------------------------------------------------------
// Funkcja sprawdzajaca czy nalezy zmienic czas z letniego na zimowy i odwrotnie
// xTime/in: czas wejsciowy
// return: TRUE / FALSE - zmiana czasu na DST lub nie
//-----------------------------------------------------------------------------
bool_t timeCheckDstStatus(time_complex_t * xInTime)
{
	bool_t IsDstTime = FALSE;
	static time_complex_t startDstTime;
	static time_complex_t endDstTime;

	if (TIME_DST_BASE_YEAR <= (xInTime->year))
	{
		startDstTime = timeDstDates[xInTime->year - TIME_DST_BASE_YEAR].periodStart;
		endDstTime	 = timeDstDates[xInTime->year - TIME_DST_BASE_YEAR].periodEnd;
	}

	uint32_t timestampActualTime = 0;
	uint32_t timestampStartDst = 0;
	uint32_t timestampEndDst = 0;

	timeTimeToTimestamp(&startDstTime, &timestampStartDst);
	timeTimeToTimestamp(&endDstTime, &timestampEndDst);
	timeTimeToTimestamp(xInTime, &timestampActualTime);

	if ((timestampActualTime >= timestampStartDst) && (timestampActualTime < timestampEndDst))
	{
		IsDstTime = TRUE;
	}
	else
	{
		IsDstTime = FALSE;
	}
	
	return IsDstTime;
}
//-----------------------------------------------------------------------------
// Funkcja kopiujaca czas zlozony
// xSrcTime/in: czas zrodlowy
// xDstTime/in: czas docelowy
// return: void
//-----------------------------------------------------------------------------
void timeComplexCopy(time_complex_t * xSrcTime, time_complex_t * xDstTime)
{
	*xDstTime = *xSrcTime;
}

//-----------------------------------------------------------------------------
// Funkcja porownujaca dwa czasy
// xTimeA/in: czas A
// xTimeB/in: czas B
// return: wynik porownania czasu
//-----------------------------------------------------------------------------
time_compare_result_t timeComplexCompare(time_complex_t * xTimeA, time_complex_t * xTimeB)
{
	time_compare_result_t compareRes;
	uint32_t timestampA = 0;
	uint32_t timestampB = 0;


	timeTimeToTimestamp(xTimeA, &timestampA);
	timeTimeToTimestamp(xTimeB, &timestampB);

	if(timestampA == timestampB)
	{
		compareRes.result 		= TIME_COMP_EQUAL;
		compareRes.secAbsDiff	=  0;
	}
	else
	{
		if(timestampA < timestampB)
		{
			compareRes.result 		= TIME_COMP_B_GREATER;
			compareRes.secAbsDiff	=  timestampB - timestampA;
		}
		else
		{
			compareRes.result 		= TIME_COMP_A_GREATER;
			compareRes.secAbsDiff	=  timestampA - timestampB;
		}
	}
	return compareRes;
}

//-----------------------------------------------------------------------------
// Funkcja porownujaca dwa czasy
// xTimeA/in: 	czas A
// xTimeB/out: 	czas B
//return: 0 - jezeli sa identyczne; 1 - jezeli A > B; -1 jezeli A < B
//-----------------------------------------------------------------------------
int8_t timeSimpleCompare(time_simple_t * xTimeA, time_simple_t * xTimeB)
{

	if (xTimeA->hour > xTimeB->hour)
	{
		return 1;
	}
	else
	{
		if (xTimeA->hour < xTimeB->hour)
		{
			return -1;
		}
	}

	if (xTimeA->min  > xTimeB->min)
	{
		return 1;
	}
	else
	{
		if (xTimeA->min < xTimeB->min)
		{
			return -1;
		}
	}

	return 0;

}

uint8_t timeLocalToUtcConv(time_complex_t * xInLocalTime, time_complex_t * xOutUtcTime)
{
	uint8_t yearScopeErr		= FALSE;
	time_complex_t timeInLocal	= *xInLocalTime;
	uint32_t  timestamp			= 0;
	uint8_t summerTimeFlag 		= FALSE;
	uint8_t err					= FALSE;

	time_complex_t timeOutUtc;

	if ((timeInLocal.year <  TIME_DST_BASE_YEAR) || (TIME_DST_MAX_YEAR <= timeInLocal.year))
	{
		yearScopeErr = TRUE;
	}

	if (!yearScopeErr)
	{
		timeTimeToTimestamp(&timeInLocal, &timestamp);
//		USARTx_SendString(SC"LOCAL TIMESTAMP: ");
		
//		UART_PutInt(timestamp, 10);
		timestamp -= (1 * TIME_SEC_IN_HOUR); //UTC +1:00
		
		timeTimestampToTime(timestamp, &timeOutUtc);
//		USARTx_SendString(SC"UTC TIMESTAMP: ");
//		UART_PutInt(timestamp, 10);
		
		summerTimeFlag = timeCheckDstStatus(&timeOutUtc);

//		USARTx_SendString(SC"Flaga DST: ");
//		UART_PutInt(summerTimeFlag, 10);
		
		if (summerTimeFlag)
		{
			timestamp -= TIME_SEC_IN_HOUR;
		}
			
		timeTimestampToTime(timestamp, &timeOutUtc);
		*xOutUtcTime = timeOutUtc;
	}
	else
	{
//		USARTx_SendString(SC"i: timeLocalToUtcConv - przekroczony rok 2036 lub ponizej 2012, timestamp zawiera bledne dane\r\n");
		err	= TRUE;
	}

	return err;
}
