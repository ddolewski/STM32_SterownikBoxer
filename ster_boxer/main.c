#include "global.h"
#include "fifo.h"
#include "boxer_timers.h"
#include "stm32f0xx_flash.h"
///////////////////////////////////////////////////////////
// Wykorzystane peryferia:
// USART2 - WiFi
// ADC - pH
// I2C1 - rtc + czujniki
//

static systime_t saveConfigTimer = 0;

static void PeripheralInit(void);

void konf_zegary(void)
{
  ErrorStatus HSEStartUpStatus;

  // Reset ustawien RCC
  RCC_DeInit();
  // Wlacz HSE
  RCC_HSEConfig(RCC_HSE_ON);
  // Czekaj za HSE bedzie gotowy
  HSEStartUpStatus = RCC_WaitForHSEStartUp();
  if(HSEStartUpStatus == SUCCESS)
  {
        FLASH_PrefetchBufferCmd(ENABLE);

        // zwloka dla pamieci Flash
        FLASH_SetLatency(FLASH_Latency_1);
        // HCLK = SYSCLK
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        // PCLK2 = HCLK
//        RCC_PCLK2Config(RCC_HCLK_Div1);
        // PCLK1 = HCLK/2
//        RCC_PCLK1Config(RCC_HCLK_Div2);
        RCC_PCLKConfig(RCC_HCLK_Div1);
        // PLLCLK = 8MHz * 9 = 72 MHz
        RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_4);
        // Wlacz PLL
        RCC_PLLCmd(ENABLE);
        // Czekaj az PLL poprawnie sie uruchomi
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        // PLL bedzie zrodlem sygnalu zegarowego
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        // Czekaj az PLL bedzie sygnalem zegarowym systemu
        while(RCC_GetSYSCLKSource() != 0x08);
  }
}

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();
	systimeInit();
	PeripheralInit();

    while (TRUE)
	{
    	TransmitSerial_Handler();
    	ReceiveSerial_Handler();
    	FanSoftStart_Handler();
    	MainTimer_Handler();
    	PhMeasurementCalibration_Handler();
    	Climate_TempCtrl_Handler();
    	Climate_SensorsHandler();
    	Irrigation_Handler();
    	Display_Handler();
	}

    return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////
static void PeripheralInit(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
	RCC->AHBENR |= RCC_AHBENR_GPIOFEN;

	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_GoTo(0,0);
	GLCD_WriteString(UC"Inicjalizacja...");
	SerialPort_Init();

	GPIOx_PinConfig(SOIL_MOIST_PORT, Mode_In, OSpeed_50MHz, OType_OD, OState_PU, SOIL_MOIST_PIN);

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
	systimeDelayMs(1000);
	GPIOx_SetPin(BUZZER_PORT, BUZZER_PIN);
#endif

	PWM_FansInit();
	//PWM_PumpInit();

    ADC_DMA_Init();

#ifndef DEBUG_TERMINAL_USART
	I2C1_Init();
	ErrorStatus rtcError = PCF8563_Init(I2C1);

	if (rtcError == ERROR)
	{
		//error
	}
#else
	DEBUG_Init();
#endif

#ifdef RTC_WRITE_TEST
	time_complex_t timeTest;
	timeTest.sec = 0;
	timeTest.min = 26;
	timeTest.hour = 10;
	timeTest.mday = 22;
	timeTest.wday = 6;
	timeTest.month = 1;
	timeTest.year = 2017;

	static time_complex_t timeUtc 	= {2000, 1, 1, 1, 0, 0, 0};
	timeLocalToUtcConv(&timeTest, &timeUtc);
	PCF8563_WriteTime(&timeUtc, I2C1);
#endif

#ifndef I2C_OFF_MODE
	I2C2_Init();

//	ErrorStatus Error = ERROR;
//	ErrorStatus ErrorTmp1 = SUCCESS;
//	ErrorStatus ErrorTmp2 = SUCCESS;
	ErrorStatus tslError = SUCCESS;
	tslError = TSL2561_Init(I2C2, TSL2561_GND_ADDR); // tutaj sa bledy i2c
	systimeDelayMs(30);
	tslError = TSL2561_Config(I2C2, TSL2561_GND_ADDR); // tutaj sa bledy i2c
	systimeDelayMs(10);

	ErrorStatus shtError = SHT21_SoftReset(I2C2, SHT21_ADDR);
#endif

	FLASH_ReadConfiguration();
	FLASH_ReadLightCounters();
//	FLASH_STORAGE_TEST();

#ifndef OWIRE_OFF_MODE
	memCopy(ds18b20_1.cROM, sensor1ROM, 8);
	memCopy(ds18b20_2.cROM, sensor2ROM, 8);

	initializeConversion(&ds18b20_1);
	initializeConversion(&ds18b20_2);
	systimeDelayMs(1000);
	readTemperature(&ds18b20_1);
	displayData.tempDS18B20_1_t = ds18b20_1.fTemp;
	readTemperature(&ds18b20_2);
	displayData.tempDS18B20_2_t = ds18b20_2.fTemp;
#endif

#ifndef I2C_OFF_MODE

	displayData.lux = TSL2561_ReadLux(&tslError);
    uint16_t tempWord = 0;
    uint16_t humWord = 0;

	tempWord = SHT21_MeasureTempCommand(I2C2, SHT21_ADDR, &shtError);
	humWord = SHT21_MeasureHumCommand(I2C2, SHT21_ADDR, &shtError);

	humWord = ((uint16_t)(SHT_HumData.msb_lsb[0]) << 8) | SHT_HumData.msb_lsb[1];
	tempWord = ((uint16_t)(SHT_TempData.msb_lsb[0]) << 8) | SHT_TempData.msb_lsb[1];

	displayData.tempSHT2x = SHT21_CalcTemp(tempWord);
	displayData.humiditySHT2x = SHT21_CalcRH(humWord);
#endif

	displayData.page = 1;
	systimeDelayMs(2000);
	GLCD_ClearScreen();
}
