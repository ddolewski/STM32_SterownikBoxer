#ifndef MY_INC_BOXER_LIGHT_H_
#define MY_INC_BOXER_LIGHT_H_

#include "stdint.h"
#include "system_gpio.h"

#define LAMP_TURNON()	GPIOx_SetPin(LAMP_PORT, LAMP_PIN);
#define LAMP_TURNOFF()	GPIOx_ResetPin(LAMP_PORT, LAMP_PIN);

typedef enum
{
	LIGHT_OFF = 'R',
	LIGHT_ON = 'S'
}light_state_t;

typedef struct
{
	light_state_t lightingState;
	uint8_t timeOnHours;
	uint8_t timeOffHours;
	uint32_t counterSeconds;
	uint8_t counterHours;
}light_control_t;

light_state_t lastLightState;
volatile light_control_t xLightControl;

void Lightning_Handler(void);

#endif /* MY_INC_BOXER_LIGHT_H_ */
