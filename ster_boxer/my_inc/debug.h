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

#define USART_COMM 				USART2
#define USART_DEBUG_RCC 		RCC_APB1Periph_USART2
#define USART_DEBUG_RX			GPIO_Pin_3
#define USART_DEBUG_TX			GPIO_Pin_2

ErrorStatus USARTx_SendString(USART_TypeDef *USARTx, uint8_t *string);
ErrorStatus USARTx_SendChar(USART_TypeDef * USARTx, uint8_t data);

#ifndef DEBUG_TERMINAL_USART
//#define _printc(X) 				do {} while(0)
//#define _print(X) 				do {} while(0)
//#define _printLine(X) 			do {} while(0)
//#define _printInt(X) 			do {} while(0)
//#define _printHex(X) 			do {} while(0)
//#define _paramPrint(X) 			do {} while(0)
//
//#define _error(X) 				do {} while(0)
//#define _errorc(X) 				do {} while(0)
//
//#define _info(X) 				do {} while(0)
//#define _infoc(X) 				do {} while(0)

#else

void _printc(uint8_t xTxByte);
void _print(const uint8_t * xString);
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
#endif

#endif /* MY_INC_DEBUG_H_ */
