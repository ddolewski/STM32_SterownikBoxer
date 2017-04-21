#ifndef MY_INC_BOXER_STRING_H_
#define MY_INC_BOXER_STRING_H_

#include "global.h"
#include "timestamp.h"

void PrepareUdpString(uint32_t Lux, float realHum, float tempC, float ds18b20Temp1, float ds18b20Temp2, char * xOutString);
void MakeDateString(char * TimeString, time_complex_t * rtc_time);
void displayWeekDayConvert(uint8_t xWeekDayNum, char * xStrDay);
void displayMakeDateString(char * xDateString, time_complex_t * xRtcDate);
void displayMakeTimeString(char * TimeString, time_complex_t * xRtcTime);

#endif /* MY_INC_BOXER_STRING_H_ */
