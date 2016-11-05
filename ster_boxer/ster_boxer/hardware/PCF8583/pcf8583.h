#ifndef __pcf8583
#define __pcf8583

#include "stm32f0xx.h"
#include "stm32f0xx_i2c.h"
#include "boxer_struct.h"

#define PCF8583_ADDR          	((uint16_t)(0xA0))
#define PCF8583_SEC_REG  		((uint8_t)(0x02))
#define PCF8583_MIN_REG  		((uint8_t)(0x03))
#define PCF8583_HOU_REG  		((uint8_t)(0x04))
#define PCF8583_DAY_REG  		((uint8_t)(0x05))
#define PCF8583_MON_REG  		((uint8_t)(0x06))

uint8_t secondPCF8583, minutePCF8583, hourPCF8583, dayPCF8583, monthPCF8583;
uint8_t readDataFlag;

void I2C1_Init(void);
void I2C2_Init(void);
uint8_t PCF8583_RegRead(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, ErrorStatus * Error);
ErrorStatus PCF8583_RegWrite(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, uint8_t data);
int BcdToDec(int data);
int DecToBcd(int data);
void PCF8583_SetDate(I2C_TypeDef* I2Cx, date_t * rtcDate);
void PCF8583_ReadDate(I2C_TypeDef* I2Cx, date_t * rtcDate);
#endif
