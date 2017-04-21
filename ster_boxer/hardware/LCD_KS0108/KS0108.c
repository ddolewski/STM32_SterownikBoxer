//-------------------------------------------------------------------------------------------------
// Graphic LCD C library
// (c) Rados�aw Kwiecie�, radek@dxp.pl
//-------------------------------------------------------------------------------------------------
#include "KS0108.h"
#include "glcd_font5x8.h"
#include "system_gpio.h"
//-------------------------------------------------------------------------------------------------
// Reference to MCU-specific external functions
//-------------------------------------------------------------------------------------------------
extern void GLCD_WriteData(uint8_t);
extern void GLCD_WriteCommand(uint8_t, uint8_t);
extern uint8_t GLCD_ReadData(void);
//-------------------------------------------------------------------------------------------------
// screen coordinates
uint8_t screen_x = 0;
uint8_t screen_y = 0;

void GLCD_Init(void)
{
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_GoTo(0,0);
	GLCD_WriteString("Inicjalizacja...");
}
//-------------------------------------------------------------------------------------------------
// Initialization interface and LCD controller
//-------------------------------------------------------------------------------------------------
void GLCD_Initialize(void)
{
	uint8_t i;
	GLCD_InitializeCtrlPin();
	GLCD_InitializeDataPin(Mode_Out);
	GPIOx_SetPin(KS0108_PORT_D0, KS0108_PIN_D0);
	GPIOx_SetPin(KS0108_PORT_D1, KS0108_PIN_D1);
	GPIOx_SetPin(KS0108_PORT_D2, KS0108_PIN_D2);
	GPIOx_SetPin(KS0108_PORT_D3, KS0108_PIN_D3);
	GPIOx_SetPin(KS0108_PORT_D4, KS0108_PIN_D4);
	GPIOx_SetPin(KS0108_PORT_D5, KS0108_PIN_D5);
	GPIOx_SetPin(KS0108_PORT_D6, KS0108_PIN_D6);
	GPIOx_SetPin(KS0108_PORT_D7, KS0108_PIN_D7);

	for(i = 0; i < 3; i++)
	{
		GLCD_WriteCommand((DISPLAY_ON_CMD | ON), i);
	}
}
//-------------------------------------------------------------------------------------------------
// Sets screen coordinates and address 
//-------------------------------------------------------------------------------------------------
void GLCD_GoTo(uint8_t x, uint8_t y)
{
	uint8_t i;
	screen_x = x;
	screen_y = y;

	for(i = 0; i < KS0108_SCREEN_WIDTH/64; i++)
	{
		GLCD_WriteCommand(DISPLAY_SET_Y | 0, i);
		GLCD_WriteCommand(DISPLAY_SET_X | y, i);
		GLCD_WriteCommand(DISPLAY_START_LINE | 0,i);
	}
	GLCD_WriteCommand((DISPLAY_SET_Y | (x % 64)), (x / 64));
	GLCD_WriteCommand((DISPLAY_SET_X | y), (x / 64));
}
//-------------------------------------------------------------------------------------------------
// Sets text coordinates and address 
//-------------------------------------------------------------------------------------------------
void GLCD_TextGoTo(uint8_t x, uint8_t y)
{
	GLCD_GoTo(x*6, y);
}
//-------------------------------------------------------------------------------------------------
// Clears screen
//-------------------------------------------------------------------------------------------------
void GLCD_ClearScreen(void)
{
	uint8_t i, j;
	for(j = 0; j < KS0108_SCREEN_HEIGHT/8; j++)
	{
		GLCD_GoTo(0,j);
		for(i = 0; i < KS0108_SCREEN_WIDTH; i++)
		{
			GLCD_WriteData(0x00);
		}
	}
}
//-------------------------------------------------------------------------------------------------
// Writes char to screen memory
//-------------------------------------------------------------------------------------------------
void GLCD_WriteChar(uint8_t charToWrite)
{
	uint8_t i;
	charToWrite -= 32;
	for(i = 0; i < 5; i++)
	{
		GLCD_WriteData(GLCD_ReadByteFromROMMemory((uint8_t *)(font5x8 + (5 * charToWrite) + i)));
	}
	GLCD_WriteData(0x00);
}
//-------------------------------------------------------------------------------------------------
// Write null-terminated string to screen memory
//-------------------------------------------------------------------------------------------------
void GLCD_WriteString(char * stringToWrite)
{
	while(*stringToWrite)
	{
		GLCD_WriteChar(*stringToWrite++);
	}
}
//-------------------------------------------------------------------------------------------------
// Sets or clears pixel at (x,y)
//-------------------------------------------------------------------------------------------------
void GLCD_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
	uint8_t tmp;
	GLCD_GoTo(x, y/8);
	tmp = GLCD_ReadData();
	GLCD_GoTo(x, y/8);
	tmp = GLCD_ReadData();
	GLCD_GoTo(x, y/8);
	if(color)
		tmp |= (1 << (y%8));
	else
		tmp &= ~(1 << (y%8));
	GLCD_WriteData(tmp);
}
//-------------------------------------------------------------------------------------------------
// Displays bitmap at (x,y) and (dx,dy) size
//-------------------------------------------------------------------------------------------------
void GLCD_Bitmap(uint8_t * bmp, uint8_t x, uint8_t y, uint8_t dx, uint8_t dy)
{
	uint8_t i, j;
	for(j = 0; j < dy / 8; j++)
	{
		GLCD_GoTo(x,y + j);
		for(i = 0; i < dx; i++)
		{
			GLCD_WriteData(GLCD_ReadByteFromROMMemory(bmp++));
		}
	}
}
