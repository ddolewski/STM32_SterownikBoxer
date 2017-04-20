/*
 * debug.c
 *
 *  Created on: 29 maj 2015
 *      Author: Doles
 */

#include "debug.h"
#include "fifo.h"

#define DEBUG_TX			GPIO_Pin_6
#define DEBUG_BUFF_SIZE		128


volatile fifo_t debug_fifo;
volatile char DebugBuffer[DEBUG_BUFF_SIZE] = "DMA Transmit test (UART1)\r\n";

void DEBUG_Init(void)
{
    fifo_init(&debug_fifo, (void *)DebugBuffer, TX_BUFF_SIZE);

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
//    DMA_InitTypeDef DMA_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);

//	DMA_DeInit(DMA1_Channel2);
//	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->TDR;
//	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&DebugBuffer;
//	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
//	DMA_InitStructure.DMA_BufferSize = DEBUG_BUFF_SIZE;
//	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
//	DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;
//	DMA_Init(DMA1_Channel2, &DMA_InitStructure);
//
//	DMA_Cmd(DMA1_Channel2, ENABLE);
//	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);

//	NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1); //to samo co uart2
//	NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_USART1EN, ENABLE);
    RCC_USARTCLKConfig(RCC_USART1CLK_SYSCLK);

    USART_InitStructure.USART_BaudRate = 230400;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx;

    /* Connect PXx to USARTx_Tx */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_0);

    GPIO_InitStructure.GPIO_Pin = DEBUG_TX; //PB6
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_Init(USART1, &USART_InitStructure);

	NVIC_SetPriority(USART1_IRQn, 3);
	NVIC_EnableIRQ(USART1_IRQn);
//	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    /* Enable USART */
    USART_Cmd(USART1, ENABLE);

}

void DMA1_Channel2_3_IRQnHandler(void)
{
	/* Test on DMA Transfer Complete interrupt */
	if (DMA_GetITStatus(DMA1_IT_TC2) != RESET)
	{
		/* Clear DMA Transfer Complete interrupt pending bit */
		DMA_ClearITPendingBit(DMA1_IT_TC2);
	}
}

void DEBUG_SendString(char * xString)
{
#ifdef DEBUG_TERMINAL_USART
	fifo_write(&debug_fifo, xString, strlen(xString));
	USART_ITConfig( USART1, USART_IT_TXE, ENABLE );
#endif
}

void DEBUG_SendByte(uint8_t xData)
{
#ifdef DEBUG_TERMINAL_USART
	fifo_write(&debug_fifo, &xData, 1);
	USART_ITConfig( USART1, USART_IT_TXE, ENABLE );
#endif
}

void USART1_IRQHandler(void)
{
	char txChar = 0;

	if ( USART_GetITStatus( USART1, USART_IT_TXE ) != RESET )
	{
		if ( fifo_read(&debug_fifo, &txChar, 1) != 0) //zdejmij kolejny element z kolejki
		{
			USART_SendData( USART1, txChar );
		}
		else
		{
			USART_ITConfig( USART1, USART_IT_TXE, DISABLE ); //kolejka jest pusta, wylaczamy przerwanie i przestajemy nadawac
		}

		USART_ClearITPendingBit(USART1, USART_IT_TXE);
	}
}
//-----------------------------------------------------------------------------
// FUNKCJA WYSYLA BAJT PRZEZ UART6
//-----------------------------------------------------------------------------
void _printChar(uint8_t xTxByte)
{
	DEBUG_SendByte(xTxByte);
}

//-----------------------------------------------------------------------------
// Wypisanie stringu xString na konsole
//-----------------------------------------------------------------------------
void _printString(const uint8_t * xString)
{
	DEBUG_SendString(xString);
}
//-----------------------------------------------------------------------------
// Funkcja debugujaca wypisujaca linie pozioma
//-----------------------------------------------------------------------------
void _printLine(void)
{
	_printString(UC"_________________________________________________________________\r\n");
}
//-----------------------------------------------------------------------------
// Wypisanie wartosci dziesietnej ze znakiem xValue na konsole
//-----------------------------------------------------------------------------
void _printInt(int32_t xValue)
{
	uint32_t i = 1000000000;
	uint32_t temp, zeros=0;

	if (xValue < 0)
	{
		DEBUG_SendByte('-');
		xValue *= -1;
	}

	if (xValue)
	{
		for (i = 1000000000; i > 1; i /= 10)
		{
			temp = xValue / i;
			temp %= 10;
			if (temp)
				zeros = 1;
			if (temp != 0 || zeros != 0)
				DEBUG_SendByte('0' + temp);
		}

		temp = xValue % 10;
		DEBUG_SendByte('0' + temp);
	}
	else
	{
		DEBUG_SendByte('0');
	}

//	_print(UC"\n\r");
}
//-----------------------------------------------------------------------------
// Wypisanie bajtu xByte szesnastkowo
//-----------------------------------------------------------------------------
void _printHex(uint8_t xByte)
{
	uint8_t tmp;
	tmp = xByte>>4;
	tmp = tmp < 10 ? '0' + tmp : 'A' + tmp - 10;
	DEBUG_SendByte(tmp);
	tmp = xByte & 0x0F;
	tmp = tmp < 10 ? '0' + tmp : 'A' + tmp - 10;
	DEBUG_SendByte(tmp);
}

//-----------------------------------------------------------------------------
// Wypisanie parametu na konsole: stringu xString i wartosci xValue
//-----------------------------------------------------------------------------
void _printParam(uint8_t * xString, int32_t xValue)
{
	_printString(UC"\r\n");
	_printString(xString);
	_printString(UC": ");
	_printInt(xValue);
}

//-----------------------------------------------------------------------------
// Wypisanie bledu w formacie "e: " + xString
//-----------------------------------------------------------------------------
void _error(const uint8_t * xString)
{
	_printString(UC"\r\ne: ");
	_printString(xString);
}

//-----------------------------------------------------------------------------
// Kontynuacja wypisywania bledu
//-----------------------------------------------------------------------------
void _errorc(const uint8_t * xString)
{
	_printString(xString);
}

//-----------------------------------------------------------------------------
// Wypisanie informacji w formacie "i: " + xString
//-----------------------------------------------------------------------------
void _info(const uint8_t * xString)
{
	_printString(UC"\r\ni: ");
	_printString(xString);
}

//-----------------------------------------------------------------------------
// Kontynuacja wypisywania informacji
//-----------------------------------------------------------------------------
void _infoc(const uint8_t * xString)
{
	_printString(xString);
}
