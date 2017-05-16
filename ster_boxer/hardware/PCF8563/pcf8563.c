/*
 * pcf8583.c
 *
 * Created: 2016-05-04 19:00:46
 *  Author: Doles
 */ 

#include <timestamp.h>
#include "pcf8563.h"
#include "boxer_display.h"
#include "boxer_string.h"

static uint8_t PCF8563_RegRead(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, ErrorStatus * Error);
static ErrorStatus PCF8563_RegWrite(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, uint8_t data);
static int BcdToDec(int data);
static int DecToBcd(int data);

static time_complex_t localTime;

ErrorStatus PCF8563_Init(I2C_TypeDef * I2Cx)
{
	ErrorStatus error = SUCCESS;
	uint8_t temp = ((1<<7) | (0x03<<0));

	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, 0x0D, temp);
	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, PCF8563_CTRL_STAT_REG1, 0x08);
	return error;
}

ErrorStatus PCF8563_ReadTime(time_complex_t * xTime, I2C_TypeDef* I2Cx)
{
	ErrorStatus error = SUCCESS;
	uint8_t tmp_dta;
	uint8_t regAddr;

	for (regAddr = 2; regAddr < 9; regAddr++)
	{
		tmp_dta = PCF8563_RegRead(I2Cx, PCF8563_ADDR, regAddr, &error);

		if (error != ERROR)
		{
			switch (regAddr)
			{
				case 2: xTime->sec 	    = BcdToDec(tmp_dta & 0x7F); break;
				case 3: xTime->min 	    = BcdToDec(tmp_dta & 0x7F); break;
				case 4: xTime->hour 	= BcdToDec(tmp_dta & 0x3F); break;
				case 5: xTime->mday     = BcdToDec(tmp_dta & 0x3F); break;
				case 6: xTime->wday 	= BcdToDec(tmp_dta & 0x07); break;
				case 7: xTime->month 	= BcdToDec(tmp_dta & 0x1F); break;
				case 8: xTime->year 	= BcdToDec(tmp_dta); break;
			}
		}
		else
		{
//			USARTx_SendString(USART_COMM, "pcf8563 readtime error\n\r");
		}
	}
	
	//UART_PutInt(xTime->year, 10);
	xTime->year += 2000;
	return error;
}

ErrorStatus PCF8563_WriteTime(time_complex_t * xTime, I2C_TypeDef * I2Cx)
{
	time_complex_t time = *xTime;
	time.year 	-= 2000;
	time.wday	= timeCalcDayWeek(&time);
	ErrorStatus error = SUCCESS;

	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, PCF8563_CTRL_STAT_REG1, 0x08);
	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, PCF8563_CTRL_STAT_REG2, 0x00);
	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, PCF8563_SEC_REG, time.sec);
	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, PCF8563_MIN_REG, time.min);
	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, PCF8563_HOU_REG, time.hour);
	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, PCF8563_MDAY_REG, time.mday);
	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, PCF8563_WDAY_REG, time.wday);
	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, PCF8563_MON_REG, time.month);
	error = PCF8563_RegWrite(I2Cx, PCF8563_ADDR, PCF8563_YEAR_REG, time.year);

	return error;
}

void RTC_Handler(void)
{
#ifndef DEBUG_TERMINAL_USART
		PCF8563_ReadTime(&xRtcFullDate, I2C1);
#endif
		timeUtcToLocalConv(&xRtcFullDate, &localTime);
		displayMakeTimeString(xTimeString, &localTime);
		displayMakeDateString(xDateString, &localTime);
		displayWeekDayConvert(localTime.wday, weekDayString);

		strcpy(displayData.time, xTimeString);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static uint8_t PCF8563_RegRead(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, ErrorStatus * Error)
{
	uint32_t TimeOut = 10000;
	uint8_t ReadValue = 0;

	I2C_NumberOfBytesConfig(I2Cx, 1);
	I2C_SlaveAddressConfig(I2Cx, SlaveAddr);
	I2C_MasterRequestConfig(I2Cx, I2C_Direction_Transmitter);

	I2C_GenerateSTART(I2Cx, ENABLE);

	while(!I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS))
	{
		if(TimeOut > 0)
		{
			TimeOut--;
		}
		else
		{
			*Error = ERROR;
			return 1;
		}
	}

	TimeOut = 10000;
	I2C_SendData(I2Cx, RegisterAddr);

	while(!I2C_GetFlagStatus(I2Cx, I2C_ISR_TC))
	{
		if(TimeOut > 0)
		{
			TimeOut--;
		}
		else
		{
			*Error = ERROR;
			return 1;
		}
	}

	I2C_NumberOfBytesConfig(I2Cx, 1);
	I2C_MasterRequestConfig(I2Cx, I2C_Direction_Receiver);

	TimeOut = 10000;
	I2C_GenerateSTART(I2Cx, ENABLE);

	while(!I2C_GetFlagStatus(I2Cx, I2C_FLAG_RXNE))
	{
		if(TimeOut > 0)
		{
			TimeOut--;
		}
		else
		{
			*Error = ERROR;
			return 1;
		}
	}

	TimeOut = 10000;
	ReadValue = I2C_ReceiveData(I2Cx);

	I2C_GenerateSTOP(I2Cx, ENABLE);

	while(!I2C_GetFlagStatus(I2Cx, I2C_ISR_STOPF))
	{
		if(TimeOut > 0)
		{
			TimeOut--;
		}
		else
		{
			*Error = ERROR;
			return 1;
		}
	}

	return ReadValue;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static ErrorStatus PCF8563_RegWrite(I2C_TypeDef * I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, uint8_t data)
{
	uint32_t TimeOut = 10000;
	uint8_t TimeBCD = 0;
	TimeBCD = DecToBcd(data);

	I2C_NumberOfBytesConfig(I2Cx, 2);
	I2C_SlaveAddressConfig(I2Cx, SlaveAddr);
	I2C_MasterRequestConfig(I2Cx, I2C_Direction_Transmitter);

	I2C_GenerateSTART(I2Cx, ENABLE);

	while(!I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS))
	{
		if(TimeOut > 0)
		{
			TimeOut--;
		}
		else
		{
			return ERROR;
		}
	}

	TimeOut = 10000;
	I2C_SendData(I2Cx, RegisterAddr);

	while(!I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS))
	{
		if(TimeOut > 0)
		{
			TimeOut--;
		}
		else
		{
			return ERROR;
		}
	}

	TimeOut = 10000;
	I2C_SendData(I2Cx, TimeBCD);

	while(!I2C_GetFlagStatus(I2Cx, I2C_ISR_TC))
	{
		if(TimeOut > 0)
		{
			TimeOut--;
		}
		else
		{
			return ERROR;
		}
	}

	TimeOut = 10000;
	I2C_GenerateSTOP(I2Cx, ENABLE);

	while(!I2C_GetFlagStatus(I2Cx, I2C_ISR_STOPF))
	{
		if(TimeOut > 0)
		{
			TimeOut--;
		}
		else
		{
			return ERROR;
		}
	}

	return SUCCESS;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int DecToBcd(int data)
{
	return ((data / 10) << 4) + (data % 10);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int BcdToDec(int data)
{
	return (10 * (data >> 4) + (data & 0x0F));
}
