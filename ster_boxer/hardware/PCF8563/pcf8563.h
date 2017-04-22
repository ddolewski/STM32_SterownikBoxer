#ifndef PCF8563_H_
#define PCF8563_H_

#include <timestamp.h>
#include "stm32f0xx.h"
#include "stm32f0xx_i2c.h"
#include "systime.h"
#include "global.h"

#define PCF8563_ADDR 				(uint8_t)0xA2

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

#define TIME_MONTH_MAR				(uint8_t)3
#define TIME_MONTH_OCT				(uint8_t)10
#define TIME_DAYS_OF_WEEK			(uint8_t)7
#define TIME_NR_OF_MONTHS			(uint8_t)12

typedef struct
{
	uint8_t min;
	uint8_t hour;
}time_short_t;

ErrorStatus PCF8563_Init(I2C_TypeDef * I2Cx);
ErrorStatus PCF8563_ReadTime(time_complex_t * xTime, I2C_TypeDef* I2Cx);
ErrorStatus PCF8563_WriteTime(time_complex_t * xTime, I2C_TypeDef * I2Cx);
void RTC_Handler(void);


#endif /* PCF8563_H_ */
