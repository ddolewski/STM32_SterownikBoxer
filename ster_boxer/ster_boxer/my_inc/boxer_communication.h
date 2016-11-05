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
#define BUFFER_SIZE            (countof(DataFromPC))
#define BUFF_SEND_PRIMARY_SIZE	57	//czyli tylko bajty startu a reszta wypelniona samymi zerami

#define RXSIZE 	256
#define TXSIZE 	128

typedef struct
{
     char * buf;
     int head;
     int tail;
     int size;
} fifo_t;

typedef enum
{
	S_CHAR = 83,
	T_CHAR = 84,
	A_CHAR = 65,
	R_CHAR = 82,
	E_CHAR = 69,
	N_CHAR = 78,
	D_CHAR = 68,
	L_CHAR = 76,
	I_CHAR = 73,
	C_CHAR = 67,
	P_CHAR = 80
}protocol_chars_t;

volatile uint8_t RxBuffIndex;
volatile uint8_t findFirstStartByte;
volatile uint8_t findSecondStartByte;
volatile uint8_t byteReceivedFlag;
volatile char DataFromPC[100];
volatile char DataToSend[100];

volatile char RxBuffer[RXSIZE];
fifo_t RxFifo[1];

volatile char TxBuffer[TXSIZE];
fifo_t TxFifo[1];

int fifo_read(fifo_t * f, void * buf, int nbytes);
int fifo_write(fifo_t * f, const void * buf, int nbytes);

void UART2_Init(void);
void USART_SendString(USART_TypeDef *USARTx, char *string);

void PcCommunication_Handler(void);

#endif /* MY_INC_BOXER_COMMUNICATION_H_ */
