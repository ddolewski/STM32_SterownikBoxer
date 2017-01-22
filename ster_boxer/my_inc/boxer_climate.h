/*
 * boxer_climate.h
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_CLIMATE_H_
#define MY_INC_BOXER_CLIMATE_H_

#include "boxer_struct.h"
#include "stm32f0xx_tim.h"
#include "system_gpio.h"
#include "hardware\DS18B20\ds18b20.h"
#include "debug.h"
#include "boxer_timers.h"

void ClimateTempControl_Handler(DS18B20Sensor_t * ds18b20_1);

#endif /* MY_INC_BOXER_CLIMATE_H_ */