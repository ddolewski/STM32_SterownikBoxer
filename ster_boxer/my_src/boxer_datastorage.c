/*
 * boxer_datastorage.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_datastorage.h"
#include "boxer_light.h"
#include "boxer_climate.h"
#include <math.h>

static const flashSettings_t defFlashConfig =
{
	.signatureA = SIGNATURE_A,
	.backupTempControl =
	{
		.userTemp 		= 25,
		.fanPull 		= 40,
		.fanPush 		= 60,
		.tempCtrlMode 	= TEMP_AUTO,
	},

	.backupLightControl =
	{
		.lightingState 	= LIGHT_OFF,
		.timeOnHours 	= 12,
		.timeOffHours 	= 12,
		.counterSeconds = 0,
		.counterHours 	= 0
	},

	.backupIrrigationControl =
	{
		.mode 		= IRRIGATION_MODE_OFF,
		.frequency 	= 0,
		.water 		= 0
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
		xIrrigationControl 	= xBackupConfig.backupIrrigationControl;
		xLightControl 		= xBackupConfig.backupLightControl;
		xTempControl 		= xBackupConfig.backupTempControl;
		xFactorsEquationpH 	= xBackupConfig.backupPhEcuationFactors;

		if (isfinite(xFactorsEquationpH.soilFactor_A)  == 0  ||
			isfinite(xFactorsEquationpH.soilFactor_B)  == 0  ||
			isfinite(xFactorsEquationpH.waterFactor_A) == 0  ||
			isfinite(xFactorsEquationpH.waterFactor_B) == 0)
		{
			FLASH_RestoreDefaultConfig();
		}

		if (xLightControl.timeOnHours == 0 && xLightControl.timeOffHours == 24)
		{
			LAMP_TURNOFF();
			xLightControl.lightingState 	= LIGHT_OFF;
			xLightControl.counterSeconds 	= 0;
			xLightControl.counterHours 		= 0;
		}
		else if (xLightControl.timeOnHours == 24 && xLightControl.timeOffHours == 0)
		{
			LAMP_TURNON();
			xLightControl.lightingState 	= LIGHT_ON;
			xLightControl.counterSeconds 	= 0;
			xLightControl.counterHours 		= 0;
		}
	}
	else
	{
		xIrrigationControl 	= defFlashConfig.backupIrrigationControl;
		xLightControl 		= defFlashConfig.backupLightControl;
		xTempControl 		= defFlashConfig.backupTempControl;
		xFactorsEquationpH 	= defFlashConfig.backupPhEcuationFactors;

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

	xBackupConfig.backupIrrigationControl 	= xIrrigationControl;
	xBackupConfig.backupLightControl 		= xLightControl;
	xBackupConfig.backupTempControl 		= xTempControl;
	xBackupConfig.backupPhEcuationFactors 	= xFactorsEquationpH;

	flashError = SYSTEM_FLASH_WritePage((uint16_t*)&xBackupConfig, CONFIG_PAGE_NUMBER, sizeof(xBackupConfig));

	if (flashError == ERROR)
	{
		_printString("blad zapisu flash\r\n");
	}
}

void FLASH_RestoreDefaultConfig(void)
{
	xIrrigationControl 	= defFlashConfig.backupIrrigationControl;
	xLightControl 		= defFlashConfig.backupLightControl;
	xTempControl 		= defFlashConfig.backupTempControl;
	xFactorsEquationpH 	= defFlashConfig.backupPhEcuationFactors;

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

	xBackupConfig.backupTempControl.xTempControl = TEMP_AUTO;
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
