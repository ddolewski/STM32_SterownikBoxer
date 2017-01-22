//-------------------------------------------------------------------------------------------------
// Universal KS0108 driver library
// STM32 MCU low-level driver
// (c) Rados³aw Kwiecieñ, radek@dxp.pl
//-------------------------------------------------------------------------------------------------
#include "stm32f0xx.h"
#include "stm32f0xx_gpio.h"
#include "system_gpio.h"
#include "KS0108.h"

#define DISPLAY_STATUS_BUSY		0x80

extern uint8_t screen_x;
//extern uint8_t screen_y;
static uint16_t KS0108_PinBuff[] =
{
	KS0108_PIN_D0,
	KS0108_PIN_D1,
	KS0108_PIN_D2,
	KS0108_PIN_D3,
	KS0108_PIN_D4,
	KS0108_PIN_D5,
	KS0108_PIN_D6,
	KS0108_PIN_D7,
};

static GPIO_TypeDef * KS0108_PortBuff[] =
{
	KS0108_PORT_D0,
	KS0108_PORT_D1,
	KS0108_PORT_D2,
	KS0108_PORT_D3,
	KS0108_PORT_D4,
	KS0108_PORT_D5,
	KS0108_PORT_D6,
	KS0108_PORT_D7,
};

//GPIO_InitTypeDef GPIO_InitStructure;

//-------------------------------------------------------------------------------------------------
// Delay function /for 8MHz/
//-------------------------------------------------------------------------------------------------
static void GLCD_Delay(void)
{
	__NOP();__NOP();__NOP();__NOP();
}
//-------------------------------------------------------------------------------------------------
// Enalbe Controller (0-1)
//-------------------------------------------------------------------------------------------------
static void GLCD_EnableController(uint8_t controller)
{
	switch (controller)
	{
		case 0:
			GPIOx_ResetPin(KS0108_PORT_CS2, KS0108_PIN_CS2);
			break;
		case 1:
			GPIOx_ResetPin(KS0108_PORT_CS1, KS0108_PIN_CS1);
			break;
		default:
			break;
	}
}
//-------------------------------------------------------------------------------------------------
// Disable Controller (0-1)
//-------------------------------------------------------------------------------------------------
static void GLCD_DisableController(uint8_t controller)
{
	switch (controller)
	{
		case 0:
			GPIOx_SetPin(KS0108_PORT_CS2, KS0108_PIN_CS2);
			break;
		case 1:
			GPIOx_SetPin(KS0108_PORT_CS1, KS0108_PIN_CS1);
			break;
		default:
			break;
	}
}
//-------------------------------------------------------------------------------------------------
// Read Status byte from specified controller (0-1)
//-------------------------------------------------------------------------------------------------
static uint8_t GLCD_ReadStatus(uint8_t controller)
{
	uint8_t status;

	GLCD_InitializeDataPin(Mode_In);

	GPIOx_SetPin(KS0108_PORT_RW, KS0108_PIN_RW);
	GPIOx_ResetPin(KS0108_PORT_RS, KS0108_PIN_RS);

	GLCD_EnableController(controller);
	GLCD_Delay();
	GPIOx_SetPin(KS0108_PORT_EN, KS0108_PIN_EN);
	GLCD_Delay();

	uint8_t i;
	uint8_t readBit = 0, readByte = 0;

	for (i = 0; i < 8; i++)
	{
		readBit = GPIOx_ReadInputPin(KS0108_PortBuff[i], KS0108_PinBuff[i]);
		if (readBit)
		{
			readByte |= (1 << i);
		}
	}

	status = readByte & 0xFF;
	GPIOx_ResetPin(KS0108_PORT_EN, KS0108_PIN_EN);
	GLCD_DisableController(controller);
	return status;
}
//-------------------------------------------------------------------------------------------------
// Write command to specified controller
//-------------------------------------------------------------------------------------------------
void GLCD_WriteCommand(uint8_t commandToWrite, uint8_t controller)
{
	while(GLCD_ReadStatus(controller)&DISPLAY_STATUS_BUSY);

	GLCD_InitializeDataPin(Mode_Out);

	GPIOx_ResetPin(KS0108_PORT_RS, KS0108_PIN_RS);
	GPIOx_ResetPin(KS0108_PORT_RW, KS0108_PIN_RW);

	GLCD_Delay();
	GLCD_EnableController(controller);
	GLCD_Delay();

	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if (commandToWrite & (1 << i))
		{
			GPIOx_SetPin(KS0108_PortBuff[i], KS0108_PinBuff[i]);
		}
	}

	for (i = 0; i < 8; i++)
	{
		if (!(commandToWrite & (1 << i)))
		{
			GPIOx_ResetPin(KS0108_PortBuff[i], KS0108_PinBuff[i]);
		}
	}

	GLCD_Delay();
	GPIOx_SetPin(KS0108_PORT_EN, KS0108_PIN_EN);
	GLCD_Delay();
	GPIOx_ResetPin(KS0108_PORT_EN, KS0108_PIN_EN);
	GLCD_Delay();
	GLCD_DisableController(controller);
}
//-------------------------------------------------------------------------------------------------
// Read data from current position
//-------------------------------------------------------------------------------------------------
uint8_t GLCD_ReadData(void)
{
	uint8_t tmp;
	while(GLCD_ReadStatus(screen_x / 64)&DISPLAY_STATUS_BUSY);

	GLCD_InitializeDataPin(Mode_In);

	GPIOx_SetPin(KS0108_PORT_RW, KS0108_PIN_RW);
	GPIOx_SetPin(KS0108_PORT_RS, KS0108_PIN_RS);

	GLCD_EnableController(screen_x / 64);
	GLCD_Delay();
	GPIOx_SetPin(KS0108_PORT_EN, KS0108_PIN_EN);
	GLCD_Delay();

	uint8_t i;
	uint8_t readBit = 0, readByte = 0;

	for (i = 0; i < 8; i++)
	{
		readBit = GPIOx_ReadInputPin(KS0108_PortBuff[i], KS0108_PinBuff[i]);
		if (readBit)
		{
			readByte |= (1 << i);
		}
	}

	tmp = readByte & 0xFF;
	GPIOx_ResetPin(KS0108_PORT_EN, KS0108_PIN_EN);
	GLCD_DisableController(screen_x / 64);
	screen_x++;
	return tmp;
}
//-------------------------------------------------------------------------------------------------
// Write data to current position
//-------------------------------------------------------------------------------------------------
void GLCD_WriteData(uint8_t dataToWrite)
{
	while(GLCD_ReadStatus(screen_x / 64)&DISPLAY_STATUS_BUSY);

	GLCD_InitializeDataPin(Mode_Out);

	GPIOx_ResetPin(KS0108_PORT_RW, KS0108_PIN_RW);
	GLCD_Delay();
	GPIOx_SetPin(KS0108_PORT_RS, KS0108_PIN_RS);
	GLCD_Delay();

	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if (dataToWrite & (1 << i))
		{
			GPIOx_SetPin(KS0108_PortBuff[i], KS0108_PinBuff[i]);
		}
	}

	for (i = 0; i < 8; i++)
	{
		if (!(dataToWrite & (1 << i)))
		{
			GPIOx_ResetPin(KS0108_PortBuff[i], KS0108_PinBuff[i]);
		}
	}

	GLCD_Delay();
	GLCD_EnableController(screen_x / 64);
	GLCD_Delay();
	GPIOx_SetPin(KS0108_PORT_EN, KS0108_PIN_EN);
	GLCD_Delay();
	GPIOx_ResetPin(KS0108_PORT_EN, KS0108_PIN_EN);
	GLCD_Delay();
	GLCD_DisableController(screen_x / 64);
	screen_x++;
}
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
uint8_t GLCD_ReadByteFromROMMemory(uint8_t * ptr)
{
	return *(ptr);
}

void GLCD_InitializeDataPin(uint8_t mode)
{
	GPIOx_PinConfig(KS0108_PORT_D0, mode, OSpeed_2MHz, OType_PP, OState_Float, KS0108_PIN_D0);

	GPIOx_PinConfig(KS0108_PORT_D1, mode, OSpeed_2MHz, OType_PP, OState_Float, KS0108_PIN_D1);

	GPIOx_PinConfig(KS0108_PORT_D2, mode, OSpeed_2MHz, OType_PP, OState_Float, KS0108_PIN_D2);

	GPIOx_PinConfig(KS0108_PORT_D3, mode, OSpeed_2MHz, OType_PP, OState_Float, KS0108_PIN_D3);

	GPIOx_PinConfig(KS0108_PORT_D4, mode, OSpeed_2MHz, OType_PP, OState_Float, KS0108_PIN_D4);

	GPIOx_PinConfig(KS0108_PORT_D5, mode, OSpeed_2MHz, OType_PP, OState_Float, KS0108_PIN_D5);

	GPIOx_PinConfig(KS0108_PORT_D6, mode, OSpeed_2MHz, OType_PP, OState_Float, KS0108_PIN_D6);

	GPIOx_PinConfig(KS0108_PORT_D7, mode, OSpeed_2MHz, OType_PP, OState_Float, KS0108_PIN_D7);
}

void GLCD_InitializeCtrlPin(void)
{
	GPIOx_PinConfig(KS0108_PORT_RS, Mode_Out, OSpeed_2MHz, OType_PP, OState_PU, KS0108_PIN_RS);
	GPIOx_SetPin(KS0108_PORT_RS, KS0108_PIN_RS);

	GPIOx_PinConfig(KS0108_PORT_RW, Mode_Out, OSpeed_2MHz, OType_PP, OState_PU, KS0108_PIN_RW);
	GPIOx_SetPin(KS0108_PORT_RW, KS0108_PIN_RW);

	GPIOx_PinConfig(KS0108_PORT_EN, Mode_Out, OSpeed_2MHz, OType_PP, OState_PU, KS0108_PIN_EN);
	GPIOx_SetPin(KS0108_PORT_EN, KS0108_PIN_EN);

	GPIOx_PinConfig(KS0108_PORT_CS1, Mode_Out, OSpeed_2MHz, OType_PP, OState_PU, KS0108_PIN_CS1);
	GPIOx_SetPin(KS0108_PORT_CS1, KS0108_PIN_CS1);

	GPIOx_PinConfig(KS0108_PORT_CS2, Mode_Out, OSpeed_2MHz, OType_PP, OState_PU, KS0108_PIN_CS2);
	GPIOx_SetPin(KS0108_PORT_CS2, KS0108_PIN_CS2);
}
