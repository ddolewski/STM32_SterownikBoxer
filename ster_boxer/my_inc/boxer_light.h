#ifndef MY_INC_BOXER_LIGHT_H_
#define MY_INC_BOXER_LIGHT_H_

#include "stdint.h"

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
}light_control_t;

typedef struct
{
	uint32_t counterSeconds;
	uint8_t counterHours;
}light_counters_t;

light_state_t lastLightState;
volatile light_control_t xLightControl;
volatile light_counters_t xLightCounters;

#endif /* MY_INC_BOXER_LIGHT_H_ */
