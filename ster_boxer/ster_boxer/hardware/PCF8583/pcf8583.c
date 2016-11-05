#include "pcf8583.h"

static systime_t readTimeTimer = 0;

void I2C1_Init(void)
{
	GPIOx_ClockConfig(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource6, GPIOx_AF_1); //scl
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource7, GPIOx_AF_1); //sda
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C1_SCL);
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C1_SDA);

	I2C_InitTypeDef  I2C_InitStructure;
	RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0xEC;
	I2C_InitStructure.I2C_Timing = 0x00701863;//0x10805E89; //0x40B22536; //100khz
	I2C_Init(I2C1, &I2C_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_SoftwareResetCmd(I2C1, ENABLE);
	I2C_SoftwareResetCmd(I2C1, DISABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_Cmd(I2C1, ENABLE);

	I2C_SoftwareResetCmd(I2C1, ENABLE);
	I2C_SoftwareResetCmd(I2C1, DISABLE);
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
uint8_t PCF8583_RegRead(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, ErrorStatus * Error)
{
	uint32_t TimeOut = 100000;

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

	TimeOut = 100000;
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

	TimeOut = 100000;
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

	TimeOut = 100000;
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

	ConvertValue = BcdToDec(ReadValue);
	return ConvertValue;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorStatus PCF8583_RegWrite(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, uint8_t data)
{
	uint32_t TimeOut = 100000;
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

	TimeOut = 100000;
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

	TimeOut = 100000;
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

	TimeOut = 100000;
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PCF8583_SetDate(I2C_TypeDef* I2Cx, date_t * rtcDate)
{
	rtcDate->errorInfo = PCF8583_RegWrite(I2Cx, PCF8583_ADDR, PCF8583_SEC_REG, rtcDate->sec);
	rtcDate->errorInfo = PCF8583_RegWrite(I2Cx, PCF8583_ADDR, PCF8583_MIN_REG, rtcDate->min);
	rtcDate->errorInfo = PCF8583_RegWrite(I2Cx, PCF8583_ADDR, PCF8583_HOU_REG, rtcDate->hour);
	rtcDate->errorInfo = PCF8583_RegWrite(I2Cx, PCF8583_ADDR, PCF8583_DAY_REG, rtcDate->day);
	rtcDate->errorInfo = PCF8583_RegWrite(I2Cx, PCF8583_ADDR, PCF8583_MON_REG, rtcDate->month);
}

void PCF8583_ReadDate(I2C_TypeDef* I2Cx, date_t * rtcDate)
{
	if (systimeTimeoutControl(&readTimeTimer, 400))
	{
		readDataFlag = 1;
		rtcDate->sec = PCF8583_RegRead(I2Cx, PCF8583_ADDR, PCF8583_SEC_REG, &rtcDate->errorInfo);
		rtcDate->min = PCF8583_RegRead(I2Cx, PCF8583_ADDR, PCF8583_MIN_REG, &rtcDate->errorInfo);
		rtcDate->hour = PCF8583_RegRead(I2Cx, PCF8583_ADDR, PCF8583_HOU_REG, &rtcDate->errorInfo);
		readDataFlag = 0;
	}
}
