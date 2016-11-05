/*
 * boxer_string.h
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_STRING_H_
#define MY_INC_BOXER_STRING_H_

#include "global.h"
#include "boxer_struct.h"

void PrepareUdpString(uint32_t Lux, float realHum, float tempC, float ds18b20Temp1, float ds18b20Temp2, char * xOutString);
void MakeDateString(char* TimeString, date_t * rtcDate);

#endif /* MY_INC_BOXER_STRING_H_ */
