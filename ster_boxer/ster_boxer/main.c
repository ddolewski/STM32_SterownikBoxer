#include "global.h"
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
static systime_t fansSoftStartTimer = 0;

static DS18B20Sensor_t ds18b20_1 = {0};
static DS18B20Sensor_t ds18b20_2 = {0};
static uint8_t ucReset;
static ErrorStatus Error = FALSE;

static void PeripheralInit(void);

#include "stm32f0xx_flash.h"

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
        //RCC_PCLK2Config(RCC_HCLK_Div1);
        // PCLK1 = HCLK/2
        //RCC_PCLK1Config(RCC_HCLK_Div2);
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
//	konf_zegary();
	systimeInit();
	PeripheralInit();

    while (TRUE)
	{
    	PcCommunication_Handler();
    	FanSoftStart_Handler();
    	MainTimer_Handle();
    	PCF8583_ReadDate(I2C1, &rtcDate);
    	PhMeasurementCalibration_Handler();
    	ClimateTempControl_Handler(&ds18b20_1);
		/************************************************************************/
		/*                       	 READING SENSORS                          	*/
		/************************************************************************/
    	
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
    		PrepareUdpString(displayData.lux, displayData.humiditySHT2x, displayData.tempSHT2x, ds18b20_1.fTemp, ds18b20_2.fTemp, DataToSend);
    		USARTx_SendString(USART_COMM, (uint8_t*)DataToSend);
    		flagsGlobal.udpSendMsg = FALSE;
    	}
    	else
    	{
    		if (calibrateFlags.calibrateDone == TRUE)
    		{
    			USARTx_SendString(USART_COMM, (uint8_t*)"STA CD END"); //calibrate done
    			calibrateFlags.calibrateDone = FALSE;
    		}
    	}

        if (systimeTimeoutControl(&saveConfigTimer, 1000)) //cykliczny zapis ustawien do FLASH co 5min
        {
        	if (lightCounters.counterSeconds % 300 == 0)
        	{
            	uint32_uint8_converter_t u32ToU8Array;
            	u32ToU8Array.u32Value = lightCounters.counterSeconds;
            	uint8_t lampHouCounter = lightCounters.counterHours;

            	PCF8583_RegWrite(I2C1, PCF8583_ADDR, 0x08, u32ToU8Array.u8ValueArray[0]);
            	PCF8583_RegWrite(I2C1, PCF8583_ADDR, 0x09, u32ToU8Array.u8ValueArray[1]);
            	PCF8583_RegWrite(I2C1, PCF8583_ADDR, 0x0A, u32ToU8Array.u8ValueArray[2]);
            	PCF8583_RegWrite(I2C1, PCF8583_ADDR, 0x0B, u32ToU8Array.u8ValueArray[3]);
            	PCF8583_RegWrite(I2C1, PCF8583_ADDR, 0x0C, lampHouCounter);
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
	UART2_Init();

	GPIOx_PinConfig(SOIL_MOIST_PORT, Mode_In, OSpeed_50MHz, OType_OD, OState_PU, SOIL_MOIST_PIN);

	GPIOx_PinConfig(WATER_LEVEL_PORT, Mode_In, OSpeed_50MHz, OType_OD, OState_PU, WATER_LEVEL_PIN);

	GPIOx_PinConfig(LAMP_PORT, Mode_Out, OSpeed_50MHz, OType_PP, OState_PD, LAMP_PIN);
	GPIOx_ResetPin(LAMP_PORT, LAMP_PIN);

	GPIOx_PinConfig(SOLENOID_PORT, Mode_Out, OSpeed_50MHz, OType_PP, OState_PD, SOLENOID_PIN);
	GPIOx_ResetPin(SOLENOID_PORT, SOLENOID_PIN);

	//todo sprawdzic jaki powinien byc stan na BUZZER_PIN
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

	I2C1_Init();

// testowe kasowanie/ustawianie pamieci RAM PCF'a
//	uint32_uint8_converter_t u32ToU8Array;
//	u32ToU8Array.u32Value = 477533;
//	uint8_t lampHouCounter = 11;

//	ErrorStatus error1 = SUCCESS;
//
//	error1 = PCF8583_RegWrite(I2C1, PCF8583_ADDR, 0x08, 0);
//	error1 = PCF8583_RegWrite(I2C1, PCF8583_ADDR, 0x09, 0);
//	error1 = PCF8583_RegWrite(I2C1, PCF8583_ADDR, 0x0A, 0);
//	error1 = PCF8583_RegWrite(I2C1, PCF8583_ADDR, 0x0B, 0);
//	error1 = PCF8583_RegWrite(I2C1, PCF8583_ADDR, 0x0C, 0);

	// przywrocenie licznikow lampy
	ErrorStatus error = SUCCESS;
	uint32_uint8_converter_t u8ArrayToU32;
	uint8_t lampHouCounter = 0;

	u8ArrayToU32.u8ValueArray[0] = PCF8583_RegRead(I2C1, PCF8583_ADDR, 0x08, &error);
	u8ArrayToU32.u8ValueArray[1] = PCF8583_RegRead(I2C1, PCF8583_ADDR, 0x09, &error);
	u8ArrayToU32.u8ValueArray[2] = PCF8583_RegRead(I2C1, PCF8583_ADDR, 0x0A, &error);
	u8ArrayToU32.u8ValueArray[3] = PCF8583_RegRead(I2C1, PCF8583_ADDR, 0x0B, &error);
	lampHouCounter = PCF8583_RegRead(I2C1, PCF8583_ADDR, 0x0C, &error);

	lightCounters.counterHours = lampHouCounter;
	lightCounters.counterSeconds = u8ArrayToU32.u32Value;

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

	ReadConfiguration();
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

	displayData.lux = TSL2561_ReadLux(&Error);
    uint16_t tempWord = 0;
    uint16_t humWord = 0;

	tempWord = SHT21_MeasureTempCommand(I2C2, SHT21_ADDR, &Error);
	humWord = SHT21_MeasureHumCommand(I2C2, SHT21_ADDR, &Error);

	humWord = ((uint16_t)(SHT_HumData.msb_lsb[0]) << 8) | SHT_HumData.msb_lsb[1];
	tempWord = ((uint16_t)(SHT_TempData.msb_lsb[0]) << 8) | SHT_TempData.msb_lsb[1];

	displayData.tempSHT2x = SHT21_CalcTemp(tempWord);
	displayData.humiditySHT2x = SHT21_CalcRH(humWord);

	displayData.page = 1;
	//systimeDelayMs(2000);
	GLCD_ClearScreen();
}
