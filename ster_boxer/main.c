#include "global.h"
#include "boxer_timers.h"
#include "boxer_display.h"
#include "boxer_climate.h"
#include "boxer_ph.h"
#include "boxer_irrigation.h"
#include "boxer_datastorage.h"
#include "i2c.h"
#include "tsl2561.h"
#include "pcf8563.h"
#include "misc.h"
#include "stm32f0xx_flash.h"
////////////////////////////////////////////////////////////////////////////////////////////////
static void PeripheralInit(void);


int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLKConfig(RCC_HCLK_Div1);
	PeripheralInit();

    while (TRUE)
	{
    	MainTimer_Handler();
    	TransmitSerial_Handler();
    	ReceiveSerial_Handler();
    	Climate_TempCtrl_Handler();
    	Climate_SensorsHandler();
    	Display_Handler();
    	PhProccess_Handler();
    	Irrigation_Handler();
	}

    return ERROR;
}
////////////////////////////////////////////////////////////////////////////////////////////////
static void PeripheralInit(void)
{
	systimeInit();

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOFEN;

	GLCD_Init();
	SerialPort_Init();

	GPIOx_PinConfig(SOIL_MOIST_PORT, Mode_In, OSpeed_50MHz, OType_OD, OState_PU, SOIL_MOIST_PIN);
	GPIOx_PinConfig(SOIL_MOIST_EN, Mode_Out, OSpeed_50MHz, OType_PP, OState_PD, SOIL_MOIST_EN_PIN);

	GPIOx_PinConfig(WATER_LEVEL_PORT, Mode_In, OSpeed_50MHz, OType_OD, OState_PU, WATER_LEVEL_PIN);

	GPIOx_PinConfig(LAMP_PORT, Mode_Out, OSpeed_50MHz, OType_PP, OState_PD, LAMP_PIN);
	GPIOx_ResetPin(LAMP_PORT, LAMP_PIN);

	GPIOx_PinConfig(SOLENOID_PORT, Mode_Out, OSpeed_50MHz, OType_PP, OState_PD, SOLENOID_PIN);
	GPIOx_ResetPin(SOLENOID_PORT, SOLENOID_PIN);

	GPIOx_PinConfig(BUZZER_PORT, Mode_Out, OSpeed_50MHz, OType_PP, OState_PU, BUZZER_PIN);
	GPIOx_SetPin(BUZZER_PORT, BUZZER_PIN);

	GPIOx_PinConfig(WIFI_PWR_PORT, Mode_Out, OSpeed_50MHz, OType_PP, OState_PU, WIFI_PWR_PIN);
	GPIOx_SetPin(WIFI_PWR_PORT, WIFI_PWR_PIN);

	GPIOx_PinConfig(WIFI_RST_PORT, Mode_Out, OSpeed_50MHz, OType_PP, OState_PU, WIFI_RST_PIN);
	GPIOx_SetPin(WIFI_RST_PORT, WIFI_RST_PIN);

#ifndef BUZZER_OFF_MODE
	GPIOx_ResetPin(BUZZER_PORT, BUZZER_PIN);
	systimeDelayMs(25);
	GPIOx_SetPin(BUZZER_PORT, BUZZER_PIN);
#endif

    ADC_DMA_Init();

#ifndef DEBUG_TERMINAL_USART
	I2C1_Init();
	ErrorStatus rtcError = PCF8563_Init(I2C1);

	if (rtcError == ERROR)
	{
		_error("blad inicjalizacji RTC\r\n");
	}
#else
	DEBUG_Init();
	_printString("TEST\r\n");
#endif

#ifndef DEBUG_TERMINAL_USART
#ifdef RTC_WRITE_TEST
	time_complex_t timeTest;
	timeTest.sec = 0;
	timeTest.min = 0;
	timeTest.hour = 0;
	timeTest.mday = 0;
	timeTest.wday = 0;
	timeTest.month = 0;
	timeTest.year = 0;

	static time_complex_t timeUtc 	= {2000, 1, 1, 1, 0, 0, 0};
	timeLocalToUtcConv(&timeTest, &timeUtc);
	PCF8563_WriteTime(&timeUtc, I2C1);
#endif
#endif

#ifndef I2C_OFF_MODE
	I2C2_Init();
	errorTsl = TSL2561_Init();
#ifdef MEASURE_LOGS
	if (tslError == ERROR)
	{
		_error("TSL2561 init error");
	}
	else
	{
		_printString("TSL2561 init ok\r\n");
	}
#endif

#endif

	FLASH_ReadConfiguration();
	FLASH_STORAGE_TEST();

	OneWire_TimerInit();

#ifndef OWIRE_OFF_MODE
	memCopy(sensorTempUp.cROM, sensor1ROM, 8);
	memCopy(sensorTempDown.cROM, sensor2ROM, 8);

	uint8_t oWireErrCounter = 0;
	while (displayData.temp_up_t == 0)
	{
		oWireErrCounter++;
		if (oWireErrCounter == 20)
		{
			oWireErrCounter = 0;
			break;
		}

		initializeConversion(&sensorTempUp);
		systimeDelayMs(760);
		readTemperature(&sensorTempUp);
		displayData.temp_up_t = sensorTempUp.fTemp;
	}

	while (displayData.temp_down_t == 0)
	{
		oWireErrCounter++;
		if (oWireErrCounter == 20)
		{
			oWireErrCounter = 0;
			break;
		}

		initializeConversion(&sensorTempDown);
		systimeDelayMs(760);
		readTemperature(&sensorTempDown);
		displayData.temp_down_t = sensorTempDown.fTemp;
	}

#endif

#ifndef I2C_OFF_MODE
	errorTsl = TSL2561_ReadLux(&displayData.lux);

#ifdef MEASURE_LOGS
	if (tslError == ERROR)
	{
		_error("TSL2561 read lux error");
	}
	else
	{
		_printString("TSL2561 read lux ok\r\n");
	}
#endif
	uint16_t tempWord = 0;
	uint16_t humWord = 0;

	uint8_t i2cErrCounter = 0;
	while (displayData.temp_middle_t <= 0)
	{
		i2cErrCounter++;
		if (i2cErrCounter == 5)
		{
			break;
		}

		errorSht = SHT21_SoftReset(I2C2, SHT21_ADDR);
		systimeDelayMs(50);
	#ifdef MEASURE_LOGS
		if (tslError == ERROR)
		{
			_error("SHT21 reset error");
		}
		else
		{
			_printString("SHT21 reset ok\r\n");
		}
	#endif

	    tempWord = SHT21_MeasureTempCommand(I2C2, SHT21_ADDR, &errorSht);

	#ifdef MEASURE_LOGS
		if (shtError == ERROR)
		{
			_error("SHT21 meas temp error");
		}
		else
		{
			_printString("SHT21 meas temp ok\r\n");
		}
	#endif
	    humWord = SHT21_MeasureHumCommand(I2C2, SHT21_ADDR, &errorSht);

	#ifdef MEASURE_LOGS
		if (shtError == ERROR)
		{
			_error("SHT21 meas hum error");
		}
		else
		{
			_printString("SHT21 meas hum ok\r\n");
		}
	#endif
		humWord  = ((uint16_t)(SHT_HumData.msb_lsb[0])  << 8) | SHT_HumData.msb_lsb[1];
		tempWord = ((uint16_t)(SHT_TempData.msb_lsb[0]) << 8) | SHT_TempData.msb_lsb[1];

		displayData.temp_middle_t 	= SHT21_CalcTemp(tempWord);
		displayData.humiditySHT2x 	= SHT21_CalcRH(humWord);
	}
#endif

	Irrigation_CheckSoilMoisture();

	PWM_FansInit();

	while (softStartDone == FALSE)
	{
		SoftStart_Handler();
		delay_us__(150);
	}

//	PWM_PumpInit();

#ifdef TURN_OFF_FIRST_NTP_REQ
	atnel_Mode = ATNEL_MODE_TRANSPARENT;
#else
	Ntp_SendRequest();
#endif

	displayData.page = PAGE_1;
	GLCD_ClearScreen();

#ifdef I2C_OFF_MODE
	GPIOx_PinConfig(GPIOB, Mode_Out, OSpeed_50MHz, OType_PP, OState_PU, GPIOx_Pin_10);
	GPIOx_SetPin(GPIOB, GPIOx_Pin_10);
#endif
}
////////////////////////////////////////////////////////////////////////////////////////////////


