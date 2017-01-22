#ifndef SYSTEM_FLASH_H_
#define SYSTEM_FLASH_H_

#include "stm32f0xx.h"

#define KEY1 				(uint32_t)0x45670123	// 1 klucz odblokowania dostepu od flash i rejestrow sterujacych
#define KEY2				(uint32_t)0xCDEF89AB	// 2 klucz odblokowania dostepu od flash i rejestrow sterujacych
#define FLASH_START_PAGE	(uint32_t)0x08000000	// adres poczatkowy FLASH (strony 0, komorki 0)
#define LOWER_ADDRESS		(uint32_t)0x0800F000 	// od strony 60
#define UPPER_ADDRESS		(uint32_t)0x0800FFFF	// do strony 63 (ostatnia strona FLASH)
#define PAGE_SIZE			(uint16_t)1024			// 1024 czyli 1kB na strone

#define CONFIG_PAGE_NUMBER 			62
#define LIGHT_COUNTERS_PAGE_NUMBER	63

typedef enum
{
	UNLOCK = 0,
	LOCK = !UNLOCK
}FlashAccessStatus_t;

uint8_t* CurrentCellAddress;
uint16_t* write_address;

void SYSTEM_FLASH_SetAccess(FlashAccessStatus_t status);
FlashAccessStatus_t SYSTEM_FLASH_GetAccess(void);
ErrorStatus SYSTEM_FLASH_ErasePage(uint8_t PageNumber);
ErrorStatus SYSTEM_FLASH_ReadPage(uint16_t * buffer, uint8_t PageNumber, uint16_t CellTo);
ErrorStatus SYSTEM_FLASH_WritePage(uint16_t * buffer, uint8_t PageNumber, uint16_t DataSize);

#endif /* SYSTEM_FLASH_H_ */
