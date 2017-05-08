#include "global.h"
#include "boxer_timers.h"
#include "boxer_display.h"
#include "boxer_climate.h"
#include "boxer_ph.h"
#include "boxer_irrigation.h"
#include "stm32f0xx_flash.h"
#include "boxer_datastorage.h"
#include "hardware/TSL2561/tsl2561.h"
#include "hardware/PCF8563/pcf8563.h"
#include "misc.h"
///////////////////////////////////////////////////////////
static void PeripheralInit(void);
#ifndef DEBUG_TERMINAL_USART
static void I2C1_Init(void);
#endif
static void I2C2_Init(void);

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLKConfig(RCC_HCLK_Div1);

	systimeInit();
	PeripheralInit();

	static bool_t softStartDone = FALSE;
    while (TRUE)
	{
    	if (initFanPwm == TRUE)
    	{
    		if (PWM_FANSoftStart() == TRUE)
    		{
    			initFanPwm = FALSE;
    		}
    	}

    	MainTimer_Handler();
    	TransmitSerial_Handler();
    	ReceiveSerial_Handler();
    	RTC_Handler();
    	Climate_SensorsHandler();
    	Climate_TempCtrl_Handler();
    	Display_Handler();
    	PhProccess_Handler();
    	Irrigation_Handler();
	}

    return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////////////////
static void PeripheralInit(void)
{
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

	//todo sprawdzic jaki powinien byc stan na WIFI_PWR
	GPIOx_PinConfig(WIFI_PWR_PORT, Mode_Out, OSpeed_50MHz, OType_PP, OState_PU, WIFI_PWR_PIN);
	GPIOx_SetPin(WIFI_PWR_PORT, WIFI_PWR_PIN);

	//todo sprawdzic jaki powinien byc stan na WIFI_RST
	GPIOx_PinConfig(WIFI_RST_PORT, Mode_Out, OSpeed_50MHz, OType_PP, OState_PU, WIFI_RST_PIN);
	GPIOx_SetPin(WIFI_RST_PORT, WIFI_RST_PIN);

#ifndef BUZZER_OFF_MODE
	GPIOx_ResetPin(BUZZER_PORT, BUZZER_PIN);
	systimeDelayMs(800);
	GPIOx_SetPin(BUZZER_PORT, BUZZER_PIN);
#endif

	PWM_FansInit();
	PWM_PumpInit();

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
	ErrorStatus tslError = TSL2561_Init();
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
	FLASH_ReadLightCounters();
	FLASH_STORAGE_TEST();

#ifndef OWIRE_OFF_MODE
	OneWire_TimerInit();
	memCopy(sensorTempUp.cROM, sensor1ROM, 8);
	memCopy(sensorTempDown.cROM, sensor2ROM, 8);

	initializeConversion(&sensorTempUp);
	initializeConversion(&sensorTempDown);
	systimeDelayMs(760);
	readTemperature(&sensorTempUp);
	displayData.temp_up_t = sensorTempUp.fTemp;
	readTemperature(&sensorTempDown);
	displayData.temp_down_t = sensorTempDown.fTemp;
#endif

#ifndef I2C_OFF_MODE

	displayData.lux = TSL2561_ReadLux(&tslError);

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
	ErrorStatus shtError = SUCCESS;

	uint8_t i2cErrCounter = 0;
	while (displayData.temp_middle_t <= 0)
	{
		i2cErrCounter++;
		if (i2cErrCounter == 10)
		{
			break;
		}

		shtError = SHT21_SoftReset(I2C2, SHT21_ADDR);
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

	    tempWord = SHT21_MeasureTempCommand(I2C2, SHT21_ADDR, &shtError);

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
	    humWord = SHT21_MeasureHumCommand(I2C2, SHT21_ADDR, &shtError);

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

	displayData.page = 1;
	GLCD_ClearScreen();

#ifdef TURN_OFF_FIRST_NTP_REQ
	atnel_Mode = ATNEL_MODE_TRANSPARENT;
#else
	Ntp_SendRequest();
#endif
}

#ifndef DEBUG_TERMINAL_USART
static void I2C1_Init(void)
{
	GPIOx_ClockConfig(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource6, GPIOx_AF_1); //scl
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource7, GPIOx_AF_1); //sda
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C1_SCL);
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C1_SDA);

	RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_SoftwareResetCmd(I2C2, ENABLE);
	I2C_SoftwareResetCmd(I2C2, DISABLE);

	I2C_InitTypeDef  I2C_InitStructure;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Timing = 0x00100000;
	I2C_Init(I2C1, &I2C_InitStructure);

	SYSCFG_I2CFastModePlusConfig(SYSCFG_I2CFastModePlus_PB6, DISABLE);
	SYSCFG_I2CFastModePlusConfig(SYSCFG_I2CFastModePlus_PB7, DISABLE);

	I2C_Cmd(I2C1, ENABLE);
}
#endif

static void I2C2_Init(void)
{
	GPIOx_ClockConfig(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource10, GPIOx_AF_1); //scl
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource11, GPIOx_AF_1); //sda
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C2_SCL);
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C2_SDA);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	I2C_SoftwareResetCmd(I2C2, ENABLE);
	I2C_SoftwareResetCmd(I2C2, DISABLE);

	I2C_InitTypeDef  I2C_InitStructure;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0x00;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Timing = 0x10800000;
	I2C_Init(I2C2, &I2C_InitStructure);

	I2C_Cmd(I2C2, ENABLE);
}

