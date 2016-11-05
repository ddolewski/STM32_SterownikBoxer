#ifndef __sht2x
#define __sht2x

#include "stm32f0xx.h"
#include "stm32f0xx_i2c.h"

#define SHT21_ADDR 				((uint16_t)(0x80))

union SHT_TempData{
	unsigned char msb_lsb[2];
	unsigned short data;
}SHT_TempData;

union SHT_HumData{
	unsigned char msb_lsb[2];
	unsigned short data;
}SHT_HumData;

// sensor command
typedef enum{
	TRIG_T_MEASUREMENT_HM = 0xE3, 		// command trig. temp meas. hold master
	TRIG_RH_MEASUREMENT_HM = 0xE5, 		// command trig. humidity meas. hold master
	TRIG_T_MEASUREMENT_POLL = 0xF3, 	// command trig. temp meas. no hold master
	TRIG_RH_MEASUREMENT_POLL = 0xF5, 	// command trig.  humidity meas. no hold master
	USER_REG_W = 0xE6, 					// command writing user register
	USER_REG_R = 0xE7, 					// command reading user register
	SOFT_RESET = 0xFE 					// command soft reset
}etSHT2xCommand;

typedef enum {
	SHT2x_RES_12_14BIT = 0x00, 	// RH=12bit, T=14bit
	SHT2x_RES_8_12BIT = 0x01, 	// RH= 8bit, T=12bit
	SHT2x_RES_10_13BIT = 0x80, 	// RH=10bit, T=13bit
	SHT2x_RES_11_11BIT = 0x81, 	// RH=11bit, T=11bit
	SHT2x_RES_MASK = 0x81 		// Mask for res. bits (7,0) in user reg.
} etSHT2xResolution;

typedef enum {
	SHT2x_EOB_ON = 0x40, 	// end of battery
	SHT2x_EOB_MASK = 0x40, 	// Mask for EOB bit(6) in user reg.
} etSHT2xEob;

typedef enum {
	SHT2x_HEATER_ON  = 0x04, 	// heater on
	SHT2x_HEATER_OFF = 0x00, 	// heater off
	SHT2x_HEATER_MASK = 0x04, 	// Mask for Heater bit(2) in user reg.
} etSHT2xHeater;

// measurement signal selection
typedef enum{
	HUMIDITY,
	TEMP
}etSHT2xMeasureType;

ErrorStatus SHT21_SoftReset(I2C_TypeDef* I2Cx, uint16_t SlaveAddr);
uint8_t SHT21_ReadUserReg(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, ErrorStatus * Error);
uint16_t SHT21_MeasureTempCommand(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, ErrorStatus * Error);
uint16_t SHT21_MeasureHumCommand(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, ErrorStatus * Error);
float SHT21_CalcTemp(uint16_t dataTemp);
float SHT21_CalcRH(uint16_t humidity);

#endif
