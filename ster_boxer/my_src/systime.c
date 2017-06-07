#include "global.h"
#include <stdint.h>
#include "stm32f0xx.h"
#include "systime.h"
#include "debug.h"
#include "boxer_climate.h"
#include "boxer_timers.h"
volatile uint8_t toggle = 0;
static volatile systime_t msTimer = 0;	//timer systemowy odmierzajacy czas w ms
//-------------------------------------------------------------------------------------------------
// Funkcja inicjalizujaca timer systemeowy
//void
//return:		void
//-------------------------------------------------------------------------------------------------
void systimeInit(void)
{
	SysTick_Config(SystemCoreClock / 1000); //1ms
}
//-------------------------------------------------------------------------------------------------
// Funkcja obslugi przerwania SysTick
//void
//return:		void
//-------------------------------------------------------------------------------------------------
void SysTick_Handler (void)
{
	msTimer++;
	systickIrq = 1;
#ifdef I2C_OFF_MODE
	toggle ^= 1;
	switch (toggle)
	{
	case 0:
		GPIOB->BRR = (uint32_t)GPIOx_Pin_10;
		break;

	case 1:
		GPIOB->BSRR = (uint32_t)GPIOx_Pin_10;
		break;

	default:
		break;
	}
#endif
}
//-------------------------------------------------------------------------------------------------
// Funkcja zwracajaca aktualny czas systemowy
//void
//return:	sysTimer	czas systemowy
//-------------------------------------------------------------------------------------------------
systime_t systimeGet (void)
{
	return msTimer;
}
//-------------------------------------------------------------------------------------------------
// Funkcja zwracajaca aktualny czas systemowy
//xLastTime		/inOut: wskaznik na zmienna reprezentujaca czas ostatniego wywolania danego timeouta
//xCheckTime	/in:	interwal czasu sprawdzenia
//return:	timeElapsed	TRUE  -  czas minal, FALSE - czas nie minal
//-------------------------------------------------------------------------------------------------
uint8_t systimeTimeoutControl (systime_t * xLastTime, systime_t xCheckTime)
{
	uint8_t  timeElapsed 	= FALSE;
	systime_t currentTime	= systimeGet();		//pobranie aktualnego czasu

	if (currentTime - (*xLastTime) >= xCheckTime)
	{
		*xLastTime = currentTime;
		timeElapsed = TRUE;
	}
	return timeElapsed;
}
//-----------------------------------------------------------------------------
// Funkcja ztrzymujaca wykonywanie programu na podany czas
// xTimeMs/in czas zatrzymania
// return: void
//-----------------------------------------------------------------------------
void systimeDelayMs(uint32_t xTimeMs)
{
	systime_t time = systimeGet();
	while (!systimeTimeoutControl(&time, xTimeMs))
	{
		__NOP();
	}
}
