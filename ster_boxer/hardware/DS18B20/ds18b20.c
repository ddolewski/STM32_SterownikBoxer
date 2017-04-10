#include "ds18b20.h"

const uint8_t sensor1ROM[8]={40, 255, 8, 153, 100, 20, 3, 10};
const uint8_t sensor2ROM[8]={40, 21, 46, 135, 4, 0, 0, 53};


GPIO_InitTypeDef GPIO_InitStructure;
static inline void ASM_DelayUS(uint32_t us);

static inline void ASM_DelayUS(uint32_t us)
{
	/* So (2^32)/12 micros max, or less than 6 minutes */
	us *= 10;
	us -= 2; //offset seems around 2 cycles
	/* fudge for function call overhead */
	us--;
	__ASM volatile(" mov r0, %[us] \n\t"
	".syntax unified \n\t"
	"1: subs r0, #1 \n\t"
	".syntax divided \n\t"
	" bhi 1b \n\t"
	:
	: [us] "r" (us)
	: "r0");
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
	ASM_DelayUS(600); //minimum 480us
	OneWire_In();
	ASM_DelayUS(50); //15-60us

	if (GPIOx_ReadInputPin(DS18B20_PORT, DS18B20_PIN))
	{
		ucPresenceImpulse = 1;
	}
	else
	{
		ucPresenceImpulse = 0;
	}

	 ASM_DelayUS(300); //60-240us

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

	ASM_DelayUS(1);

	if (cBit == 1)
	{
		OneWire_High();
	}

	ASM_DelayUS(50);
	OneWire_High();
}

//Funkcja odbiera bit z magistrali
uint8_t uc1Wire_ReadBit(void)
{
	uint8_t ucBit;

	GPIOx_PinConfig(DS18B20_PORT, Mode_Out, 0, OType_PP, OState_PD, DS18B20_PIN);
	DS18B20_PORT->BRR = DS18B20_PIN;
	ASM_DelayUS(1);

	GPIOx_PinConfig(DS18B20_PORT, Mode_In, 0, 0, OState_PD, DS18B20_PIN);
	ASM_DelayUS(10);

	if (GPIOx_ReadInputPin(DS18B20_PORT, DS18B20_PIN))
	{
		ucBit = 1;
	}
	else
	{
		ucBit = 0;
	}

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

	ASM_DelayUS(100);
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
		}

		ASM_DelayUS(50);
	}

	return ucReadByte;
}

//funkcja odczytuje unikalny kod pojedynczego czujnika
void readROM(uint8_t *buffer)
{
	uint8_t ucReset;
	uint32_t i;
	ucReset = uc1Wire_ResetPulse();
	v1Wire_SendByte(0x33);

	for(i = 0; i < 8; i++)
	{
		buffer[i] = uv1Wire_ReadByte();
	}
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
	uint8_t crcData[8] = {0};
	ucReset = uc1Wire_ResetPulse();

	v1Wire_SendByte(0x55);
	sendROM(sensor->cROM);
	v1Wire_SendByte(0xBE);
	sensor->cTempL = uv1Wire_ReadByte();
	sensor->cTempH = uv1Wire_ReadByte();
	crcData[0] = sensor->cTempL;
	crcData[1] = sensor->cTempH;

	crcData[2] =  uv1Wire_ReadByte();
	crcData[3] =  uv1Wire_ReadByte();
	crcData[4] =  uv1Wire_ReadByte();
	crcData[5] =  uv1Wire_ReadByte();
	crcData[6] =  uv1Wire_ReadByte();
	crcData[7] =  uv1Wire_ReadByte();
	uint8_t crcDS18B20 = uv1Wire_ReadByte();
	ucReset = uc1Wire_ResetPulse();

	uint8_t crcCalculated = CRC8(crcData, 8);

	if (crcDS18B20 == crcCalculated)
	{
		sensor->fTemp = (float)(sensor->cTempL + (sensor->cTempH << 8))/16;
	}

	return ucReset;
}

void readTemperatureChar(DS18B20Sensor_t * sensor)
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
}
