#ifndef MY_INC_DEBUG_H_
#define MY_INC_DEBUG_H_

#include "stm32f0xx_usart.h"
#include "boxer_communication.h"

void DEBUG_Init(void);

void _printChar(uint8_t xTxByte);
void _printString(char * xString);
void _printLine(void);
void _printInt(int32_t xValue);
void _printHex(uint8_t xByte);
void _printParam(char * xString, int32_t xValue);

void _error(char * xString);
void _errorc(char * xString);

void _info(char * xString);
void _infoc(char * xString);


#define _paramPrint(X)			do{_info(UC(#X));\
		   	   	   	   	   	   	   _print(UC" = ");\
		   	   	   	   	   	   	   _printInt(X);\
								}while(0)

#endif /* MY_INC_DEBUG_H_ */
