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

#define TX_BUFF_SIZE	128
#define RX_BUFF_SIZE	128

typedef struct {
     char * buf;
     int head;
     int tail;
     int size;
} fifo_t;

void fifo_init(volatile fifo_t * fifo, char * buf, int size);
int fifo_read(volatile fifo_t * fifo, void * buf, int nbytes);
int fifo_write(volatile fifo_t * fifo, const void * buf, int nbytes);



void SerialPort_PutChar(char xSendChar);
void SerialPort_PutString(char * xString);

void SerialPort_Init(void);
void ReceiveSerial_Handler(void);

#endif /* MY_INC_BOXER_COMMUNICATION_H_ */
