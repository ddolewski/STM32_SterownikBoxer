/*
 * boxer_datastorage.h
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_DATASTORAGE_H_
#define MY_INC_BOXER_DATASTORAGE_H_

#include "system_flash.h"
#include "stdint.h"
#include "boxer_struct.h"

#define SIGNATURE_A 				0x01234567
#define SIGNATURE_B 				0x89ABCDEF


typedef struct
{
	uint32_t signatureA;

	temp_control_t backupTempControl;
	light_control_t backupLightControl;
	irrigate_control_t backupIrrigationControl;
	ph_factors_t backupPhEcuationFactors;

	uint32_t signatureB;
}flashSettings_t;

extern flashSettings_t currentFlashConfig;
//extern flashCopy_t backupConfig;// __attribute__((section(".STORAGE_REGION")));

void ReadConfiguration(void);
void SaveConfiguration(void);
void FLASH_STORAGE_TEST(void);
#endif /* MY_INC_BOXER_DATASTORAGE_H_ */
