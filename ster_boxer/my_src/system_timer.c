/*******************************************************************************
Biblioteka obslugi i konfiguracji timer�w
mikrokontroler: STM32F051R8T6, 48MHz
data utworzenia: 2015-04-23
data modyfikacji: 2014-04-23
autor: Damian Dolewski

opis:
Biblioteka zawierajace funkcje obslugi

********************************************************************************/

#include "system_timer.h"
#include "system_gpio.h"
#include "stm32f0xx_rcc.h"
//-----------------------------------------------------------------------------
// Funkcja wlaczajaca zegar RCC dla wybranego timera
// RCC_APBPeriph/in: maska wybranego timera
// NewPeriphState/in: ENABLE - wlacz zegar, DISABLE - wylacz zegar
//-----------------------------------------------------------------------------
void TIMx_ClockRCC(uint32_t RCC_APBPeriph, FunctionalState NewPeriphState)
{
	if (NewPeriphState == ENABLE)
	{
		if (RCC_APBPeriph == RCC_APB2ENR_TIM1EN)
		{
			RCC_APB2PeriphClockCmd(RCC_APBPeriph, ENABLE);
		}
		else
		{
			RCC_APB1PeriphClockCmd(RCC_APBPeriph, ENABLE);
		}
	}
	else
	{
		if (RCC_APBPeriph == RCC_APB2ENR_TIM1EN)
		{
			RCC_APB2PeriphClockCmd(RCC_APBPeriph, DISABLE);
		}
		else
		{
			RCC_APB1PeriphClockCmd(RCC_APBPeriph, DISABLE);
		}
	}
}
//-----------------------------------------------------------------------------
// Funkcja wlaczajaca lub wylaczajaca wybrany timer
// TIMx/in: wskaznik na strukture timera, TIM1..TIM17
// NewPeriphState/in: ENABLE - wlacz timer, DISABLE - wylacz timer
//-----------------------------------------------------------------------------
void TIMx_TimerState(TIM_TypeDef *TIMx, FunctionalState NewPeriphState)
{
	if (NewPeriphState == ENABLE)
	{
		TIMx->CR1 |= TIM_CR1_CEN;
	}
	else
	{
		TIMx->CR1 &= ~ TIM_CR1_CEN;
	}
}
//-----------------------------------------------------------------------------
// Funkcja czyszczaga flage przerwania wybranego timera
// TIMx/in: wskaznik na strukture timera, TIM1..TIM17
// TIM_SR_x/in: maska wybranej flagi przerwania w rejestrze statusu SR
//-----------------------------------------------------------------------------
void TIMx_ClearITFlag(TIM_TypeDef *TIMx, uint16_t TIM_SR_x)
{
	TIMx->SR &= ~TIM_SR_x;
}
//-----------------------------------------------------------------------------
// Funkcja odczytujaca flage przerwania wybranego timera
// TIMx/in: wskaznik na strukture timera, TIM1..TIM17
// TIM_SR_x/in: maska wybranej flagi przerwania w rejestrze statusu SR
//-----------------------------------------------------------------------------
uint8_t TIMx_GetITFlag(TIM_TypeDef *TIMx, uint16_t TIM_SR_x)
{
	if ((TIMx->SR & TIM_SR_x) == TIM_SR_x)
	{
		return SET;
	}
	else
	{
		return RESET;
	}
}
//-----------------------------------------------------------------------------
// Funkcja konfigurujaca wybrany timer w trybie PWM
// TIMx/in: wskaznik na strukture timera, TIM1..TIM17
//-----------------------------------------------------------------------------
void TIMx_ConfigPWMMode(TIM_TypeDef *TIMx)
{
	TIMx->PSC = 0;
	TIMx->ARR = 0;
	TIMx->CCR4 = 0;
	TIMx->CCER = 0;
	TIMx->CR1 = 0;
	GPIOx_ClockConfig(RCC_AHBPeriph_GPIOA, ENABLE);
	GPIOx_PinAFConfig(GPIOA, GPIOx_PinSource3, GPIOx_AF_2);
	GPIOx_PinConfig(GPIOA, Mode_AF, OSpeed_50MHz, OType_PP, OState_PU, GPIOx_Pin_3);

	TIMx->PSC = (uint16_t)((APB1_CLK / TIM_CLK) - 1); 	// preskaler
	TIMx->ARR = (uint16_t)(TIM_CLK / PWM_CLK); 			// okres
	TIMx->CCR4 = 25; // wype�nienie
	TIMx->CCMR2 |= TIM_CCMR2_OC4M; // PWM Mode 2 "111"
	TIMx->CCER |= TIM_CCER_CC4NP | TIM_CCER_CC4E; // ch4 out polarity low(active) | ch4 out enable
	TIMx->CR1 |= TIM_CR1_ARPE;
	TIMx->EGR |= TIM_EGR_UG;
}
//-----------------------------------------------------------------------------
// Funkcja konfigurujaca wybrany timer w trybie output compare
// TIMx/in: wskaznik na strukture timera, TIM1..TIM17
//-----------------------------------------------------------------------------
void TIMx_ConfigOutputCompareMode(TIM_TypeDef *TIMx)
{
	TIMx->PSC = 0;
	TIMx->ARR = 0;
	TIMx->CCMR2 = 0;
	TIMx->CCER = 0;
	TIMx->CR1 = 0;
	TIMx->DIER = 0;
	/*
	 * 	OBLICZANIE WARTOSCI ARR I PSC DLA DANEJ CZESTOTLIWOSCI:
	 ********************************************************************************************************
	 * 	REJESTR PSC (PRESCALER TIMERA):
	 * 	PSC = (CZESTOTLIWOSC MAGISTRALI F_APB / CZESTOTLIWOSC TAKTOWANIA TIMERA (USTALONA NP. 12 MHZ) - 1
	 ********************************************************************************************************
	 * 	REJESTR AUTO-RELOAD REGISTER (OKRES):
	 * 	ARR = CZESTOTLIWOSC TAKTOWANIA TIMERA (PATRZ WYZEJ) / OCZEKIWANA CZESTOTLIWOSC (NP. WYJSCIA OCx)
	 */

	// timer ustawiony na 1MHz (zmierzone: ? MHz)
#ifdef SEG_SLOW
	TIMx->PSC = 60000; 	// testowe oko�o 1Hz
	TIMx->ARR = 1000; 	// testowe oko�o 1Hz
//	TIMx->PSC = 1000; 	// testowe
//	TIMx->ARR = 120; 	// testowe
#else
	TIMx->PSC = (uint16_t)((24000000 / 12000000) - 1);
	TIMx->ARR = (uint16_t)(12000000 / 1000000);
#endif
	//TIMx->CR1 |= TIM_CR1_ARPE;
	TIMx->CR1 |= TIM_CR1_URS;
	TIMx->DIER |= TIM_DIER_UIE;
	//TIMx->CR1 |= TIM_CR1_CEN;
	//TIMx->CCMR2 |= TIM_CCMR2_OC4M_0 | TIM_CCMR2_OC4M_1; // bity: 011 - toggle OC4
	//TIMx->CCER |= TIM_CCER_CC4P | TIM_CCER_CC4E; 		// ch4 out polarity high(active) | ch4 out enable
	//TIMx->DIER |= TIM_DIER_CC4IE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
