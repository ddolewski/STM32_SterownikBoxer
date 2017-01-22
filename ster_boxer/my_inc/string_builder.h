#ifndef __string_builder
#define __string_builder

#include "stdint.h"
#define	True		1
#define False 		0

char* ftoa(float xNumber, char * xBuffer, uint8_t xAccuracy);
char* itoa(int xNumber, char * xBuffer);
uint32_t atoi(uint8_t *str);
//int itoa(int xValue, char *xString, int xRadix)


#endif
