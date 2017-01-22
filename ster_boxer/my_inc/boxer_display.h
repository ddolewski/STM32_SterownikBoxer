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
#include "hardware/LCD_KS0108/KS0108.h"
#include "hardware/LCD_KS0108/glcd_font5x8.h"
#include "hardware/LCD_KS0108/graphic.h"

uint8_t lastTimeOnHour;
uint8_t lastTimeOffHour;
//date_t rtcDate;
time_complex_t rtcFullDate;

char timeString[20];
char dateString[20];
char weekDayString[20];


void Display_Handler(void);
#endif /* MY_INC_BOXER_DISPLAY_H_ */
