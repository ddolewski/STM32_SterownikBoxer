/*
 * boxer_timers.c
 *
 *  Created on: 3 sie 2015
 *      Author: Doles
 */

#include "boxer_timers.h"
#include "boxer_communication.h"

static systime_t oneSecTimer = 0;
static systime_t softStartTimer = 0;
static uint8_t gSetPwmFAN1Flag = 0;
static uint8_t gSetPwmFAN2Flag = 0;
static uint8_t gSetPwmPUMPFlag = 0;

static void FanSoftStart_Handler(void);
static void Lightning_Core(void);
static uint32_t PWM_PercentToRegister(uint32_t xPercent);
static void PWM_IncOnePercent(uint8_t xPwmDev);
static void PWM_DecOnePercent(uint8_t xPwmDev);
static uint8_t PWM_FANSoftStart(bool_t xStatus);
static uint16_t TimerPeriod = 0;
uint8_t gFansSoftStartFlag = 0;

static uint8_t atnelWaitCounter = 0;

static uint8_t dataCounter = 0;
//bool_t ntpSyncProccess = FALSE;
//#ifdef NTP_DEBUG
//static uint16_t ntpRequestTimer = 3590;
//
//#else
//static uint16_t ntpRequestTimer = 0;
//#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t PWM_IncPercentTo(uint8_t xPwmDev, uint32_t xPercent)
{
	uint8_t ret = 0;
	switch (xPwmDev)
	{
		case PWM_PUMP:
			if (REG_PWM_PUMP < PWM_PercentToRegister(xPercent))
			{
				PWM_IncOnePercent(PWM_PUMP);
				gSetPwmPUMPFlag = 0;
			}
			else
			{
				ret = 1;

				if (gSetPwmPUMPFlag == 0)
				{
					gSetPwmPUMPFlag = 1;
					REG_PWM_PUMP = PWM_PercentToRegister(xPercent);
				}
			}
			break;

		case PWM_FAN_PULL_AIR:
			if (REG_PWM_PULL_AIR_FAN < PWM_PercentToRegister(xPercent))
			{
				PWM_IncOnePercent(PWM_FAN_PULL_AIR); // wentylator wyciągający powietrze
				gSetPwmFAN1Flag = 0;
			}
			else
			{
				ret = 1;
				if (gSetPwmFAN1Flag == 0)
				{
					gSetPwmFAN1Flag = 1;
					REG_PWM_PULL_AIR_FAN = PWM_PercentToRegister(xPercent);
				}
			}
			break;

		case PWM_FAN_PUSH_AIR:
			if (REG_PWM_PUSH_AIR_FAN < PWM_PercentToRegister(xPercent))
			{
				PWM_IncOnePercent(PWM_FAN_PUSH_AIR); // wentylator wciągający powietrze
				gSetPwmFAN2Flag = 0;
			}
			else
			{
				ret = 1;
				if (gSetPwmFAN2Flag == 0)
				{
					gSetPwmFAN2Flag = 1;
					REG_PWM_PUSH_AIR_FAN = PWM_PercentToRegister(xPercent);
				}
			}
			break;
		default:
			break;
	}

	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t PWM_DecPercentTo(uint8_t xPwmDev, uint32_t xPercent)
{
	uint8_t ret = 0;
	switch (xPwmDev)
	{
		case PWM_PUMP:
			if (REG_PWM_PUMP > PWM_PercentToRegister(xPercent))
			{
				PWM_DecOnePercent(PWM_PUMP);
				gSetPwmPUMPFlag = 0;
			}
			else
			{
				ret = 1;
				if (gSetPwmPUMPFlag == 0)
				{
					gSetPwmPUMPFlag = 1;
					REG_PWM_PUMP = PWM_PercentToRegister(xPercent);
				}
			}
			break;

		case PWM_FAN_PULL_AIR:
			if (REG_PWM_PULL_AIR_FAN > PWM_PercentToRegister(xPercent))
			{
				PWM_DecOnePercent(PWM_FAN_PULL_AIR); // wentylator wyciągający powietrze
				gSetPwmFAN1Flag = 0;
			}
			else
			{
				ret = 1;
				if (gSetPwmFAN1Flag == 0)
				{
					gSetPwmFAN1Flag = 1;
					REG_PWM_PULL_AIR_FAN = PWM_PercentToRegister(xPercent);
				}
			}
			break;

		case PWM_FAN_PUSH_AIR:
			if (REG_PWM_PUSH_AIR_FAN > PWM_PercentToRegister(xPercent))
			{
				PWM_DecOnePercent(PWM_FAN_PUSH_AIR); // wentylator wciągający powietrze
				gSetPwmFAN2Flag = 0;
			}
			else
			{
				ret = 1;
				if (gSetPwmFAN2Flag == 0)
				{
					gSetPwmFAN2Flag = 1;
					REG_PWM_PUSH_AIR_FAN = PWM_PercentToRegister(xPercent);
				}
			}
			break;
		default:
			break;
	}
	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void PWM_IncOnePercent(uint8_t xPwmDev)
{
	switch (xPwmDev)
	{
		case PWM_PUMP:
			REG_PWM_PUMP += (uint16_t) (((uint32_t) 1 * (TimerPeriod - 1)) / 100);//ONE_PERCENT; // pompa
			break;

		case PWM_FAN_PULL_AIR:
			REG_PWM_PULL_AIR_FAN += (uint16_t) (((uint32_t) 1 * (TimerPeriod - 1)) / 100);//ONE_PERCENT; // wentylator wyciągający powietrze
			break;

		case PWM_FAN_PUSH_AIR:
			REG_PWM_PUSH_AIR_FAN += (uint16_t) (((uint32_t) 1 * (TimerPeriod - 1)) / 100);//ONE_PERCENT; // wentylator wciągający powietrze
			break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void PWM_DecOnePercent(uint8_t xPwmDev)
{
	switch (xPwmDev)
	{
		case PWM_PUMP:
			REG_PWM_PUMP -= (uint16_t) (((uint32_t) 1 * (TimerPeriod - 1)) / 100);//ONE_PERCENT; // pompa
			break;

		case PWM_FAN_PULL_AIR:
			REG_PWM_PULL_AIR_FAN -= (uint16_t) (((uint32_t) 1 * (TimerPeriod - 1)) / 100);//ONE_PERCENT; // wentylator wyciągający powietrze
			break;

		case PWM_FAN_PUSH_AIR:
			REG_PWM_PUSH_AIR_FAN -= (uint16_t) (((uint32_t) 1 * (TimerPeriod - 1)) / 100);//ONE_PERCENT; // wentylator wciągający powietrze
			break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PWM_SetPercent(uint8_t xPwmDev, uint32_t xPercent)
{
	switch (xPwmDev)
	{
		case PWM_PUMP:
			REG_PWM_PUMP = PWM_PercentToRegister(xPercent); // pompa
			break;

		case PWM_FAN_PULL_AIR:
			REG_PWM_PULL_AIR_FAN = PWM_PercentToRegister(xPercent); // wentylator wyciągający powietrze
			break;

		case PWM_FAN_PUSH_AIR:
			REG_PWM_PUSH_AIR_FAN = PWM_PercentToRegister(xPercent); // wentylator wciągający powietrze
			break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static uint32_t PWM_PercentToRegister(uint32_t xPercent)
{
	return ((uint16_t) (((uint32_t) xPercent * (TimerPeriod - 1)) / 100));//(xPercent * ONE_PERCENT);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void FanSoftStart_Handler(void)
{
	if (gFansSoftStartFlag == TRUE)
	{
		if (PWM_FANSoftStart(TRUE) == TRUE)
		{
			gFansSoftStartFlag = FALSE;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainTimer_Handler(void)
{
	FanSoftStart_Handler();

	if (systimeTimeoutControl(&oneSecTimer, 1000))
	{
		if (atnel_Mode == ATNEL_MODE_TRANSPARENT)
		{
			dataCounter++;
			if (dataCounter == 3)
			{
				dataCounter = 0;
				atnel_TrCmdReqType = TRNSP_MEAS_DATA_REQ;
			}
		}

		displayData.pageCounter++;

		if (atnel_wait_change_mode == TRUE)
		{
			atnelWaitCounter++;
			if (atnelWaitCounter == 5)
			{
				atnelWaitCounter = 0;
				atnel_wait_change_mode = FALSE;
				Atnel_SetTransparentMode();
			}
		}

		Ntp_Handler();

    	if (xLightCounters.counterSeconds % 300 == 0)
    	{
    		FLASH_SaveLightCounters();
    	}

    	Lightning_Core();
		ADC_CalibrateProbes_Core();
//		Irrigation_Core();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void Lightning_Core(void)
{
	lastLightState = xLightControl.lightingState;
	if (xLightControl.timeOnHours == 0 && xLightControl.timeOffHours == 24)
	{
		GPIOx_ResetPin(LAMP_PORT, LAMP_PIN);
		xLightControl.lightingState = LIGHT_OFF;
		xLightCounters.counterSeconds = 0;
		xLightCounters.counterHours = 0;
	}
	else if (xLightControl.timeOnHours == 24 && xLightControl.timeOffHours == 0)
	{
		GPIOx_ResetPin(LAMP_PORT, LAMP_PIN);
		xLightControl.lightingState = LIGHT_ON;
		xLightCounters.counterSeconds = 0;
		xLightCounters.counterHours = 0;
	}
	else
	{
		xLightCounters.counterSeconds++;
		if (xLightCounters.counterSeconds == 3600)
		{
			xLightCounters.counterHours++;
			xLightCounters.counterSeconds = 0;
		}

		if (xLightControl.lightingState == LIGHT_ON) //lampa wlaczona
		{
			if (xLightControl.timeOnHours != 0 && xLightControl.timeOnHours != 24)
			{
				GPIOx_SetPin(LAMP_PORT, LAMP_PIN);
				if (xLightControl.timeOnHours == xLightCounters.counterHours)
				{
					xLightControl.lightingState = LIGHT_OFF;
					xLightCounters.counterHours = 0;
					xLightCounters.counterSeconds = 0;
				}
			}
		}
		else if (xLightControl.lightingState == LIGHT_OFF) //lampa wylaczona
		{
			if (xLightControl.timeOffHours != 0 && xLightControl.timeOffHours != 24)
			{
				GPIOx_ResetPin(LAMP_PORT, LAMP_PIN);
				if (xLightControl.timeOffHours == xLightCounters.counterHours)
				{
					xLightControl.lightingState = LIGHT_ON;
					xLightCounters.counterHours = 0;
					xLightCounters.counterSeconds = 0;
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PWM_PumpInit(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
//	uint16_t TimerPeriod = 0;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* GPIOA Configuration: Channel 1 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_2);

	/* Compute the value to be set in ARR regiter to generate signal frequency at 12kHz */
	TimerPeriod = (SystemCoreClock / PWM_FAN_CLK) - 1;

	/* TIM1 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);

	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* Channel 1, 2, 3 and 4 Configuration in PWM mode */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_OCInitStructure.TIM_Pulse = PWM_PercentToRegister(0);
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);

	/* TIM1 counter enable */
	TIM_Cmd(TIM2, ENABLE);

	/* TIM1 Main Output Enable */
	TIM_CtrlPWMOutputs(TIM2, ENABLE);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIM_CCMR2_OC3M_PWM1	0x0060	// OC3M[2:0] - PWM mode 1
#define TIM_CCMR2_OC4M_PWM1	0x6000	// OC4M[2:0] - PWM mode 1
void PWM_FansInit(void)
{

  /* TIM1 Configuration ---------------------------------------------------
   Generate 7 PWM signals with 4 different duty cycles:
   TIM1CLK = SystemCoreClock, Prescaler = 0, TIM1 counter clock = SystemCoreClock
   SystemCoreClock is set to 72 MHz for Low-density, Medium-density, High-density
   and Connectivity line devices and to 24 MHz for Low-Density Value line and
   Medium-Density Value line devices

   The objective is to generate 7 PWM signal at 17.57 KHz:
	 - TIM1_Period = (SystemCoreClock / 17570) - 1
   The channel 1 and channel 1N duty cycle is set to 50%
   The channel 2 and channel 2N duty cycle is set to 37.5%
   The channel 3 and channel 3N duty cycle is set to 25%
   The channel 4 duty cycle is set to 12.5%
   The Timer pulse is calculated as follows:
	 - ChannelxPulse = DutyCycle * (TIM1_Period - 1) / 100
  ----------------------------------------------------------------------- */
  /* Compute the value to be set in ARR regiter to generate signal frequency at 17.57 Khz */
//  TimerPeriod = (SystemCoreClock / 17570 ) - 1;
//  /* Compute CCR1 value to generate a duty cycle at 50% for channel 1 and 1N */
//  Channel1Pulse = (uint16_t) (((uint32_t) 5 * (TimerPeriod - 1)) / 10);
//  /* Compute CCR2 value to generate a duty cycle at 37.5%  for channel 2 and 2N */
//  Channel2Pulse = (uint16_t) (((uint32_t) 375 * (TimerPeriod - 1)) / 1000);
//  /* Compute CCR3 value to generate a duty cycle at 25%  for channel 3 and 3N */
//  Channel3Pulse = (uint16_t) (((uint32_t) 25 * (TimerPeriod - 1)) / 100);
//  /* Compute CCR4 value to generate a duty cycle at 12.5%  for channel 4 */
//  Channel4Pulse = (uint16_t) (((uint32_t) 125 * (TimerPeriod- 1)) / 1000);

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;


	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	/* GPIOA Configuration: Channel 3 and 4 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_1);

	const uint16_t prescaler = 48; //aby zmniejszyc F taktowania z 48Mhz na 1Mhz
	/* Compute the value to be set in ARR regiter to generate signal frequency at 30Hz */
	TimerPeriod = ((SystemCoreClock/prescaler) / PWM_FAN_CLK) - 1;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);

	/* Time Base configuration */
//	TIM_TimeBaseStructure.TIM_Prescaler = 0;
//	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
//	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
//	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
//	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseStructure.TIM_Prescaler = prescaler; //24
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod; //63999
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OCInitStructure.TIM_Pulse = PWM_PercentToRegister(0);
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);

	TIM_OCInitStructure.TIM_Pulse = PWM_PercentToRegister(0);
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);

	TIM_ARRPreloadConfig(TIM3,ENABLE);
	TIM_CtrlPWMOutputs(TIM3, ENABLE);
	TIM_Cmd(TIM3, ENABLE);

	gFansSoftStartFlag = 1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static uint8_t PWM_FANSoftStart(bool_t xStatus)
{
	uint8_t ret = 0;
	if (systimeTimeoutControl(&softStartTimer, 100))
	{
		if (xStatus)
		{
			PWM_IncPercentTo(PWM_FAN_PULL_AIR, 90);
			PWM_IncPercentTo(PWM_FAN_PUSH_AIR, 90);

//			_printParam("TIM3->CCR4=", TIM3->CCR4);
		}

		if (REG_PWM_PULL_AIR_FAN >= 3600)
		{
			ret = 1;
		}
	}

	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
