#ifndef MY_INC_BOXER_DISPLAY_H_
#define MY_INC_BOXER_DISPLAY_H_

#include "stdint.h"
#include "string.h"
#include "string_builder.h"
#include "timestamp.h"
#include "KS0108.h"
#include "glcd_font5x8.h"
#include "graphic.h"

typedef struct
{
	float tempSHT2x;
	float tempDS18B20_1_t;
	float tempDS18B20_2_t;
	uint32_t lux;
	float humiditySHT2x;
	char time[20];
	float ph1;
	float ph2;
	uint8_t page;
	uint8_t pageCounter;
}lcdDisplayData_t;

volatile lcdDisplayData_t displayData;

uint8_t xLastTimeOnHour;
uint8_t xLastTimeOffHour;
time_complex_t xRtcFullDate;

char xTimeString[20];
char xDateString[20];
char weekDayString[20];

void Display_Handler(void);
#endif /* MY_INC_BOXER_DISPLAY_H_ */
