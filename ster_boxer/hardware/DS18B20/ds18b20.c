#include "ds18b20.h"
#include "string.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

const uint8_t sensor1ROM[8]={40, 255, 8, 153, 100, 20, 3, 10};
//const uint8_t sensor2ROM[8]={40, 21, 46, 135, 4, 0, 0, 53}; // ROM starego czujnika
const uint8_t sensor2ROM[8]={40, 255, 180, 92, 139, 22, 3, 213};

volatile uint8_t toggle = 0;
volatile uint8_t delayFlag = 0;

GPIO_InitTypeDef GPIO_InitStructure;


void OneWire_TimerInit(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN; //timer clock enable
	TIM6->CR1 |= TIM_CR1_CEN; 			//Counter enable
	TIM6->PSC = 47; 					//Timer prescaler
	TIM6->CNT = 0;
}

void delay_us__(uint32_t us)
{
	TIM6->CNT = 0;
	while ((us - TIM6->CNT) > 0);
}

void delay_ms__(uint16_t delay) //up to 65535 ms (~65s)
{
	uint16_t i;
	for (i = 0; i < delay; i++)
	{
		delay_us__(1000);
	}
}

void OneWire_Low(void)
{
	GPIOx_ResetPin(DS18B20_PORT, DS18B20_PIN);
}

void OneWire_High(void)
{
	GPIOx_SetPin(DS18B20_PORT, DS18B20_PIN);
}

void OneWire_In(void)
{
	GPIOx_PinConfig(DS18B20_PORT, Mode_In, 0, 0, OState_PD, DS18B20_PIN);
}

void OneWire_Out(void)
{
	GPIOx_PinConfig(DS18B20_PORT, Mode_Out, 0, OType_PP, OState_PD, DS18B20_PIN);
}
//Funkcja resetuje magistrale i oczekuje na impuls Presence
uint8_t uc1Wire_ResetPulse(void)
{
	uint8_t ucPresenceImpulse;

	OneWire_Out();
	OneWire_Low();
	delay_us__(480); //minimum 480us
	OneWire_In();
	delay_us__(60); //15-60us

	if (GPIOx_ReadInputPin(DS18B20_PORT, DS18B20_PIN))
	{
		ucPresenceImpulse = 1;
	}
	else
	{
		ucPresenceImpulse = 0;
	}

	 delay_us__(420);

	if (GPIOx_ReadInputPin(DS18B20_PORT, DS18B20_PIN))
	{
		ucPresenceImpulse = 1;
	}
	else
	{
		ucPresenceImpulse = 0;
	}

	return ucPresenceImpulse;
}

//Funkcja wysyla na magistrale pojedynczy bit
void v1Wire_SendBit(uint8_t cBit)
{
	OneWire_Out();
	OneWire_Low();

	delay_us__(1);

	if (cBit == 1)
	{
		OneWire_High();
	}

	delay_us__(60);
	OneWire_High();
}

//Funkcja odbiera bit z magistrali
uint8_t uc1Wire_ReadBit(void)
{
	uint8_t ucBit;

	GPIOx_PinConfig(DS18B20_PORT, Mode_Out, 0, OType_PP, OState_PD, DS18B20_PIN);
	DS18B20_PORT->BRR = DS18B20_PIN;
	delay_us__(1);

	GPIOx_PinConfig(DS18B20_PORT, Mode_In, 0, 0, OState_PD, DS18B20_PIN);
	delay_us__(14);

	if (GPIOx_ReadInputPin(DS18B20_PORT, DS18B20_PIN))
	{
		ucBit = 1;
	}
	else
	{
		ucBit = 0;
	}

	delay_us__(45);
	return ucBit;
}

//Funkcja wysyla bajt na magistrale
void v1Wire_SendByte(uint8_t ucByteValue)
{
	uint8_t ucCounter;
	uint8_t ucValueToSend;

	for (ucCounter = 0; ucCounter < 8; ucCounter++)
	{
		ucValueToSend = ucByteValue >> ucCounter;
		ucValueToSend &= (1 << 0);
		v1Wire_SendBit(ucValueToSend);
	}

	delay_us__(100);
}

//Funkcja odbiera bajt z magistrali
uint8_t uv1Wire_ReadByte(void)
{
	uint8_t ucCounter;
	uint8_t ucReadByte = 0;

	for (ucCounter = 0; ucCounter < 8; ucCounter++)
	{
		if (uc1Wire_ReadBit())
		{
			ucReadByte |= (1 << ucCounter);
			delay_us__(15);
		}
	}

	return ucReadByte;
}

//funkcja odczytuje unikalny kod pojedynczego czujnika
uint8_t readROM(uint8_t *buffer)
{
	uint8_t ucReset;
	uint32_t i;
	ucReset = uc1Wire_ResetPulse();
	v1Wire_SendByte(0x33);

	for(i = 0; i < 8; i++)
	{
		buffer[i] = uv1Wire_ReadByte();
	}

	return ucReset;
}

//funkcja wysyla unikalny kod czujnika
void sendROM(uint8_t * table)
{
	uint32_t i;
	for (i = 0; i < 8; i++)
	{
		v1Wire_SendByte(table[i]);
	}
}

//funkcja inicjalizujaca konwersje temperatury danego czujnika
uint8_t initializeConversion(DS18B20Sensor_t * sensor)
{
	uint8_t ucReset;
	ucReset = uc1Wire_ResetPulse();
	
	if (ucReset == 1)
	{
		v1Wire_SendByte(0x55);
		sendROM(sensor->cROM);
		v1Wire_SendByte(0x44);
	}
	
	return ucReset;
}

uint8_t CRC8(uint8_t *inData, uint8_t len)
{
   uint8_t crc;
   crc = 0;
   for(; len; len--)
   {
      crc ^= *inData++;
      crc ^= (crc << 3) ^ (crc << 4) ^ (crc << 6);
      crc ^= (crc >> 4) ^ (crc >> 5);
   }
   return crc;
}

uint8_t readTemperature(DS18B20Sensor_t * sensor)
{
	uint8_t ucReset;
	uint8_t dsData[8] = {0};
	ucReset = uc1Wire_ResetPulse();

	v1Wire_SendByte(0x55);
	sendROM(sensor->cROM);
	v1Wire_SendByte(0xBE);
	sensor->cTempL = uv1Wire_ReadByte();
	sensor->cTempH = uv1Wire_ReadByte();
	dsData[0] = sensor->cTempL;  //temperature LSB
	dsData[1] = sensor->cTempH;  //temperature MSB

	dsData[2] =  uv1Wire_ReadByte(); //Th register or user byte 1
	dsData[3] =  uv1Wire_ReadByte(); //Tl register or user byte 2
	dsData[4] =  uv1Wire_ReadByte(); //configuration register
	dsData[5] =  uv1Wire_ReadByte(); //reserved (0xFF)
	dsData[6] =  uv1Wire_ReadByte(); //reserved (0x0C)
	dsData[7] =  uv1Wire_ReadByte(); //reserved (0x10)
	uint8_t crcDS18B20 = uv1Wire_ReadByte(); //CRC8 Dallas standard
	ucReset = uc1Wire_ResetPulse();

	uint8_t crcCalculated = CRC8(dsData, 8);

	if (crcDS18B20 == crcCalculated)
	{
		float fTemp = (float)(sensor->cTempL + (sensor->cTempH << 8))/16;
		if (fTemp < 40 && fTemp > 5)
		{
			sensor->fTemp = fTemp;
		}
	}

	return ucReset;
}

uint8_t readTemperatureChar(DS18B20Sensor_t * sensor)
{
	uint8_t ucReset;
	int cLSB[8]	=	{8, 4, 2, 1, 500, 250, 125, 62};
	int cMSB[3]	=	{64, 32, 16};
	
	int iInteger = 0;
	int iFraction = 0;

	ucReset = uc1Wire_ResetPulse();

	v1Wire_SendByte(0x55);
	sendROM(sensor->cROM);
	v1Wire_SendByte(0xBE);
	sensor->cTempL = uv1Wire_ReadByte();
	sensor->cTempH = uv1Wire_ReadByte();
	ucReset = uc1Wire_ResetPulse();

	//czesc calkowita	
	if (sensor->cTempL & 1 << 7)
		iInteger+=	cLSB[0];

	if (sensor->cTempL & 1 << 6)
		iInteger+=	cLSB[1];

	if (sensor->cTempL & 1 << 5)
		iInteger+=	cLSB[2];

	if (sensor->cTempL & 1 << 4)
		iInteger+=	cLSB[3];

	if (sensor->cTempH & 1 << 2)
		iInteger+=	cMSB[0];

	if (sensor->cTempH & 1 << 1)
		iInteger+=	cMSB[1];	

	if (sensor->cTempH & 1 << 0)
		iInteger+=	cMSB[2];
	
	//czesc ulamkowa
	if (sensor->	cTempL & 1 << 3)
		iFraction+=	cLSB[4];	
	if (sensor->	cTempL & 1 << 2)
		iFraction+=	cLSB[5];	
	if (sensor->	cTempL & 1 << 1)
		iFraction+=	cLSB[6];
	if (sensor->	cTempL & 1 << 0)
		iFraction+=	cLSB[7];	
	
	int length = strlen(sensor->cAtempL);
	sensor->cAtempL[length] = '.';
	sensor->cAtempL[length + 1] = '\0';
	
	strcat (sensor->cAtempL, sensor->cAtempH);

	return ucReset;
}
