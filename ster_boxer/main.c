#include "global.h"
#include "stm32f0xx_flash.h"
///////////////////////////////////////////////////////////
// Wykorzystane peryferia:
// USART2 - WiFi
// ADC - pH
// I2C1 - rtc + czujniki
//


static systime_t measureOwireTimer = 0;
static systime_t measureI2cTimer = 0;
static systime_t saveConfigTimer = 0;
static systime_t shtInitTimer = 0;
static systime_t oWireInitTimer = 0;
static systime_t readTimeTimer = 0;

static DS18B20Sensor_t ds18b20_1 = {0};
static DS18B20Sensor_t ds18b20_2 = {0};
static uint8_t ucReset;
static ErrorStatus Error = SUCCESS;

static time_complex_t localTime;


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
    	ReceiveSerial_Handler();
    	FanSoftStart_Handler();
    	MainTimer_Handle();
    	PhMeasurementCalibration_Handler();
    	ClimateTempControl_Handler(&ds18b20_1);
		/************************************************************************/
		/*                       	 READING SENSORS                          	*/
		/************************************************************************/
    	if (systimeTimeoutControl(&readTimeTimer, 400))
    	{
#ifndef DEBUG_TERMINAL_USART
    		PCF8563_ReadTime(&rtcFullDate, I2C1);
#endif
    		timeUtcToLocalConv(&rtcFullDate, &localTime);
			displayMakeTimeString(timeString, &localTime);
			displayMakeDateString(dateString, &localTime);
			displayWeekDayConvert(localTime.wday, weekDayString);

			strcpy(displayData.time, timeString);
    	}

    	if (systimeTimeoutControl(&oWireInitTimer, 2000))
    	{
#ifndef OWIRE_OFF_MODE
    		ucReset = initializeConversion(&ds18b20_1);
        	ucReset = initializeConversion(&ds18b20_2);
#endif
    	}

    	if (systimeTimeoutControl(&measureOwireTimer, 3000))
		{
#ifndef OWIRE_OFF_MODE
    		readTemperature(&ds18b20_1);
			displayData.tempDS18B20_1_t = ds18b20_1.fTemp;
			readTemperature(&ds18b20_2);
			displayData.tempDS18B20_2_t = ds18b20_2.fTemp;
#endif
		}

    	if (systimeTimeoutControl(&shtInitTimer, 2500))
    	{
#ifndef I2C_OFF_MODE
    		Error = SHT21_SoftReset(I2C2, SHT21_ADDR);
#endif
    	}

    	if (systimeTimeoutControl(&measureI2cTimer, 5000))
		{
#ifndef I2C_OFF_MODE
			displayData.lux = TSL2561_ReadLux(&Error);

            uint16_t tempWord = 0;
            uint16_t humWord = 0;

        	tempWord = SHT21_MeasureTempCommand(I2C2, SHT21_ADDR, &Error);
        	humWord = SHT21_MeasureHumCommand(I2C2, SHT21_ADDR, &Error);

        	humWord = ((uint16_t)(SHT_HumData.msb_lsb[0]) << 8) | SHT_HumData.msb_lsb[1];
        	tempWord = ((uint16_t)(SHT_TempData.msb_lsb[0]) << 8) | SHT_TempData.msb_lsb[1];

        	displayData.tempSHT2x = SHT21_CalcTemp(tempWord);
        	displayData.humiditySHT2x = SHT21_CalcRH(humWord);
#endif
		}

		/************************************************************************/
		/*								IRRIGATION                              */
		/************************************************************************/
		Irrigation_PumpControll();
		Irrigation_WaterLevel();
		Irrigation_SoilMoisture_Handler();

    	Display_Handler();

    	if (flagsGlobal.udpSendMsg == TRUE)
    	{
    		char DataToSend[TX_BUFF_SIZE] = {0};
    		PrepareUdpString(displayData.lux, displayData.humiditySHT2x, displayData.tempSHT2x, ds18b20_1.fTemp, ds18b20_2.fTemp, DataToSend);
//    		USARTx_SendString(USART_COMM, (uint8_t*)DataToSend);
    		SerialPort_PutString(DataToSend);
    		flagsGlobal.udpSendMsg = FALSE;
    	}
    	else
    	{
    		if (calibrateFlags.calibrateDone == TRUE)
    		{
//    			USARTx_SendString(USART_COMM, (uint8_t*)"STA CD END"); //calibrate done
    			SerialPort_PutString((uint8_t*)"STA CD END");
    			calibrateFlags.calibrateDone = FALSE;
    		}
    	}

        if (systimeTimeoutControl(&saveConfigTimer, 1000)) //cykliczny zapis ustawien do FLASH co 5min
        {
        	if (xLightCounters.counterSeconds % 300 == 0)
        	{
        		FLASH_SaveLightCounters();
        	}
        }
	}

    return 0;
}

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
	Error = PCF8563_Init(I2C1);

	if (Error == ERROR)
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

	ErrorStatus Error = ERROR;
	ErrorStatus ErrorTmp1 = SUCCESS;
	ErrorStatus ErrorTmp2 = SUCCESS;

	ErrorTmp1 = TSL2561_Init(I2C2, TSL2561_GND_ADDR); // tutaj sa bledy i2c
	systimeDelayMs(30);
	ErrorTmp2 = TSL2561_Config(I2C2, TSL2561_GND_ADDR); // tutaj sa bledy i2c
	systimeDelayMs(10);

	Error = SHT21_SoftReset(I2C2, SHT21_ADDR);
#endif

	FLASH_ReadConfiguration();
	FLASH_ReadLightCounters();
//	FLASH_STORAGE_TEST();

#ifndef OWIRE_OFF_MODE
	memCopy(ds18b20_1.cROM, sensor1ROM, 8);
	memCopy(ds18b20_2.cROM, sensor2ROM, 8);

	ucReset = initializeConversion(&ds18b20_1);
	ucReset = initializeConversion(&ds18b20_2);
	systimeDelayMs(1000);
	readTemperature(&ds18b20_1);
	displayData.tempDS18B20_1_t = ds18b20_1.fTemp;
	readTemperature(&ds18b20_2);
	displayData.tempDS18B20_2_t = ds18b20_2.fTemp;
#endif

#ifndef I2C_OFF_MODE
	displayData.lux = TSL2561_ReadLux(&Error);
    uint16_t tempWord = 0;
    uint16_t humWord = 0;

	tempWord = SHT21_MeasureTempCommand(I2C2, SHT21_ADDR, &Error);
	humWord = SHT21_MeasureHumCommand(I2C2, SHT21_ADDR, &Error);

	humWord = ((uint16_t)(SHT_HumData.msb_lsb[0]) << 8) | SHT_HumData.msb_lsb[1];
	tempWord = ((uint16_t)(SHT_TempData.msb_lsb[0]) << 8) | SHT_TempData.msb_lsb[1];

	displayData.tempSHT2x = SHT21_CalcTemp(tempWord);
	displayData.humiditySHT2x = SHT21_CalcRH(humWord);
#endif

	displayData.page = 1;
	systimeDelayMs(2000);
	GLCD_ClearScreen();
}
