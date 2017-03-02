/*
 * pcf8583.c
 *
 * Created: 2016-05-04 19:00:46
 *  Author: Doles
 */ 

#include <timestamp.h>
#include "pcf8563.h"

static systime_t readTimeTimer = 0;
static time_complex_t localTime;
//-----------------------------------------------------------------------------
// Funkcja konwertujaca bin na bcd
// byte_to_conv/in: konwertowany bajt
// return: wartosc bcd
//-----------------------------------------------------------------------------
uint8_t miscBin2bcd(uint8_t byte_to_conv)
{
	uint8_t byte_converted;
	byte_converted = byte_to_conv/10;
	byte_converted <<= 4;
	byte_to_conv %= 10;
	byte_converted += byte_to_conv;
	return byte_converted;
}

//-----------------------------------------------------------------------------
// Funkcja konwertujaca bcd na bin
// byte_to_conv/in: konwertowana wartocd bcd
// return: wartosc bajtu
//-----------------------------------------------------------------------------
uint8_t miscBcd2bin(uint8_t byte_to_conv)
{
	uint8_t byte_converted;
	byte_converted = byte_to_conv & 0x0F;
	byte_to_conv >>= 4;
	byte_to_conv *= 10;
	byte_converted += byte_to_conv;
	return byte_converted;
}
	
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
	if (systimeTimeoutControl(&readTimeTimer, 400))
	{
#ifndef DEBUG_TERMINAL_USART
		PCF8563_ReadTime(&rtcFullDate, I2C1);
#endif
		timeUtcToLocalConv(&rtcFullDate, &localTime);
		displayMakeTimeString(timeString, &localTime);
		displayMakeDateString(dateString, &localTime);
		displayWeekDayConvert(localTime.wday, weekDayString);

		strcpy(displayData.time, timeString);
	}
}

void I2C1_Init(void)
{
	GPIOx_ClockConfig(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource6, GPIOx_AF_1); //scl
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource7, GPIOx_AF_1); //sda
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C1_SCL);
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C1_SDA);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_InitTypeDef  I2C_InitStructure;
	RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);

	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0xEC;
	I2C_InitStructure.I2C_Timing = 0x00901850;//0x00701863;//0x10805E89; //0x40B22536; //100khz
	I2C_Init(I2C1, &I2C_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_SoftwareResetCmd(I2C1, ENABLE);
	I2C_SoftwareResetCmd(I2C1, DISABLE);

//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
//
//	I2C_Cmd(I2C1, ENABLE);
//
//	I2C_SoftwareResetCmd(I2C1, ENABLE);
//	I2C_SoftwareResetCmd(I2C1, DISABLE);
}

void I2C2_Init(void)
{
	GPIOx_ClockConfig(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource10, GPIOx_AF_1); //scl
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource11, GPIOx_AF_1); //sda
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C2_SCL);
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C2_SDA);

	I2C_InitTypeDef  I2C_InitStructure;
//	RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0x0F;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0xAB;
	I2C_InitStructure.I2C_Timing = 0xC0106DFF;//0x502044F3;//0x10805E89; //0x40B22536; //100khz
	I2C_Init(I2C2, &I2C_InitStructure);

//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	I2C_SoftwareResetCmd(I2C2, ENABLE);
	I2C_SoftwareResetCmd(I2C2, DISABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	I2C_Cmd(I2C2, ENABLE);

	I2C_SoftwareResetCmd(I2C2, ENABLE);
	I2C_SoftwareResetCmd(I2C2, DISABLE);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t PCF8563_RegRead(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, ErrorStatus * Error)
{
	uint32_t TimeOut = 10000;

	uint8_t ReadValue = 0;
	uint8_t ConvertValue = 0;

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

//	ConvertValue = BcdToDec(ReadValue);
	return ReadValue;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorStatus PCF8563_RegWrite(I2C_TypeDef * I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, uint8_t data)
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
	I2C_SendData(I2Cx, RegisterAddr); // PCF time register

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
int DecToBcd(int data)
{
	return ((data / 10) << 4) + (data % 10);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int BcdToDec(int data)
{
	return (10 * (data >> 4) + (data & 0x0F));
}
