/*
 * boxer_display.h
 *
 *  Created on: 5 sie 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_DISPLAY_H_
#define MY_INC_BOXER_DISPLAY_H_

#include "stdint.h"
#include "string.h"
#include "boxer_struct.h"
#include "string_builder.h"
#include "timestamp.h"
#include "KS0108.h"
#include "glcd_font5x8.h"
#include "graphic.h"

volatile lcdDisplayData_t displayData;

uint8_t xLastTimeOnHour;
uint8_t xLastTimeOffHour;
time_complex_t xRtcFullDate;

float xLastWaterPh;
float xLastSoilPh;

char xTimeString[20];
char xDateString[20];
char weekDayString[20];

void Display_Handler(void);
#endif /* MY_INC_BOXER_DISPLAY_H_ */
