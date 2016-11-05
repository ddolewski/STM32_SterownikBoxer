#ifndef SYSTEM_TIMER_H_
#define SYSTEM_TIMER_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "system_rcc.h"
#include "stm32f0xx.h"
#define TIM_CLK  					12000000 	// 12 MHz
#define APB1_CLK  					24000000 	// 24 MHz
#define PWM_CLK 					3000 			// 30 Hz

//#include "global.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TIMx_ClockRCC(uint32_t RCC_APBPeriph, FunctionalState NewPeriphState);
void TIMx_ConfigOutputCompareMode(TIM_TypeDef * TIMx);
void TIMx_ConfigPWMMode(TIM_TypeDef * TIMx);
void TIMx_TimerState(TIM_TypeDef * TIMx, FunctionalState NewPeriphState);
void TIMx_ClearITFlag(TIM_TypeDef * TIMx, uint16_t TIM_SR_x);
uint8_t TIMx_GetITFlag(TIM_TypeDef * TIMx, uint16_t TIM_SR_x);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif /* SYSTEM_TIMER_H_ */
