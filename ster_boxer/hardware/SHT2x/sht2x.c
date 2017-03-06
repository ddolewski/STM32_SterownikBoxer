#include "sht2x.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t SHT21_MeasureTempCommand(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, ErrorStatus * Error)
{
	uint32_t TimeOut = 100000;

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
	I2C_SendData(I2Cx, TRIG_T_MEASUREMENT_HM);

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

	I2C_NumberOfBytesConfig(I2Cx, 2);
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
	SHT_TempData.msb_lsb[0] = I2C_ReceiveData(I2Cx);

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
	SHT_TempData.msb_lsb[1] = I2C_ReceiveData(I2Cx);

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
	return SHT_TempData.data;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t SHT21_MeasureHumCommand(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, ErrorStatus * Error)
{
	uint32_t TimeOut = 100000;

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
	I2C_SendData(I2Cx, TRIG_RH_MEASUREMENT_HM);

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

	I2C_NumberOfBytesConfig(I2Cx, 2);
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
	SHT_HumData.msb_lsb[0] = I2C_ReceiveData(I2Cx);

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
	SHT_HumData.msb_lsb[1] = I2C_ReceiveData(I2Cx);

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
	return SHT_HumData.data;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float SHT21_CalcTemp(uint16_t dataTemp)
{
	float temperatureC = 0; 	// variable for result
	dataTemp &= ~0x0003; 		// clear bits [1..0] (status bits)
	//-- calculate temperature [°C] --
	temperatureC = -46.85 + 175.72/65536 *(float)dataTemp; //T= -46.85 + 175.72 * ST/2^16
	return temperatureC;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float SHT21_CalcRH(uint16_t humidity)
{
	float realHumidity = 0; 	// variable for result
	humidity &= ~0x0003; 		// clear bits [1..0] (status bits)
	//-- calculate relative humidity [%RH] --
	realHumidity = -6.0 + 125.0/65536 * (float)humidity; // RH= -6 + 125 * SRH/2^16
	return realHumidity;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t SHT21_ReadUserReg(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, ErrorStatus * Error)
{
	uint32_t TimeOut = 100000;
	uint8_t readValue = 0;

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
	I2C_SendData(I2Cx, USER_REG_R);

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
	readValue = I2C_ReceiveData(I2Cx);

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
	return readValue;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorStatus SHT21_SoftReset(I2C_TypeDef* I2Cx, uint16_t SlaveAddr)
{
	uint32_t TimeOut = 100000;

	I2C_SoftwareResetCmd(I2Cx, DISABLE);
	I2C_SoftwareResetCmd(I2Cx, ENABLE);
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
			return ERROR;
		}
	}

	TimeOut = 100000;
	I2C_SendData(I2Cx, SOFT_RESET);
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
