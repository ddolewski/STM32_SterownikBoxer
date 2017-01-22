#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

//#include "global.h"
#include "stdint.h"
#include "boxer_struct.h"

#define TIME_DST_BASE_YEAR 	2016
#define TIME_DST_MAX_YEAR	2036

//typedef struct
//{
//	uint16_t 	year;
//	uint8_t 	month;
//	uint8_t 	mday;
//	uint8_t 	wday;
//	uint8_t 	hour;
//	uint8_t 	min;
//	uint8_t 	sec;
//
//}time_complex_t;
//
//typedef struct
//{
//	uint8_t 	hour;
//	uint8_t 	min;
//}time_simple_t;

typedef struct
{
	time_complex_t periodStart;
	time_complex_t periodEnd;
}dst_date_t;

typedef enum
{
	TIME_COMP_A_GREATER,
	TIME_COMP_B_GREATER,
	TIME_COMP_EQUAL
}time_comp_t;

typedef struct
{
	time_comp_t	result;
	uint32_t secAbsDiff;
}time_compare_result_t;

typedef enum
{
	LOCALIZATION_FIRST,
	EUROPE = LOCALIZATION_FIRST,
	AUSTRALIA,
	UNITED_STATES,
	CANADA,
	LOCALIZATION_LAST = CANADA
}localization_t;

uint8_t timeLeapYearCheck(uint16_t year_to_test);
void timeTimestampToTime(uint32_t xTimestamp, time_complex_t * xTime);
void timeTimeToTimestamp(time_complex_t * xTime, uint32_t * xTimestamp);
uint8_t timeCalcDayWeek(time_complex_t * xTime);
uint8_t timeMaxDayOfMonthGet(uint8_t xMonth, uint16_t  xYear);
uint8_t timeUtcToLocalConv(time_complex_t * xInUtcTime, time_complex_t * xOutLocalTime);
void timeComplexCopy(time_complex_t * xSrcTime, time_complex_t * xDstTime);
time_compare_result_t timeComplexCompare(time_complex_t * xTimeA, time_complex_t * xTimeB);
int8_t timeSimpleCompare(time_simple_t * xTimeA, time_simple_t * xTimeB);
uint8_t timeLocalToUtcConv(time_complex_t * xInLocalTime, time_complex_t * xOutUtcTime);

void testTimeDstCorrection(time_complex_t* xInTime, time_complex_t* xOutTime, int8_t xLocaltimeZoneHours, int8_t xLocaltimeZoneMinutes, uint8_t xInDstActive);
#endif /* TIME_H_ */
