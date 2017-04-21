#ifndef MY_INC_BOXER_CLIMATE_H_
#define MY_INC_BOXER_CLIMATE_H_

#include "stm32f0xx_tim.h"
#include "system_gpio.h"
#include "ds18b20.h"
#include "debug.h"
#include "boxer_timers.h"
#include "systime.h"
#include "hardware/DS18B20/ds18b20.h"
#include "hardware/SHT2x/sht2x.h"

typedef enum
{
	TEMP_AUTO = 'A',
	TEMP_MANUAL = 'M'
}temp_control_state_t;

typedef enum
{
	TEMP_MIN = 20,
	TEMP_MAX = 33,
}temp_range_t;

typedef struct
{
	uint8_t userTemp;
	uint8_t fanPull;
	uint8_t fanPush;
	temp_control_state_t tempCtrlMode;
}temp_control_t;

volatile temp_control_t tempControl;

void Climate_TempCtrl_Handler(void);
void Climate_SensorsHandler(void);

#endif /* MY_INC_BOXER_CLIMATE_H_ */
