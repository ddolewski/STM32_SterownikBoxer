/*
 * boxer_irrigation.h
 *
 *  Created on: 30 lip 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_IRRIGATION_H_
#define MY_INC_BOXER_IRRIGATION_H_

#include "boxer_struct.h"
#include "system_gpio.h"

extern soil_moist_t lastSoilMoistState;
extern soil_moist_t soilMoisture;
extern irrigate_control_t irrigationControl;

void Irrigation_Core(void);
void Irrigation_Handler(void);

#endif /* MY_INC_BOXER_IRRIGATION_H_ */
