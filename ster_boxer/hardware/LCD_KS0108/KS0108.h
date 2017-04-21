//-------------------------------------------------------------------------------------------------
// Graphic LCD C library
// http://glcd-libc.dxp.pl
// (c) Rados�aw Kwiecie�, radek@dxp.pl
//-------------------------------------------------------------------------------------------------
#ifndef __KS0108
#define __KS0108

#include <stdint.h>
#include "stm32f0xx_gpio.h"

#define KS0108_PORT_RS  			GPIOA
#define KS0108_PORT_RW  			GPIOA
#define KS0108_PORT_EN  			GPIOA

#define KS0108_PORT_CS1  			GPIOA
#define KS0108_PORT_CS2 			GPIOA

#define KS0108_PORT_D0  			GPIOB
#define KS0108_PORT_D1  			GPIOB
#define KS0108_PORT_D2  			GPIOB
#define KS0108_PORT_D3  			GPIOB
#define KS0108_PORT_D4  			GPIOC
#define KS0108_PORT_D5  			GPIOC
#define KS0108_PORT_D6  			GPIOC
#define KS0108_PORT_D7  			GPIOC

#define KS0108_PIN_RS				GPIO_Pin_10
#define KS0108_PIN_RW    			GPIO_Pin_8
#define KS0108_PIN_EN    			GPIO_Pin_9

#define KS0108_PIN_CS1   			GPIO_Pin_11
#define KS0108_PIN_CS2   			GPIO_Pin_12

#define KS0108_PIN_D0    			GPIO_Pin_12
#define KS0108_PIN_D1    			GPIO_Pin_13
#define KS0108_PIN_D2    			GPIO_Pin_14
#define KS0108_PIN_D3    			GPIO_Pin_15
#define KS0108_PIN_D4    			GPIO_Pin_6
#define KS0108_PIN_D5    			GPIO_Pin_7
#define KS0108_PIN_D6    			GPIO_Pin_8
#define KS0108_PIN_D7    			GPIO_Pin_9

#define KS0108_SCREEN_WIDTH		128
#define KS0108_SCREEN_HEIGHT	64

#define DISPLAY_SET_Y       	0x40
#define DISPLAY_SET_X       	0xB8
#define DISPLAY_START_LINE  	0xC0
#define DISPLAY_ON_CMD			0x3E
#define ON						0x01
#define OFF						0x00
#define DISPLAY_STATUS_BUSY		0x80
//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void GLCD_Init(void);
void GLCD_Initialize(void);
void GLCD_WriteData(uint8_t data);
void GLCD_ClearScreen(void);
void GLCD_TextGoTo(uint8_t x, uint8_t y);
void GLCD_GoTo(uint8_t x, uint8_t y);
void GLCD_WriteString(char * stringToWrite);
uint8_t GLCD_ReadByteFromROMMemory(uint8_t * RomByte);
void GLCD_InitializeDataPin(uint8_t mode);
void GLCD_InitializeCtrlPin(void);

#endif
