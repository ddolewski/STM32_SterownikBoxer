#include "fifo.h"

//This initializes the FIFO structure with the given buffer and size
void fifo_init(volatile fifo_t * fifo, char * buf, int size)
{
	fifo->head = 0;
	fifo->tail = 0;
	fifo->size = size;
	fifo->buf = buf;
}

void fifo_flush(volatile fifo_t *  flushFifo)
{
	flushFifo->head = 0;
	flushFifo->tail = 0;
	flushFifo->size = RX_BUFF_SIZE;
	memset(flushFifo->buf, 0, RX_BUFF_SIZE);
}

//This reads nbytes bytes from the FIFO
//The number of bytes read is returned
int fifo_read(volatile fifo_t * fifo, void * buf, int nbytes)
{
	int i;
	char * p;
	p = buf;
	for (i = 0; i < nbytes; i++)
	{
		if ( fifo->tail != fifo->head )
		{
			//see if any data is available
			*p++ = fifo->buf[fifo->tail];  	//grab a byte from the buffer
			fifo->tail++; 	 				//increment the tail
			if ( fifo->tail == fifo->size )	//check for wrap-around
			{
				fifo->tail = 0;
			}
		}
		else
		{
			return i; //number of bytes read
		}
	}

	return nbytes;
}

//This writes up to nbytes bytes to the FIFO
//If the head runs in to the tail, not all bytes are written
//The number of bytes written is returned
int fifo_write(volatile fifo_t * fifo, const void * buf, int nbytes)
{
	int i;
	const char * p;
	p = buf;

	for (i = 0; i < nbytes; i++)
	{
		//first check to see if there is space in the buffer
		if ((fifo->head + 1 == fifo->tail) || ((fifo->head + 1 == fifo->size) && (fifo->tail == 0)))
		{
			return i; //no more room
		}
		else
		{
			fifo->buf[fifo->head] = *p++;
			fifo->head++;  //increment the head
			if ( fifo->head == fifo->size ) //check for wrap-around
			{
				fifo->head = 0;
			}
		}
	}

	return nbytes;
}
