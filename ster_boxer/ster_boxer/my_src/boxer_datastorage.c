/*
 * boxer_datastorage.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_datastorage.h"

flashSettings_t	currentFlashConfig;				//ustawienia obecne

static const flashSettings_t defFlashConfig =
{
	.signatureA = SIGNATURE_A,
	.backupTempControl =
	{
		.userTemp = 25,
		.tempControl = TEMP_CONTROL_OFF,
	},

	.backupLightControl =
	{
		.lightingState = LIGHT_OFF,
		.timeOnHours = 12,
		.timeOffHours = 12,
	},

	.backupIrrigationControl =
	{
		.mode = IRRIGATION_MODE_OFF,
		.frequency = 0,
		.water = 0
	},

	.backupPhEcuationFactors =
	{
		.waterFactor_A = 0,
		.waterFactor_B = 0,
		.soilFactor_A  = 0,
		.soilFactor_B  = 0,
	},

	.signatureB = SIGNATURE_B
};
//flashCopy_t backupConfig __attribute__((section(".STORAGE_REGION"))) = {0};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ReadConfiguration(void)
{
	flashSettings_t xBackupConfig;
	SYSTEM_FLASH_ReadPage((uint16_t*)&xBackupConfig, PAGE_NUMBER, sizeof(xBackupConfig));

	if ((xBackupConfig.signatureA == SIGNATURE_A) && (xBackupConfig.signatureB == SIGNATURE_B))
	{
		irrigationControl = xBackupConfig.backupIrrigationControl;
		lightControl = xBackupConfig.backupLightControl;
		tempControl = xBackupConfig.backupTempControl;
		FactorsEquationpH = xBackupConfig.backupPhEcuationFactors;
	}
	else
	{
		irrigationControl = defFlashConfig.backupIrrigationControl;
		lightControl = defFlashConfig.backupLightControl;
		tempControl = defFlashConfig.backupTempControl;
		FactorsEquationpH = defFlashConfig.backupPhEcuationFactors;

		ErrorStatus flashError = SYSTEM_FLASH_WritePage((uint16_t*)&defFlashConfig, PAGE_NUMBER, sizeof(defFlashConfig));

		if (flashError == ERROR)
		{
			//TODO informacja o bledzie flash
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SaveConfiguration(void)
{
	flashSettings_t xBackupConfig;
	ErrorStatus flashError = SYSTEM_FLASH_ErasePage(PAGE_NUMBER);

	if (flashError == ERROR)
	{
		//todo blad kasowania pamieci flash
	}

	xBackupConfig.signatureA = SIGNATURE_A;
	xBackupConfig.signatureB = SIGNATURE_B;

	xBackupConfig.backupIrrigationControl = irrigationControl;
	xBackupConfig.backupLightControl = lightControl;
	xBackupConfig.backupTempControl = tempControl;
	xBackupConfig.backupPhEcuationFactors = FactorsEquationpH;

	flashError = SYSTEM_FLASH_WritePage((uint16_t*)&xBackupConfig, PAGE_NUMBER, sizeof(xBackupConfig));

	if (flashError == ERROR)
	{
		//todo blad zapisu pamieci flash
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FLASH_STORAGE_TEST(void)
{
	flashSettings_t xBackupConfig;
#ifdef FLASH_SAVE_TEST
	xBackupConfig.backupIrrigationControl.mode = IRRIGATION_MODE_OFF;
	xBackupConfig.backupIrrigationControl.frequency = 5;
	xBackupConfig.backupIrrigationControl.water = 250;

	xBackupConfig.backupLightControl.lightingState = LIGHT_ON;
	xBackupConfig.backupLightControl.hoursCounter = 2;
	xBackupConfig.backupLightControl.secondCounter = 34;
	xBackupConfig.backupLightControl.timeOffHours = 4;
	xBackupConfig.backupLightControl.timeOnHours = 20;

	xBackupConfig.backupTempControl.tempControl = TEMP_CONTROL_OFF;
	xBackupConfig.backupTempControl.userTemp = 30;

	xBackupConfig.backupPhEcuationFactors.waterFactor_A = 6.34;
	xBackupConfig.backupPhEcuationFactors.waterFactor_B = -0.84;
	xBackupConfig.backupPhEcuationFactors.soilFactor_A = 3.56;
	xBackupConfig.backupPhEcuationFactors.soilFactor_B = -1.43;

	xBackupConfig.signatureA = SIGNATURE_A;
	xBackupConfig.signatureB = SIGNATURE_B;

    SaveConfiguration();
	ErrorStatus flashError = SYSTEM_FLASH_ErasePage(PAGE_NUMBER);
	if (flashError == ERROR)
	{
		//todo blad kasowania pamieci flash
	}

	flashError = SYSTEM_FLASH_WritePage((uint16_t*)&xBackupConfig, PAGE_NUMBER, sizeof(xBackupConfig));
	if (flashError == ERROR)
	{
		//todo blad odczytu pamieci flash
	}

#endif

#ifdef FLASH_READ_TEST
	memset(&xBackupConfig, 0, sizeof(xBackupConfig));
    ReadConfiguration();
#endif
}
