/*
 * boxer_timers.h
 *
 *  Created on: 3 sie 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_TIMERS_H_
#define MY_INC_BOXER_TIMERS_H_

#include "stm32f0xx_tim.h"
#include "stm32f0xx_syscfg.h"

#define PWM_FAN_FREQ 30

#define REG_PWM_PUSH_AIR_FAN 				TIM3->CCR3 // wciagajacy powietrze 	TIM3_CH3 -> PB0
#define REG_PWM_PULL_AIR_FAN 				TIM3->CCR4 // wyciagajacy powietrze TIM3_CH4 -> PB1
#define REG_PWM_PUMP 						TIM2->CCR2 // PWM pompy 			TIM2_CH2 -> PA1

typedef enum
{
	PWM_PUMP,
	PWM_FAN_PULL_AIR,
	PWM_FAN_PUSH_AIR
}pwm_dev_type_t;

typedef enum
{
	PWM_CHANGE_FAST,
	PWM_CHANGE_SLOW
}pwm_change_speed_t;

uint8_t lastPullPWM;
uint8_t lastPushPWM;
light_state_t lastLightState;
extern bool_t initFanPwm;

void PWM_FansInit(void);
void PWM_PumpInit(void);

void SoftStart_Handler(void);
void MainTimer_Handler(void);
void PWM_SetPercent(uint8_t xPwmDev, uint32_t xPercent);
uint8_t PWM_IncPercentTo(pwm_dev_type_t xPwmDev, uint8_t xPercent, pwm_change_speed_t xSpeed);
uint8_t PWM_DecPercentTo(pwm_dev_type_t xPwmDev, uint8_t xPercent, pwm_change_speed_t xSpeed);

#endif /* MY_INC_BOXER_TIMERS_H_ */
