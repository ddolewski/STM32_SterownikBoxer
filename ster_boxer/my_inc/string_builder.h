#ifndef __string_builder
#define __string_builder

#include "stdint.h"
#define	True		1
#define False 		0

void append(char * s, char c);
char* ftoa(float xNumber, char * xBuffer, uint8_t xAccuracy);
char* itoa(int xNumber, char * xBuffer);
uint32_t atoi(char *str);
//int itoa(int xValue, char *xString, int xRadix)


#endif
