#include "global.h"
#include <stdint.h>
#include "stm32f0xx.h"
#include "systime.h"
#include "debug.h"
#include "boxer_climate.h"
#include "boxer_timers.h"

//Timer systemowy
#define SYSTIMER 				SysTick
#define SYSTIMER_HANDLER		SysTick_Handler

static volatile systime_t sysTimer = 0;	//timer systemowy odmierzajacy czas w ms
//-------------------------------------------------------------------------------------------------
// Funkcja inicjalizujaca timer systemeowy
//void
//return:		void
//-------------------------------------------------------------------------------------------------
void systimeInit(void)
{
	SysTick_Config (SystemCoreClock / 1000);
}
//-------------------------------------------------------------------------------------------------
// Funkcja obslugi przerwania SysTick
//void
//return:		void
//-------------------------------------------------------------------------------------------------
void SYSTIMER_HANDLER (void)
{
	sysTimer++;
	systickIRQ = 1;
//	SoftStart_Handler();
//
//	if (peripheralsInit == TRUE)
//	{
//		Climate_TempCtrl_Handler();
//	}
}
//-------------------------------------------------------------------------------------------------
// Funkcja zwracajaca aktualny czas systemowy
//void
//return:	sysTimer	czas systemowy
//-------------------------------------------------------------------------------------------------
systime_t systimeGet (void)
{
	return sysTimer;
}
//-------------------------------------------------------------------------------------------------
// Funkcja zwracajaca aktualny czas systemowy
//xLastTime		/inOut: wskaznik na zmienna reprezentujac� czas ostatniego wywolania danego timeouta
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
