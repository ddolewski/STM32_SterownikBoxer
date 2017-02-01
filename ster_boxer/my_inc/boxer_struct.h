/*
 * boxer_struct.h
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_STRUCT_H_
#define MY_INC_BOXER_STRUCT_H_

#include "stdint.h"

typedef struct adcRef_t
{
	float vRefValueADC;
	float refVoltage;
	float mVFactor;
}adcRef_t;

typedef enum
{
	SOIL_UNKNOWN_STATE,
	SOIL_DRY,
	SOIL_WET
}soil_moist_t;

typedef struct ADC_value_t
{
	uint16_t soil;
	uint16_t water;
	soil_moist_t soilMoisture;
	uint16_t waterLevel;
} ADC_value_t;

typedef struct probe_t
{
	float tempSoil;
	float inAverageSoil;
	float inSoil;
	float tempWater;
	float inAverageWater;
	float inWater;
}probe_adc_t;

typedef struct pH_t
{
	float soil;
	float water;
}pH_t;

typedef struct pHBufferVoltage_t
{
	float pH4;
	float pH7;
	float pH9;
}pHBufferVoltage_t;

typedef enum
{
	PROBE_SOIL = 1,
	PROBE_WATER
}probe_type_t;

typedef enum
{
	BUFFER_PH4 = 1,
	BUFFER_PH7 = 2,
	BUFFER_PH9 = 3
}buffer_type_t;

typedef struct calibrationProcess_t
{
	uint8_t processActive;
	uint8_t calibrateDone;
	uint8_t turnOnBuzzer;
	uint8_t buzzerCounter;
	uint8_t toggleBuzzerState;
	uint8_t measureVoltagePh;
	uint8_t pHBufferChooser;
	uint8_t pH4Buffer;
	uint8_t pH7Buffer;
	uint8_t	pH9Buffer;
	uint8_t pHCounter;
	probe_type_t probeType;
}calibrationProcess_t;

typedef union
{
	uint8_t u8ValueArray[4];
	uint32_t u32Value;
}uint32_uint8_converter_t;

//typedef union FactorAWater_t
//{
//	uint8_t factorAbyte[4];
//	float ValueA;
//}FactorAWater_t;
//
//typedef union FactorBWater_t
//{
//	uint8_t factorBbyte[4];
//	float ValueB;
//}FactorBWater_t;
//
//typedef union FactorASoil_t
//{
//	uint8_t factorAbyte[4];
//	float ValueA;
//}FactorASoil_t;
//
//typedef union FactorBSoil_t
//{
//	uint8_t factorBbyte[4];
//	float ValueB;
//}FactorBSoil_t;

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

typedef enum
{
	PWM_MIN_PERCENT = 20,
	PWM_MAX_PERCENT = 95
}pwm_range_t;

typedef struct userParameters_t
{
	uint8_t userTemp;
	uint8_t fanPull;
	uint8_t fanPush;
	temp_control_state_t tempCtrlMode;
}temp_control_t;

typedef struct
{
	float waterFactor_A;
	float waterFactor_B;
	float soilFactor_A;
	float soilFactor_B;
}ph_factors_t;

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
	uint8_t water;
}irrigate_control_t;

//////////////////////////////////////////////////////////////////////////
typedef struct
{
	uint8_t saveParameters;
	uint8_t udpSendMsg;
	uint8_t udpReceiveByte;
	uint8_t bufferSizeOverflow;
	uint8_t initSensors;
	uint8_t increaseSpeedFlag;
}flag_t;

typedef struct
{
	uint8_t cTempH;
	uint8_t cTempL;
	uint8_t cROM[8];
	uint8_t cAtempH[5];
	uint8_t cAtempL[5];
	uint8_t cAtemp[10];
	float fTemp;
}DS18B20Sensor_t;

typedef struct
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t errorInfo;
}date_t;

typedef struct
{
	uint16_t 	year;
	uint8_t 	month;
	uint8_t 	mday;
	uint8_t 	wday;
	uint8_t 	hour;
	uint8_t 	min;
	uint8_t 	sec;
}time_complex_t;

typedef struct
{
	uint8_t 	hour;
	uint8_t 	min;
}time_simple_t;

typedef struct
{
	float tempSHT2x;
	float tempDS18B20_1_t;
	float tempDS18B20_2_t;
	uint32_t lux;
	float humiditySHT2x;
	char time[20];
	float ph1;
	float ph2;
	uint8_t page;
	uint8_t pageCounter;
}lcdDisplayData_t;


typedef enum
{
	FALSE = 0,
	TRUE = (!FALSE)
} bool_t;

volatile lcdDisplayData_t displayData;
volatile flag_t flagsGlobal;
volatile light_control_t xLightControl;
volatile light_counters_t xLightCounters;
volatile temp_control_t tempControl;

#endif /* MY_INC_BOXER_STRUCT_H_ */
