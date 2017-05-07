#ifndef SYSTEM_GPIO
#define SYSTEM_GPIO
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stm32f0xx.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define GPIOx_Pin_0 	                ((uint16_t)0x0001)  // Pin 0
#define GPIOx_Pin_1 	                ((uint16_t)0x0002)  // Pin 1
#define GPIOx_Pin_2 	                ((uint16_t)0x0004)  // Pin 2
#define GPIOx_Pin_3 	                ((uint16_t)0x0008)  // Pin 3
#define GPIOx_Pin_4 	                ((uint16_t)0x0010)  // Pin 4
#define GPIOx_Pin_5 	                ((uint16_t)0x0020)  // Pin 5
#define GPIOx_Pin_6 	                ((uint16_t)0x0040)  // Pin 6
#define GPIOx_Pin_7 	                ((uint16_t)0x0080)  // Pin 7
#define GPIOx_Pin_8 	                ((uint16_t)0x0100)  // Pin 8
#define GPIOx_Pin_9 	                ((uint16_t)0x0200)  // Pin 9
#define GPIOx_Pin_10 	                ((uint16_t)0x0400)  // Pin 10
#define GPIOx_Pin_11 	                ((uint16_t)0x0800)  // Pin 11
#define GPIOx_Pin_12 	               	((uint16_t)0x1000)  // Pin 12
#define GPIOx_Pin_13 	               	((uint16_t)0x2000)  // Pin 13
#define GPIOx_Pin_14 	               	((uint16_t)0x4000)  // Pin 14
#define GPIOx_Pin_15 	               	((uint16_t)0x8000)  // Pin 15
#define GPIOx_Pin_All               	((uint16_t)0xFFFF)  // All pins

#define GPIOx_AF_0            ((uint8_t)0x00) /* WKUP, EVENTOUT, TIM15, SPI1, TIM17,
                                                MCO, SWDAT, SWCLK, TIM14, USART1,
                                                CEC, IR_OUT, SPI2 */

#define GPIOx_AF_1            ((uint8_t)0x01) /* USART2, CEC, TIM3, USART1, IR_OUT, EVENTOUT, I2C1, I2C2, TIM15 */

#define GPIOx_AF_2            ((uint8_t)0x02) /* TIM2, TIM1, EVENTOUT, TIM16, TIM17 */

#define GPIOx_AF_3            ((uint8_t)0x03) /* TS, I2C1, TIM15, EVENTOUT */

#define GPIOx_AF_4            ((uint8_t)0x04) /* TIM14, I2C1 (only for STM32F0XX_LD and STM32F030X6 devices) */

#define GPIOx_AF_5            ((uint8_t)0x05) /* TIM16, TIM17 */

#define GPIOx_AF_6            ((uint8_t)0x06) /* EVENTOUT */

#define GPIOx_AF_7            ((uint8_t)0x07) /* COMP1 OUT and COMP2 OUT */

#define GPIOx_PinSource0            ((uint8_t)0x00)
#define GPIOx_PinSource1            ((uint8_t)0x01)
#define GPIOx_PinSource2            ((uint8_t)0x02)
#define GPIOx_PinSource3            ((uint8_t)0x03)
#define GPIOx_PinSource4            ((uint8_t)0x04)
#define GPIOx_PinSource5            ((uint8_t)0x05)
#define GPIOx_PinSource6            ((uint8_t)0x06)
#define GPIOx_PinSource7            ((uint8_t)0x07)
#define GPIOx_PinSource8            ((uint8_t)0x08)
#define GPIOx_PinSource9            ((uint8_t)0x09)
#define GPIOx_PinSource10           ((uint8_t)0x0A)
#define GPIOx_PinSource11           ((uint8_t)0x0B)
#define GPIOx_PinSource12           ((uint8_t)0x0C)
#define GPIOx_PinSource13           ((uint8_t)0x0D)
#define GPIOx_PinSource14           ((uint8_t)0x0E)
#define GPIOx_PinSource15           ((uint8_t)0x0F)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	Mode_Out = 0x01,
	Mode_In = 0x00,
	Mode_AF = 0x02,
	Mode_AN = 0x03
}GPIOx_Mode;

typedef enum
{
	OSpeed_2MHz = 0x02,
	OSpeed_10MHz = 0x01,
	OSpeed_50MHz = 0x03,
	OSpeed_None = 0x00
}GPIOx_Speed;

typedef enum
{
	OType_PP = 0x00,
	OType_OD = 0x01
}GPIOx_OutputType;

typedef enum
{
	OState_Float = 0x00,
	OState_PU = 0x01,
	OState_PD = 0x02
}GPIOx_Output_PuPd;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t GPIOx_ReadOutputAll(GPIO_TypeDef* GPIOx);
uint8_t GPIOx_ReadOutputPin(GPIO_TypeDef* GPIOx, uint16_t GPIOx_Pin_y);
uint16_t GPIOx_ReadInputAll(GPIO_TypeDef* GPIOx);
uint8_t GPIOx_ReadInputPin(GPIO_TypeDef* GPIOx, uint16_t GPIOx_Pin_y);
void GPIOx_ResetPin(GPIO_TypeDef* GPIOx, uint16_t GPIOx_Pin_y);
void GPIOx_SetPin(GPIO_TypeDef* GPIOx, uint16_t GPIOx_Pin_y);
void GPIOx_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIOx_Pin_y);
void GPIOx_PinConfig(GPIO_TypeDef* GPIOx, uint8_t GPIOx_Mode, uint8_t GPIOx_Speed, uint8_t GPIOx_OType, uint8_t GPIOx_OState, uint16_t GPIOx_Pin_y);
void GPIOx_PinAFConfig(GPIO_TypeDef* GPIOx, uint16_t GPIOx_PinSource, uint8_t GPIOx_AF);
void GPIOx_ClockConfig(uint32_t RCC_AHBPeriph, FunctionalState NewState);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
