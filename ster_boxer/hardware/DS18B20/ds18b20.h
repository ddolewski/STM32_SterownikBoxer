#ifndef MY_INC_DS18B20_H_
#define MY_INC_DS18B20_H_

#include "stm32f0xx_gpio.h"
#include "system_gpio.h"
#include <stdint.h>
#include "string_builder.h"

#define DS18B20_PORT			GPIOF
#define DS18B20_PIN				GPIOx_Pin_5

typedef struct
{
	uint8_t cTempH;
	uint8_t cTempL;
	uint8_t cROM[8];
	uint8_t cAtempH[5];
	uint8_t cAtempL[5];
	uint8_t cAtemp[10];
	float fTemp;
}DS18B20Sensor_t;

DS18B20Sensor_t sensorTempUp;
DS18B20Sensor_t sensorTempDown;

extern const uint8_t sensor1ROM[8];
extern const uint8_t sensor2ROM[8];

void OneWire_Low(void);
void OneWire_High(void);
void OneWire_In(void);
void OneWire_Out(void);

uint8_t uc1Wire_ResetPulse(void);
void v1Wire_SendBit(uint8_t cBit);
uint8_t uc1Wire_ReadBit(void);
void v1Wire_SendByte(uint8_t ucByteValue);
uint8_t uv1Wire_ReadByte(void);
void readROM(uint8_t * buffer);
void sendROM(uint8_t * table);
uint8_t initializeConversion(DS18B20Sensor_t * sensor);
uint8_t readTemperature(DS18B20Sensor_t * sensor);
void readTemperatureChar(DS18B20Sensor_t * sensor);
void readTemperature_SkipRom(DS18B20Sensor_t * sensor);
uint8_t CRC8(uint8_t *inData, uint8_t len);

#endif

