#include "system_rcc.h"

void RCC_SetHSE(void)
{
	if((RCC->CR & RCC_CR_HSERDY) == RESET)
	{
		RCC->CFGR |= RCC_CFGR_PLLSRC_PREDIV1;//(1 << 16); // hse/prediv selected as pll input clock
		RCC->CFGR |= RCC_CFGR_SW_1; //(1 << 1);	// pll selected as system clock
		RCC->CFGR &= ~RCC_CFGR_PLLMULL; // pll x2
		RCC->CFGR |= RCC_CFGR_PLLSRC;
		RCC->CR &= ~ RCC_CR_HSION; //turn off hsi oscilator
		RCC->CR |= RCC_CR_HSEBYP; // hse bypass
//		Delay_ms(1); //wait for hsi turn off --> after 6 cycles HSI
		RCC->CR |= RCC_CR_HSEON; // turn on hse oscilator
//		Delay_ms(10); // wait for hse turn on --> after 512 cycles HSE
		//RCC->CR |= RCC_CR_HSEBYP; // hse bypass
	}
}

void RCC_SetPLL(void)
{
	RCC->CR &= ~ RCC_CR_PLLON; //turn off pll clock

	if((RCC->CR & RCC_CR_PLLON) == RESET)
	{

	}

	if((RCC->CR & RCC_CR_PLLRDY) == RESET)
	{

	}

	RCC->CFGR |= RCC_CFGR_PLLSRC_PREDIV1;//(1 << 16); // hse/prediv selected as pll input clock
	RCC->CFGR |= RCC_CFGR_SW_1; //(1 << 1);	// pll selected as system clock

	if((RCC->CFGR & RCC_CFGR_SWS_1) == SET)
	{
		// to jest git
	}

	RCC->CR |= RCC_CR_PLLON; //turn on pll clock

	if((RCC->CR & RCC_CR_PLLRDY) == SET)
	{

	}


}


/**
* Description: This function enables the interrupt on HSE ready,
* and start the HSE as external clock.
*/
void StartHSE(void)
{
	/* Configure NVIC for RCC */
	/* (1) Enable Interrupt on RCC */
	/* (2) Set priority for RCC */
	NVIC_EnableIRQ(RCC_IRQn); /* (1)*/
	NVIC_SetPriority(RCC_IRQn,0); /* (2) */
	/* (1) Enable interrupt on HSE ready */
	/* (2) Enable the CSS
	Enable the HSE and set HSEBYP to use the external clock
	instead of an oscillator
	Enable HSE */
	/* Note : the clock is switched to HSE in the RCC_IRQHandler ISR */
	RCC->CIR |= RCC_CIR_HSERDYIE; /* (1) */
	RCC->CR |= RCC_CR_CSSON | RCC_CR_HSEBYP | RCC_CR_HSEON; /* (2) */
}

/**
* Description: This function handles RCC interrupt request
* and switch the system clock to HSE.
*/
void RCC_CRS_IRQHandler( void)
{
	/* (1) Check the flag HSE ready */
	/* (2) Clear the flag HSE ready */
	/* (3) Switch the system clock to HSE */
	if ((RCC->CIR & RCC_CIR_HSERDYF) != 0) /* (1) */
	{
		RCC->CIR |= RCC_CIR_HSERDYC; /* (2) */
		RCC->CFGR = ((RCC->CFGR & (~RCC_CFGR_SW)) | RCC_CFGR_SW_0); /* (3) */
	}
	else
	{
		/* Report an error */
	}
}
