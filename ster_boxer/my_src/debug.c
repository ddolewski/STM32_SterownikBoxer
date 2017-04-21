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

static void DEBUG_SendString(char * xString);
static void DEBUG_SendByte(uint8_t xData);

volatile fifo_t debug_fifo;
volatile char DebugBuffer[DEBUG_BUFF_SIZE] = {0};

void DEBUG_Init(void)
{
    fifo_init(&debug_fifo, (void *)DebugBuffer, TX_BUFF_SIZE);

    GPIO_InitTypeDef GPIO_InitStrture;
    USART_InitTypeDef USART_InitStrture;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_USART1EN, ENABLE);
    RCC_USARTCLKConfig(RCC_USART1CLK_SYSCLK);

    USART_InitStrture.USART_BaudRate = 230400;
    USART_InitStrture.USART_WordLength = USART_WordLength_8b;
    USART_InitStrture.USART_StopBits = USART_StopBits_1;
    USART_InitStrture.USART_Parity = USART_Parity_No;
    USART_InitStrture.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStrture.USART_Mode = USART_Mode_Tx;

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_0);

    GPIO_InitStrture.GPIO_Pin = DEBUG_TX; //PB6
    GPIO_InitStrture.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStrture.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStrture.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStrture.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStrture);

    USART_Init(USART1, &USART_InitStrture);

	NVIC_SetPriority(USART1_IRQn, 3);
	NVIC_EnableIRQ(USART1_IRQn);
    USART_Cmd(USART1, ENABLE);
}

static void DEBUG_SendString(char * xString)
{
#ifdef DEBUG_TERMINAL_USART
	fifo_write(&debug_fifo, xString, strlen(xString));
	USART_ITConfig( USART1, USART_IT_TXE, ENABLE );
#endif
}

static void DEBUG_SendByte(uint8_t xData)
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
void _printString(char * xString)
{
	DEBUG_SendString(xString);
}
//-----------------------------------------------------------------------------
// Funkcja debugujaca wypisujaca linie pozioma
//-----------------------------------------------------------------------------
void _printLine(void)
{
	_printString("_________________________________________________________________\r\n");
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

//	_print("\n\r");
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
void _printParam(char * xString, int32_t xValue)
{
	_printString("\r\n");
	_printString(xString);
	_printString(": ");
	_printInt(xValue);
}

//-----------------------------------------------------------------------------
// Wypisanie bledu w formacie "e: " + xString
//-----------------------------------------------------------------------------
void _error(char * xString)
{
	_printString("\r\ne: ");
	_printString(xString);
}

//-----------------------------------------------------------------------------
// Kontynuacja wypisywania bledu
//-----------------------------------------------------------------------------
void _errorc(char * xString)
{
	_printString(xString);
}

//-----------------------------------------------------------------------------
// Wypisanie informacji w formacie "i: " + xString
//-----------------------------------------------------------------------------
void _info(char * xString)
{
	_printString("\r\ni: ");
	_printString(xString);
}

//-----------------------------------------------------------------------------
// Kontynuacja wypisywania informacji
//-----------------------------------------------------------------------------
void _infoc(char * xString)
{
	_printString(xString);
}
