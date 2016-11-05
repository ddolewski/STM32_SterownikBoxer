/*******************************************************************************
Biblioteka konfiguracji przerwania zewnetrznego EXTI
mikrokontroler: STM32F051R8T6, 48MHz
data utworzenia: 2015-04-23
data modyfikacji: 2014-04-23
autor: Damian Dolewski

opis:
Biblioteka zawierajace funkcje obslugi

********************************************************************************/

#include "system_exti.h"
//-----------------------------------------------------------------------------
// Funkcja konfigurujaca wybrana linie (pin) jako przerwanie zewnetrzne
// EXTIx/in: linia EXTI0..19
// AFIO_EXTICRx_EXTIy_Pz/in: maska dla rejestru przemapowujacego zwykle linie 0..15
// Edge/in: sposb wyzwalania - jakie zbocze
// Mode/in: typ enum - przerwanie sprzetowe, zdarzenie sprzetowe lub przerwanie/zdarzenie programowe
//-----------------------------------------------------------------------------
ErrorStatus EXTI_ConfigLine(uint32_t EXTIx, uint16_t AFIO_EXTICRx_EXTIy_Pz, uint8_t Edge, uint8_t Mode)
{
	if(EXTIx != EXTI20 && EXTIx != EXTI24 && EXTIx != EXTI26 && EXTIx != EXTI28 && EXTIx != EXTI29 && EXTIx != EXTI30)
	{
		// przemapowanie pinu - funkcja alternatywna jako EXTI
//		if(AFIO_EXTICRx_EXTIy_Pz != AFIO_EXTI_SPECIAL_LINE)
//		{
//			if (EXTIx <= EXTI3)
//			{
//				AFIO->EXTICR[0] |= AFIO_EXTICRx_EXTIy_Pz;
//			}
//			else if (EXTIx <= EXTI7 && EXTIx >= EXTI4)
//			{
//				AFIO->EXTICR[1] |= AFIO_EXTICRx_EXTIy_Pz;
//			}
//			else if (EXTIx <= EXTI11 && EXTIx >= EXTI8)
//			{
//				AFIO->EXTICR[2] |= AFIO_EXTICRx_EXTIy_Pz;
//			}
//			else if (EXTIx <= EXTI15 && EXTIx >= EXTI12)
//			{
//				AFIO->EXTICR[3] |= AFIO_EXTICRx_EXTIy_Pz;
//			}
//		}
		// KONFIGURACJA WYZWALANIA (wybór zbocza)
		if (Edge == Edge_Falling)
		{
			EXTI->FTSR &= ~EXTIx;
			EXTI->FTSR |= EXTIx;
		}
		else if (Edge == Edge_Rising)
		{
			EXTI->RTSR &= ~EXTIx;
			EXTI->RTSR |= EXTIx;
		}
		else if (Edge == Edge_Both)
		{
			EXTI->FTSR &= ~EXTIx;
			EXTI->FTSR |= EXTIx;
			EXTI->RTSR &= ~EXTIx;
			EXTI->RTSR |= EXTIx;
		}

		// KONFIGURACJA PROCEDURY OBS£UGI: PRZERWANIE / ZDARZENIE SPRZÊTOWE LUB PRZERWANIE / ZDARZENIE PROGRAMOWE
		if (Mode == Mode_SoftwareInterruptEvent)
		{
			EXTI->SWIER &= ~EXTIx;
			EXTI->SWIER |= EXTIx;
		}
		else if (Mode == Mode_HardwareEvent)
		{
			EXTI->EMR &= ~EXTIx;
			EXTI->EMR |= EXTIx;
		}
		else if (Mode == Mode_HardwareInterrupt)
		{
			EXTI->IMR &= ~EXTIx;
			EXTI->IMR |= EXTIx;
		}
		
		return SUCCESS;
	}
	else
	{
		return ERROR;
	}
}
//-----------------------------------------------------------------------------
// Funkcja czyszczaca flage przerwania
// EXTIx/in: linia EXTI0..19
//-----------------------------------------------------------------------------
void EXTI_ClearITFlag(uint32_t EXTIx)
{
	EXTI->PR |= EXTIx;
}
//-----------------------------------------------------------------------------
// Funkcja odczytujaca flage przerwania
// EXTIx/in: linia EXTI0..19
// return: SET - jesli flaga ustawiona, RESET - jesli skasowana
//-----------------------------------------------------------------------------
uint8_t EXTI_GetITFlag(uint32_t EXTIx)
{
	if ((EXTI->PR & EXTIx) == EXTIx)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
