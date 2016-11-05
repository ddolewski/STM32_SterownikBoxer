/*
 * debug.c
 *
 *  Created on: 29 maj 2015
 *      Author: Doles
 */

#include "debug.h"

//-----------------------------------------------------------------------------
// FUNKCJA WYSYLA BAJT PRZEZ UART6
//-----------------------------------------------------------------------------
void _printc(uint8_t xTxByte)
{
	USARTx_SendChar(USART_COMM, xTxByte);
}

//-----------------------------------------------------------------------------
// Wypisanie stringu xString na konsole
//-----------------------------------------------------------------------------
void _print(const uint8_t * xString)
{
//	while (*xString)
//	{
//		USARTx_SendChar(*xString++);
//	}
//	ErrorStatus error = ERROR;
	while (*xString != 0) // wysy³am tyle razy ile znaków jest w stringu
	{
		USARTx_SendChar(USART_COMM, *xString); // wysy³am aktualny znak ze stringa
		xString++; // inkrementujê po kolei ka¿dy znak 'char' z których sk³ada siê string
	}

//	return error;

//	USARTx_SendString(USART_COMM, xString);
}
//-----------------------------------------------------------------------------
// Funkcja debugujaca wypisujaca linie pozioma
//-----------------------------------------------------------------------------
void _printLine(void)
{
	_print(UC"_________________________________________________________________\r\n");
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
		USARTx_SendChar(USART_COMM, '-');
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
				USARTx_SendChar(USART_COMM, '0' + temp);
		}

		temp = xValue % 10;
		USARTx_SendChar(USART_COMM, '0' + temp);
	}
	else
	{
		USARTx_SendChar(USART_COMM, '0');
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
	USARTx_SendChar(USART_COMM, tmp);
	tmp = xByte & 0x0F;
	tmp = tmp < 10 ? '0' + tmp : 'A' + tmp - 10;
	USARTx_SendChar(USART_COMM, tmp);
}

//-----------------------------------------------------------------------------
// Wypisanie parametu na konsole: stringu xString i wartosci xValue
//-----------------------------------------------------------------------------
void _printParam(uint8_t * xString, int32_t xValue)
{
	_print(UC"\r\n");
	_print(xString);
	_print(UC": ");
	_printInt(xValue);
}

//-----------------------------------------------------------------------------
// Wypisanie bledu w formacie "e: " + xString
//-----------------------------------------------------------------------------
void _error(const uint8_t * xString)
{
	_print(UC"\r\ne: ");
	_print(xString);
}

//-----------------------------------------------------------------------------
// Kontynuacja wypisywania bledu
//-----------------------------------------------------------------------------
void _errorc(const uint8_t * xString)
{
	_print(xString);
}

//-----------------------------------------------------------------------------
// Wypisanie informacji w formacie "i: " + xString
//-----------------------------------------------------------------------------
void _info(const uint8_t * xString)
{
	_print(UC"\r\ni: ");
	_print(xString);
}

//-----------------------------------------------------------------------------
// Kontynuacja wypisywania informacji
//-----------------------------------------------------------------------------
void _infoc(const uint8_t * xString)
{
	_print(xString);
}

//-----------------------------------------------------------------------------
// Funkcja wysylajaca pojedynczy znak wybranym modulem USART
// USARTx/in: wskaznik na strukture USART, USART1..USART5
// data/in: pojedynczy znak do wyslania
// return: status bledu po przekroczeniu timeoutu
//-----------------------------------------------------------------------------
ErrorStatus USARTx_SendChar(USART_TypeDef * USARTx, uint8_t data)
{
	uint32_t timeout = 1000000;
	USARTx->TDR = (data & 0xFF);
	// czekam na wyzerowanie flagi TXE po za³adowaniu danych do Data Register
	while ((USARTx->ISR & USART_ISR_TXE) == 0)
	{
		if (timeout > 0)
		{
			timeout--;
		}
		else
		{
			return ERROR;
		}
	}

	return SUCCESS;
}

//-----------------------------------------------------------------------------
// Funkcja wysylajaca string wybranym modulem USART
// USARTx/in: wskaznik na strukture USART, USART1..USART5
// string/in: wskaznik na string do wyslania
// return: status bledu po przekroczeniu timeoutu
//-----------------------------------------------------------------------------
ErrorStatus USARTx_SendString(USART_TypeDef *USARTx, uint8_t *string)
{
	ErrorStatus error = ERROR;
	while (*string != 0) // wysy³am tyle razy ile znaków jest w stringu
	{
		error = USARTx_SendChar(USARTx, *string); // wysy³am aktualny znak ze stringa
		string++; // inkrementujê po kolei ka¿dy znak 'char' z których sk³ada siê string
	}

	return error;
}
