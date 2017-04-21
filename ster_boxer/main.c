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
static void I2C1_Init(void);
static void I2C2_Init(void);

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();
	systimeInit();
	PeripheralInit();

    while (TRUE)
	{
    	MainTimer_Handler();
    	TransmitSerial_Handler();
    	ReceiveSerial_Handler();
    	RTC_Handler();
    	Climate_SensorsHandler();
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
	systimeDelayMs(400);
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
		DEBUG_SendString("blad inicjalizacji RTC\r\n");
	}
#else
	DEBUG_Init();
#endif

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

#ifndef I2C_OFF_MODE
	I2C2_Init();
	ErrorStatus tslError = TSL2561_Init();
	ErrorStatus shtError = SHT21_SoftReset(I2C2, SHT21_ADDR);
#endif

	FLASH_ReadConfiguration();
	FLASH_ReadLightCounters();
	FLASH_STORAGE_TEST();

#ifndef OWIRE_OFF_MODE
	memCopy(ds18b20_1.cROM, sensor1ROM, 8);
	memCopy(ds18b20_2.cROM, sensor2ROM, 8);

	initializeConversion(&ds18b20_1);
	initializeConversion(&ds18b20_2);
	systimeDelayMs(800);
	readTemperature(&ds18b20_1);
	displayData.tempDS18B20_1_t = ds18b20_1.fTemp;
	readTemperature(&ds18b20_2);
	displayData.tempDS18B20_2_t = ds18b20_2.fTemp;
#endif

#ifndef I2C_OFF_MODE

	displayData.lux = TSL2561_ReadLux(&tslError);
	uint16_t tempWord = 0;
	uint16_t humWord = 0;

//    tempWord = SHT21_MeasureTempCommand(I2C2, SHT21_ADDR, &shtError);
//    humWord = SHT21_MeasureHumCommand(I2C2, SHT21_ADDR, &shtError);

	humWord  = ((uint16_t)(SHT_HumData.msb_lsb[0])  << 8) | SHT_HumData.msb_lsb[1];
	tempWord = ((uint16_t)(SHT_TempData.msb_lsb[0]) << 8) | SHT_TempData.msb_lsb[1];

	displayData.tempSHT2x 		= SHT21_CalcTemp(tempWord);
	displayData.humiditySHT2x 	= SHT21_CalcRH(humWord);
#endif

	Irrigation_CheckSoilMoisture();

	displayData.page = 1;
	GLCD_ClearScreen();

	Ntp_SendRequest();

	peripheralsInit = TRUE;
}

static void I2C1_Init(void)
{
	GPIOx_ClockConfig(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource6, GPIOx_AF_1); //scl
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource7, GPIOx_AF_1); //sda
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C1_SCL);
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C1_SDA);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_InitTypeDef  I2C_InitStructure;
	RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);

	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Timing = 0x40B22536;//0x00701863;//0x10805E89; //0x40B22536; //100khz
	I2C_Init(I2C1, &I2C_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_SoftwareResetCmd(I2C1, ENABLE);
	I2C_SoftwareResetCmd(I2C1, DISABLE);
}

static void I2C2_Init(void)
{
	GPIOx_ClockConfig(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource10, GPIOx_AF_1); //scl
	GPIOx_PinAFConfig(GPIOB, GPIOx_PinSource11, GPIOx_AF_1); //sda
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C2_SCL);
	GPIOx_PinConfig(GPIOB, Mode_AF, OSpeed_50MHz, OType_OD, OState_PU, I2C2_SDA);

	I2C_InitTypeDef  I2C_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
	I2C_InitStructure.I2C_DigitalFilter = 0x00;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Timing = 0x40B22536;//0x502044F3;//0x10805E89; //0x40B22536; //100khz
	I2C_Init(I2C2, &I2C_InitStructure);

	I2C_SoftwareResetCmd(I2C2, ENABLE);
	I2C_SoftwareResetCmd(I2C2, DISABLE);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	I2C_Cmd(I2C2, ENABLE);

	I2C_SoftwareResetCmd(I2C2, ENABLE);
	I2C_SoftwareResetCmd(I2C2, DISABLE);
}

