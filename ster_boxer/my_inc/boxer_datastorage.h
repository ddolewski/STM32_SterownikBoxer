#ifndef MY_INC_BOXER_DATASTORAGE_H_
#define MY_INC_BOXER_DATASTORAGE_H_

#include "system_flash.h"
#include "stdint.h"
#include "boxer_climate.h"
#include "boxer_ph.h"
#include "boxer_irrigation.h"
#include "boxer_light.h"

#define SIGNATURE_A 				((uint32_t)0x01234567)
#define SIGNATURE_B 				((uint32_t)0x89ABCDEF)

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

void FLASH_ReadConfiguration(void);
void FLASH_SaveConfiguration(void);
void FLASH_RestoreDefaultConfig(void);
void FLASH_STORAGE_TEST(void);
#endif /* MY_INC_BOXER_DATASTORAGE_H_ */
