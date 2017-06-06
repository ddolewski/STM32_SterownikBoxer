#include "tsl2561.h"
#include "systime.h"
#include "debug.h"

static ErrorStatus TSL2561_PowerOn(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, bool_t powerOn);
static ErrorStatus TSL2561_Config(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t intergrationTimeGain);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorStatus TSL2561_Init(void)
{
	ErrorStatus tslError = SUCCESS;
	tslError = TSL2561_PowerOn(I2C2, TSL2561_GND_ADDR, TRUE);
	systimeDelayMs(30);
	tslError = TSL2561_Config(I2C2, TSL2561_GND_ADDR, DefaultIntegrationTimeAndGain);
	systimeDelayMs(20);

	return tslError;
}

static ErrorStatus TSL2561_PowerOn(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, bool_t powerOn)
{
	uint32_t TimeOut = 1000000;

	while (I2C_GetFlagStatus(I2Cx, I2C_ISR_BUSY) != RESET)
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

	I2C_NumberOfBytesConfig(I2Cx, 2);
	I2C_SlaveAddressConfig(I2Cx, SlaveAddr);
	I2C_MasterRequestConfig(I2Cx, I2C_Direction_Transmitter);

	I2C_GenerateSTART(I2Cx, ENABLE);

	TimeOut = 100000;
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
	I2C_SendData(I2Cx, ControlRegiser);

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
	if (powerOn == TRUE)
	{
		I2C_SendData(I2Cx, TurnOnBits);
	}
	else
	{
		I2C_SendData(I2Cx, TurnOffBits);
	}

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
static ErrorStatus TSL2561_Config(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t intergrationTimeGain)
{
	uint32_t TimeOut = 1000000;

	while (I2C_GetFlagStatus(I2Cx, I2C_ISR_BUSY) != RESET)
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
	I2C_SendData(I2Cx, TimingRegister);

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
	I2C_SendData(I2Cx, intergrationTimeGain);

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
uint8_t TSL2561_ReadByte(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, ErrorStatus * Error)
{
	uint32_t TimeOut = 1000000;
	uint8_t ReadValue = 0;

	while (I2C_GetFlagStatus(I2Cx, I2C_ISR_BUSY) != RESET)
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

	*Error = SUCCESS;
	return ReadValue;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorStatus TSL2561_ReadLux(uint32_t * xLux)
{
	ErrorStatus Error = SUCCESS;
	uint32_t Lux = 0;
	uint8_t Data0Low_uchar  = 0;
	uint8_t Data0High_uchar = 0;
	uint8_t Data1Low_uchar  = 0;
	uint8_t Data1High_uchar = 0;
	uint32_t Data0Low_uint  = 0;
	uint32_t Data0High_uint = 0;
	uint32_t Data1Low_uint  = 0;
	uint32_t Data1High_uint = 0;
	uint32_t Channel0_uint  = 0;
	uint32_t Channel1_uint  = 0;

	Data0Low_uchar  = TSL2561_ReadByte(I2C2, TSL2561_GND_ADDR, Data0LowByteMode,  &Error);
	if (Error == ERROR)
	{
		return 1;
	}

	Data0High_uchar = TSL2561_ReadByte(I2C2, TSL2561_GND_ADDR, Data0HighByteMode, &Error);
	if (Error == ERROR)
	{
		return 1;
	}

	Data1Low_uchar  = TSL2561_ReadByte(I2C2, TSL2561_GND_ADDR, Data1LowByteMode,  &Error);
	if (Error == ERROR)
	{
		return 1;
	}

	Data1High_uchar = TSL2561_ReadByte(I2C2, TSL2561_GND_ADDR, Data1HighByteMode, &Error);
	if (Error == ERROR)
	{
		return 1;
	}

	Data0Low_uint  = (uint32_t)Data0Low_uchar;
	Data0High_uint = (uint32_t)Data0High_uchar;
	Data1Low_uint  = (uint32_t)Data1Low_uchar;
	Data1High_uint = (uint32_t)Data1High_uchar;

//	_printParam("Data0Low_uint",  Data0Low_uint);
//	_printParam("Data0High_uint", Data0High_uint);
//	_printParam("Data1Low_uint",  Data1Low_uint);
//	_printParam("Data1High_uint", Data1High_uint);

	Channel0_uint = ((uint32_t)256 * Data0High_uint) + Data0Low_uint; //Shift Data0High to upper byte
	Channel1_uint = ((uint32_t)256 * Data1High_uint) + Data1Low_uint; //Shift Data1High to upper byte

//	_printParam("Channel0_uint", Channel0_uint);
//	_printParam("Channel1_uint", Channel1_uint);

	Lux = CalculateLux(GainX1, IntegrationTime402, Channel0_uint, Channel1_uint, T_FN_CL_Package);

//	_printParam("Lux", Lux);
//
//	if (Lux == 2)
//	{
//		_printString("ERROR LUX!\r\n");
//	}

	*xLux = Lux;

	return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t CalculateLux(uint32_t iGain, uint32_t tInt, uint32_t ch0, uint32_t ch1, int32_t iType)
{
	// first, scale the channel values depending on the gain and integration time
	// 16X, 402mS is nominal.
	// scale if integration time is NOT 402 msec
	uint32_t chScale;
	uint32_t channel1;
	uint32_t channel0;
	switch (tInt)
	{
		case 0: // 13.7 msec
		chScale = CHSCALE_TINT0;
		break;
		case 1: // 101 msec
		chScale = CHSCALE_TINT1;
		break;
		default: // assume no scaling
		chScale = (1 << CH_SCALE);
		break;
	}
	// scale if gain is NOT 16X
	if (!iGain)
	{
		chScale = chScale << 4; // scale 1X to 16X
	}
	// scale the channel values
	channel0 = (ch0 * chScale) >> CH_SCALE;
	channel1 = (ch1 * chScale) >> CH_SCALE;
	// find the ratio of the channel values (Channel1/Channel0)
	// protect against divide by zero
	uint32_t ratio1 = 0;
	if (channel0 != 0)
	{
		ratio1 = (channel1 << (RATIO_SCALE+1)) / channel0;
	}
	// round the ratio value
	uint32_t ratio = (ratio1 + 1) >> 1;
	// is ratio <= eachBreak ?
	uint32_t b = 0;
	uint32_t m = 0;

	switch (iType)
	{
		case 0: // T, FN and CL package
			if ((ratio >= 0) && (ratio <= K1T))
			{
				b = B1T;
				m = M1T;
			}
			else if (ratio <= K2T)
			{
				b = B2T;
				m = M2T;
			}
			else if (ratio <= K3T)
			{
				b = B3T;
				m = M3T;
			}
			else if (ratio <= K4T)
			{
				b = B4T;
				m = M4T;
			}
			else if (ratio <= K5T)
			{
				b = B5T;
				m = M5T;
			}
			else if (ratio <= K6T)
			{
				b = B6T;
				m = M6T;
			}
			else if (ratio <= K7T)
			{
				b = B7T;
				m = M7T;
			}
			else if (ratio > K8T)
			{
				b = B8T;
				m = M8T;
			}
			break;
		case 1:// CS package
		if ((ratio >= 0) && (ratio <= K1C))
		{
			b = B1C;
			m = M1C;
		}
		else if (ratio <= K2C)
		{
			b = B2C;
			m = M2C;
		}
		else if (ratio <= K3C)
		{
			b = B3C;
			m = M3C;
		}
		else if (ratio <= K4C)
		{
			b = B4C;
			m = M4C;
		}
		else if (ratio <= K5C)
		{
			b = B5C;
			m = M5C;
		}
		else if (ratio <= K6C)
		{
			b = B6C;
			m = M6C;
		}
		else if (ratio <= K7C)
		{
			b = B7C;
			m = M7C;
		}
		else if (ratio > K8C)
		{
			b = B8C;
			m = M8C;
		}
		break;
	}
	uint32_t temp;
	temp = ((channel0 * b) - (channel1 * m));
	// do not allow negative lux value
	if (temp < 0)
	{
		temp = 0;
	}
	// round lsb (2^(LUX_SCALE?1))
	temp += (1 << (LUX_SCALE-1));
	// strip off fractional portion
	uint32_t lux = temp >> LUX_SCALE;
	return lux;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
