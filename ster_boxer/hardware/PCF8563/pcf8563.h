/*
 * pcf8583.h
 *
 * Created: 2016-05-04 19:00:08
 *  Author: Doles
 */ 


#ifndef PCF8563_H_
#define PCF8563_H_

#include <timestamp.h>
#include "stm32f0xx.h"
#include "stm32f0xx_i2c.h"
#include "boxer_struct.h"
#include "global.h"

#define ACK				1
#define NACK			0

//#define SCL_I2C			0
//#define SDA_I2C			1
//#define SCL_SDA_High	(PORTC |= (1 << SCL_I2C) | (1 << SDA_I2C))

//#define PCF8563AddressWrite		162
//#define PCF8563AddressRead		163
#define PCF8563_ADDR 			0xA2

#define PCF8563_CLKOUT_CTRL_REG		((uint8_t)0x0D))

#define PCF8563_CTRL_STAT_STOP_BIT	((uint8_t)(0x05))

#define PCF8563_CTRL_STAT_REG1		((uint8_t)(0x00))
#define PCF8563_CTRL_STAT_REG2		((uint8_t)(0x01))
#define PCF8563_SEC_REG  			((uint8_t)(0x02))
#define PCF8563_MIN_REG  			((uint8_t)(0x03))
#define PCF8563_HOU_REG  			((uint8_t)(0x04))
#define PCF8563_MDAY_REG  			((uint8_t)(0x05))
#define PCF8563_WDAY_REG  			((uint8_t)(0x06))
#define PCF8563_MON_REG				((uint8_t)(0x07))
#define PCF8563_YEAR_REG			((uint8_t)(0x08))

#define TIME_MONTH_MAR		3
#define TIME_MONTH_OCT		10
#define TIME_DAYS_OF_WEEK	7
#define TIME_NR_OF_MONTHS	12

//typedef struct
//{
//	uint8_t second;
//	uint8_t minute;
//	uint8_t hour;
//}time_t;

typedef struct
{
	uint8_t min;
	uint8_t hour;
}time_short_t;

//typedef struct
//{
//	uint8_t weekDay;
//	uint8_t monthDay;
//	uint8_t month;
//	uint16_t year;
//}date_t;

char secBuff[10];
char minBuff[10];
char houBuff[10];

uint8_t readDataFlag;
//void TWI_Init(uint8_t TWBRValue);
//uint8_t TWI_Start(void);
//uint8_t TWI_Stop(void);
//uint8_t TWI_Write(uint8_t data);
//uint8_t TWI_Read(uint8_t ack, uint8_t * err);
//
//uint8_t miscBcd2bin(uint8_t byte_to_conv);
//uint8_t miscBin2bcd(uint8_t byte_to_conv);

ErrorStatus PCF8563_ReadTime(time_complex_t * xTime, I2C_TypeDef* I2Cx);
ErrorStatus PCF8563_WriteTime(time_complex_t * xTime, I2C_TypeDef * I2Cx);
ErrorStatus PCF8563_Init(I2C_TypeDef * I2Cx);



//#define PCF8583_ADDR          	((uint16_t)(0xA0))
//#define PCF8583_SEC_REG  		((uint8_t)(0x02))
//#define PCF8583_MIN_REG  		((uint8_t)(0x03))
//#define PCF8583_HOU_REG  		((uint8_t)(0x04))
//#define PCF8583_DAY_REG  		((uint8_t)(0x05))
//#define PCF8583_MON_REG  		((uint8_t)(0x06))

//uint8_t secondPCF8583, minutePCF8583, hourPCF8583, dayPCF8583, monthPCF8583;
//uint8_t readDataFlag;

void I2C1_Init(void);
void I2C2_Init(void);
uint8_t PCF8563_RegRead(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, ErrorStatus * Error);
ErrorStatus PCF8563_RegWrite(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, uint8_t data);
int BcdToDec(int data);
int DecToBcd(int data);
//void PCF8563_SetDate(I2C_TypeDef* I2Cx, date_t * rtcDate);
//void PCF8563_ReadDate(I2C_TypeDef* I2Cx, date_t * rtcDate);
#endif /* PCF8563_H_ */
