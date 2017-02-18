/*
 * fifo.h
 *
 *  Created on: 2 lut 2017
 *      Author: Doles
 */

#ifndef MY_INC_FIFO_H_
#define MY_INC_FIFO_H_

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
void fifo_flush(fifo_t *  flushFifo);
#endif /* MY_INC_FIFO_H_ */
