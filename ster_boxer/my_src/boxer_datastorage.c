/*
 * boxer_datastorage.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_datastorage.h"
#include "boxer_light.h"
#include "boxer_climate.h"

flashSettings_t	currentFlashConfig;				//ustawienia obecne

static const flashSettings_t defFlashConfig =
{
	.signatureA = SIGNATURE_A,
	.backupTempControl =
	{
		.userTemp = 25,
		.fanPull = 40,
		.fanPush = 60,
		.tempCtrlMode = TEMP_AUTO,
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

static const flashLampCounters_t defFlashLightCounters =
{
	.signatureC = SIGNATURE_C,
    .lightCounters = { .counterSeconds = 0, .counterHours = 0 },
	.lightingState = LIGHT_OFF,
	.signatureD = SIGNATURE_D
};
//flashCopy_t backupConfig __attribute__((section(".STORAGE_REGION"))) = {0};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FLASH_ReadConfiguration(void)
{
	flashSettings_t xBackupConfig;
	ErrorStatus flashError = SUCCESS;
	flashError = SYSTEM_FLASH_ReadPage((uint16_t*)&xBackupConfig, CONFIG_PAGE_NUMBER, sizeof(xBackupConfig));

	if (flashError == ERROR)
	{
		_printString("blad odczytu flash\r\n");
	}

	if ((xBackupConfig.signatureA == SIGNATURE_A) && (xBackupConfig.signatureB == SIGNATURE_B))
	{
		irrigationControl = xBackupConfig.backupIrrigationControl;
		xLightControl = xBackupConfig.backupLightControl;
		tempControl = xBackupConfig.backupTempControl;
		FactorsEquationpH = xBackupConfig.backupPhEcuationFactors;
	}
	else
	{
		irrigationControl = defFlashConfig.backupIrrigationControl;
		xLightControl = defFlashConfig.backupLightControl;
		tempControl = defFlashConfig.backupTempControl;
		FactorsEquationpH = defFlashConfig.backupPhEcuationFactors;

		flashError = SYSTEM_FLASH_ErasePage(CONFIG_PAGE_NUMBER);
		if (flashError == ERROR)
		{
			_printString("blad kasowania flash\r\n");
		}

		flashError = SYSTEM_FLASH_WritePage((uint16_t*)&defFlashConfig, CONFIG_PAGE_NUMBER, sizeof(defFlashConfig));

		if (flashError == ERROR)
		{
			_printString("blad zapisu flash\r\n");
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FLASH_SaveConfiguration(void)
{
	flashSettings_t xBackupConfig;
	ErrorStatus flashError = SYSTEM_FLASH_ErasePage(CONFIG_PAGE_NUMBER);

	if (flashError == ERROR)
	{
		_printString("blad kasowania flash\r\n");
	}

	xBackupConfig.signatureA = SIGNATURE_A;
	xBackupConfig.signatureB = SIGNATURE_B;

	xBackupConfig.backupIrrigationControl = irrigationControl;
	xBackupConfig.backupLightControl = xLightControl;
	xBackupConfig.backupTempControl = tempControl;
	xBackupConfig.backupPhEcuationFactors = FactorsEquationpH;

	flashError = SYSTEM_FLASH_WritePage((uint16_t*)&xBackupConfig, CONFIG_PAGE_NUMBER, sizeof(xBackupConfig));

	if (flashError == ERROR)
	{
		_printString("blad zapisu flash\r\n");
	}
}

void FLASH_RestoreDefaultConfig(void)
{
	irrigationControl = defFlashConfig.backupIrrigationControl;
	xLightControl = defFlashConfig.backupLightControl;
	tempControl = defFlashConfig.backupTempControl;
	FactorsEquationpH = defFlashConfig.backupPhEcuationFactors;

	ErrorStatus flashError = SYSTEM_FLASH_ErasePage(CONFIG_PAGE_NUMBER);
	if (flashError == ERROR)
	{
		_printString("blad kasowania flash\r\n");
	}

	flashError = SYSTEM_FLASH_WritePage((uint16_t*)&defFlashConfig, CONFIG_PAGE_NUMBER, sizeof(defFlashConfig));

	if (flashError == ERROR)
	{
		_printString("blad zapisu flash\r\n");
	}
}

void FLASH_ClearLightState(void)
{
	xLightCounters = defFlashLightCounters.lightCounters;
	ErrorStatus flashError = SYSTEM_FLASH_ErasePage(LIGHT_COUNTERS_PAGE_NUMBER);
	if (flashError == ERROR)
	{
		_printString("blad kasowania flash\r\n");
	}

	flashError = SYSTEM_FLASH_WritePage((uint16_t*)&defFlashLightCounters, LIGHT_COUNTERS_PAGE_NUMBER, sizeof(defFlashLightCounters));

	if (flashError == ERROR)
	{
		_printString("blad zapisu flash\r\n");
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FLASH_SaveLightCounters(void)
{
	flashLampCounters_t xBackupLightCounters;
	ErrorStatus flashError = SYSTEM_FLASH_ErasePage(LIGHT_COUNTERS_PAGE_NUMBER);

	if (flashError == ERROR)
	{
		_printString("blad kasowania flash\r\n");
	}

	xBackupLightCounters.signatureC = SIGNATURE_C;
	xBackupLightCounters.signatureD = SIGNATURE_D;

	xBackupLightCounters.lightCounters = xLightCounters;
	xBackupLightCounters.lightingState = xLightControl.lightingState;

	flashError = SYSTEM_FLASH_WritePage((uint16_t*)&xBackupLightCounters, LIGHT_COUNTERS_PAGE_NUMBER, sizeof(xBackupLightCounters));
	if (flashError == ERROR)
	{
		_printString("blad kasowania flash\r\n");
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FLASH_ReadLightCounters(void)
{
	ErrorStatus flashError = SUCCESS;
	flashLampCounters_t xBackupLightCounters;
	flashError = SYSTEM_FLASH_ReadPage((uint16_t*)&xBackupLightCounters, LIGHT_COUNTERS_PAGE_NUMBER, sizeof(xBackupLightCounters));

	if (flashError == ERROR)
	{
		_printString("blad odczytu flash\r\n");
	}

	if ((xBackupLightCounters.signatureC == SIGNATURE_C) && (xBackupLightCounters.signatureD == SIGNATURE_D))
	{
		xLightCounters = xBackupLightCounters.lightCounters;
		xLightControl.lightingState = xBackupLightCounters.lightingState;
	}
	else
	{
		xLightCounters = defFlashLightCounters.lightCounters;
		xLightControl.lightingState = defFlashLightCounters.lightingState;

		flashError = SYSTEM_FLASH_ErasePage(LIGHT_COUNTERS_PAGE_NUMBER);
		if (flashError == ERROR)
		{
			_printString("blad kasowania flash\r\n");
		}

		flashError = SYSTEM_FLASH_WritePage((uint16_t*)&defFlashLightCounters, LIGHT_COUNTERS_PAGE_NUMBER, sizeof(defFlashLightCounters));
		if (flashError == ERROR)
		{
			_printString("blad zapisu flash\r\n");
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FLASH_STORAGE_TEST(void)
{
#ifdef FLASH_SAVE_TEST
	flashSettings_t xBackupConfig;
	xBackupConfig.backupIrrigationControl.mode = IRRIGATION_MODE_OFF;
	xBackupConfig.backupIrrigationControl.frequency = 5;
	xBackupConfig.backupIrrigationControl.water = 250;

	xBackupConfig.backupLightControl.lightingState = LIGHT_ON;
	xBackupConfig.backupLightControl.hoursCounter = 2;
	xBackupConfig.backupLightControl.secondCounter = 34;
	xBackupConfig.backupLightControl.timeOffHours = 4;
	xBackupConfig.backupLightControl.timeOnHours = 20;

	xBackupConfig.backupTempControl.tempControl = TEMP_AUTO;
	xBackupConfig.backupTempControl.userTemp = 30;

	xBackupConfig.backupPhEcuationFactors.waterFactor_A = 6.34;
	xBackupConfig.backupPhEcuationFactors.waterFactor_B = -0.84;
	xBackupConfig.backupPhEcuationFactors.soilFactor_A = 3.56;
	xBackupConfig.backupPhEcuationFactors.soilFactor_B = -1.43;

	xBackupConfig.signatureA = SIGNATURE_A;
	xBackupConfig.signatureB = SIGNATURE_B;

    FLASH_SaveConfiguration();
	ErrorStatus flashError = SYSTEM_FLASH_ErasePage(CONFIG_PAGE_NUMBER);
	if (flashError == ERROR)
	{
		_printString("blad kasowania flash\r\n");
	}

	flashError = SYSTEM_FLASH_WritePage((uint16_t*)&xBackupConfig, PAGE_NUMBER, sizeof(xBackupConfig));
	if (flashError == ERROR)
	{
		_printString("blad zapisu flash\r\n");
	}

#endif

#ifdef FLASH_READ_TEST
	memset(&xBackupConfig, 0, sizeof(xBackupConfig));
    FLASH_ReadConfiguration();
#endif
}
