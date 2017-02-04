/*
 * boxer_uart.h
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_COMMUNICATION_H_
#define MY_INC_BOXER_COMMUNICATION_H_

#include "stm32f0xx_gpio.h"
#include "stm32f0xx_usart.h"
#include "boxer_struct.h"
#include "stm32f0xx_rcc.h"
#include "stdint.h"

#define countof(a)             (uint8_t)(sizeof(a) / sizeof(*(a)))

typedef enum
{
	ATNEL_UNINITIALISE,
	ATNEL_SEND_3PLUS,
	ATNEL_RECV_A,
	ATNEL_SEND_A,
	ATNEL_RECV_OK
}atnel_init_state_t;

extern atnel_init_state_t atnelInitProccess;
extern bool_t atnelAtCmdEnable;

void SerialPort_PutChar(char xSendChar);
void SerialPort_PutString(char * xString);

void SerialPort_Init(void);
void ReceiveSerial_Handler(void);
void TransmitSerial_Handler(void);
ErrorStatus AtnelGetTime(char * xStrTime);

#endif /* MY_INC_BOXER_COMMUNICATION_H_ */
