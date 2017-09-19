/*
 * i2c.c
 *
 *  Created on: 19 wrz 2017
 *      Author: Doles
 */

#include "global.h"
#include "stm32f0xx_i2c.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_syscfg.h"
#include "system_gpio.h"

#ifndef DEBUG_TERMINAL_USART
void I2C1_Init(void)
{
	GPIOx_ClockConfig(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource6, GPIOx_AF_1); //scl
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource7, GPIOx_AF_1); //sda
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C1_SCL);
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C1_SDA);

	RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_SoftwareResetCmd(I2C2, ENABLE);
	I2C_SoftwareResetCmd(I2C2, DISABLE);

	I2C_InitTypeDef  I2C_InitStructure;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Timing = 0x00905E82; //~33KHz
	I2C_Init(I2C1, &I2C_InitStructure);

	SYSCFG_I2CFastModePlusConfig(SYSCFG_I2CFastModePlus_PB6, DISABLE);
	SYSCFG_I2CFastModePlusConfig(SYSCFG_I2CFastModePlus_PB7, DISABLE);

	I2C_Cmd(I2C1, ENABLE);
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
void I2C2_Init(void)
{
	GPIOx_ClockConfig(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource10, GPIOx_AF_1); //scl
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource11, GPIOx_AF_1); //sda
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C2_SCL);
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C2_SDA);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	I2C_InitTypeDef  I2C_InitStructure;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0x00;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0xAB;
	I2C_InitStructure.I2C_Timing = 0x1060DCFD; //I2C clock = 48MHz, I2C SCL = ~50KHz --- from ST I2C timing XLS
	I2C_Init(I2C2, &I2C_InitStructure);

	I2C_Cmd(I2C2, ENABLE);
}
