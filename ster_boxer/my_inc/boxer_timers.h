/*
 * boxer_timers.h
 *
 *  Created on: 3 sie 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_TIMERS_H_
#define MY_INC_BOXER_TIMERS_H_

#include "stm32f0xx_tim.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_syscfg.h"
// okolo 12kHz
// wartosc taka ze wzgledu na to aby po przeliczeniu przez rowniania:
// TimerPeriod = (SystemCoreClock / 11994) - 1 oraz
// Channel1Pulse = 25 * (TimerPeriod - 1) / 100 miec pelne wartosci do wpisania w rejestr
// CCRx dzieki temu mozna inkrementowac PWM o rowna wartosc rejestru
// po przeliczeniu np. dla wypelnienia 100% bedzie to wartosc 4000 z czego wynika, ze
// 1% to wartosc 40 w rejestrze CCRx
//#define PWM_FAN_CLK  	11994
#define PWM_FAN_CLK 30

#define REG_PWM_PUSH_AIR_FAN 				TIM3->CCR3 // wci¹gaj¹cy powietrze TIM3_CH3 -> PB0
#define REG_PWM_PULL_AIR_FAN 				TIM3->CCR4 // wyci¹gaj¹cy powietrze TIM3_CH4 -> PB1
#define REG_PWM_PUMP 						TIM2->CCR2 // PWM pompy TIM2_CH2 -> PA1

typedef enum
{
	PWM_PUMP,
	PWM_FAN_PULL_AIR,
	PWM_FAN_PUSH_AIR
}pwm_dev_type_t;

typedef enum
{
	SOFT_START_NONE,
	SOFT_START_PUMP,
	SOFT_START_FANS
}softstart_t;

uint8_t lastPullPWM;
uint8_t lastPushPWM;
light_state_t lastLightState;
extern softstart_t softStartPWM;

void PWM_FansInit(void);
void PWM_PumpInit(void);

void SoftStart_Handler(void);
void MainTimer_Handler(void);
void PWM_SetPercent(uint8_t xPwmDev, uint32_t xPercent);
uint8_t PWM_IncPercentTo(pwm_dev_type_t xPwmDev, uint8_t xPercent);
uint8_t PWM_DecPercentTo(pwm_dev_type_t xPwmDev, uint8_t xPercent);

#endif /* MY_INC_BOXER_TIMERS_H_ */
