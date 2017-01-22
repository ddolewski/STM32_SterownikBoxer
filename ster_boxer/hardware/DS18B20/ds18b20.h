#ifndef MY_INC_DS18B20_H_
#define MY_INC_DS18B20_H_

#include "stm32f0xx_gpio.h"

#include <stdint.h>
#include "string_builder.h"
#include "boxer_struct.h"

#define DS18B20_PORT			GPIOF
#define DS18B20_PIN				GPIOx_Pin_5

extern const uint8_t sensor1ROM[8];
extern const uint8_t sensor2ROM[8];


// global search state
unsigned char ROM_NO[8];
int LastDiscrepancy;
int LastFamilyDiscrepancy;
int LastDeviceFlag;
unsigned char crc8;

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
void readTemperature(DS18B20Sensor_t * sensor);
void readTemperatureChar(DS18B20Sensor_t * sensor);
void readTemperature_SkipRom(DS18B20Sensor_t * sensor);
uint8_t CRC8(uint8_t *inData, uint8_t len);

// method declarations
int  OWFirst();
int  OWNext();
int  OWVerify();
void OWTargetSetup(unsigned char family_code);
void OWFamilySkipSetup();
int  OWSearch();
unsigned char docrc8(unsigned char value);

#endif

