/*
 * debug.h
 *
 *  Created on: 29 maj 2015
 *      Author: Doles
 */

#ifndef MY_INC_DEBUG_H_
#define MY_INC_DEBUG_H_

#include "stm32f0xx_usart.h"
#include "boxer_communication.h"

void DEBUG_Init(void);
void DEBUG_SendString(char * xString);
void DEBUG_SendByte(uint8_t xData);

void _printChar(uint8_t xTxByte);
void _printString(const uint8_t * xString);
void _printLine(void);
void _printInt(int32_t xValue);
void _printHex(uint8_t xByte);
void _printParam(uint8_t * xString, int32_t xValue);

void _error(const uint8_t * xString);
void _errorc(const uint8_t * xString);

void _info(const uint8_t * xString);
void _infoc(const uint8_t * xString);


#define _paramPrint(X)			do{_info(UC(#X));\
		   	   	   	   	   	   	   _print(UC" = ");\
		   	   	   	   	   	   	   _printInt(X);\
								}while(0)

#endif /* MY_INC_DEBUG_H_ */
