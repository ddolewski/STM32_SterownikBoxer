#ifndef MY_INC_BOXER_IRRIGATION_H_
#define MY_INC_BOXER_IRRIGATION_H_

#include "system_gpio.h"
#include "systime.h"

typedef enum
{
	IRRIGATION_MODE_OFF,
	IRRIGATION_MODE_MANUAL,
	IRRIGATION_MODE_AUTO_SOIL,
	IRRIGATION_MODE_AUTO_TIME
}irrigation_mode_t;

typedef struct
{
	irrigation_mode_t mode;
	uint8_t frequency;
	uint16_t water;
}irrigate_control_t;

typedef enum
{
	SOIL_UNKNOWN_STATE,
	SOIL_DRY,
	SOIL_WET
}soil_moist_t;

extern soil_moist_t lastSoilMoistState;
extern soil_moist_t soilMoisture;
extern irrigate_control_t xIrrigationControl;

void Irrigation_Core(void);
void Irrigation_Handler(void);
void Irrigation_PumpControll(void);
void Irrigation_CheckSoilMoisture(void);

#endif /* MY_INC_BOXER_IRRIGATION_H_ */
