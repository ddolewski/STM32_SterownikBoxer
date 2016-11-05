/*******************************************************************************
Biblioteka obslugi pamieci FLASH
mikrokontroler: STM32F051, 48MHz
data utworzenia: 2015-05-08
data modyfikacji: 2015-05-14
autor: Damian Dolewski

********************************************************************************/

#include "system_flash.h"
//-----------------------------------------------------------------------------
// Funkcja ustawiajaca dostep do pamieci wbudowanej flash
// status/in: zmienna enum, argument LOCK lub UNLOCK
//-----------------------------------------------------------------------------
void SYSTEM_FLASH_SetAccess(FlashAccessStatus_t status)
{
	if (status == LOCK)
	{
		FLASH->CR |= FLASH_CR_LOCK;
	}
	else
	{
		FLASH->KEYR = KEY1;
		FLASH->KEYR = KEY2;
	}
}
//-----------------------------------------------------------------------------
// Funkcja zwracajaca aktualny status dostepu do pamieci wbudowanej flash
// return: zmienna enum, argument LOCK lub UNLOCK
//-----------------------------------------------------------------------------
FlashAccessStatus_t SYSTEM_FLASH_GetAccess(void)
{
	if (!(FLASH->CR & FLASH_CR_LOCK))
	{
		return UNLOCK;
	}
	else
	{
		return LOCK;
	}
}
//-----------------------------------------------------------------------------
// Funkcja kasujaca wybrana strone pamieci flash w zakresie od 100 do 127
// definiowalne w pliku naglowkowym system_flash.h
// PageNumber/in: zmienna okreslajaca numer strony (bajt)
// return: zmienna statusu enum, argument ERROR lub SUCCESS
//-----------------------------------------------------------------------------
ErrorStatus SYSTEM_FLASH_ErasePage(uint8_t PageNumber)
{
	__disable_irq();
	uint16_t test_data;
	uint32_t PageAddress = 0;
	uint32_t TimeOut = 10000000;
	FlashAccessStatus_t access = SYSTEM_FLASH_GetAccess();
	if (access == LOCK)
	{
		SYSTEM_FLASH_SetAccess(UNLOCK);
		access = SYSTEM_FLASH_GetAccess();
	}

	if (access == UNLOCK)
	{
		// przeliczenie numeru strony (od np. 100 do 127) na wartosc adresu
		PageAddress = ((uint32_t)(FLASH_START_PAGE + PageNumber * PAGE_SIZE));
		// powrotne zdekodowanie aby upewnic sie ze adres jest prawidlowy
		PageNumber = (PageAddress - FLASH_START_PAGE) / PAGE_SIZE;

		if (PageAddress > UPPER_ADDRESS || PageAddress < LOWER_ADDRESS)
		{
			SYSTEM_FLASH_SetAccess(LOCK);
			return ERROR;
		}

		while ((FLASH->SR & FLASH_SR_BSY)) // sprawdzam czy flash jest zajety
		{
			if (TimeOut > 0)
			{
				TimeOut--;
			}
			else
			{
				SYSTEM_FLASH_SetAccess(LOCK);
				return ERROR;
			}
		}

		FLASH->CR |= FLASH_CR_PER; 		// ustawiam flage page erase
		FLASH->AR = ((uint32_t)(FLASH_START_PAGE + PageNumber * PAGE_SIZE)); 	// adres strony docelowej do kasowania
		FLASH->CR |= FLASH_CR_STRT; 	// ustawiam flage start

		TimeOut = 10000000;
		while ((FLASH->SR & FLASH_SR_BSY)) // sprawdzam czy flash jest zajety
		{
			if (TimeOut > 0)
			{
				TimeOut--;
			}
			else
			{
				FLASH->SR |= FLASH_SR_EOP;
				FLASH->CR &=~ FLASH_CR_PER;
				SYSTEM_FLASH_SetAccess(LOCK);
				return ERROR;
			}
		}

		// sprawdzam po kasowaniu strony czy wartosc po skasowaniu jest prawidlowa ---- 0xFF !!
		uint16_t i;
		for (i = 0; i < PAGE_SIZE; i++)
		{
			TimeOut = 10000000;
			while ((FLASH->SR & FLASH_SR_BSY)) // sprawdzam czy flash jest zajety
			{
				if (TimeOut > 0)
				{
					TimeOut--;
				}
				else
				{
					FLASH->SR |= FLASH_SR_EOP;
					FLASH->CR &=~ FLASH_CR_PER;
					SYSTEM_FLASH_SetAccess(LOCK);
					return ERROR;
				}
			}

			CurrentCellAddress = (uint8_t*)(FLASH_START_PAGE + PageNumber * PAGE_SIZE + i);
			test_data = *CurrentCellAddress; // odczytana wartosc po kasowaniu
			if ((uint8_t)test_data != 0xFF)
			{
				FLASH->SR |= FLASH_SR_EOP;
				FLASH->CR &=~ FLASH_CR_PER;
				SYSTEM_FLASH_SetAccess(LOCK);
				return ERROR;
			}

			FLASH->SR |= FLASH_SR_EOP; // kasuje flage end of operation gdy operacja przebiegla prawidlowo
		}
	}

	FLASH->CR &=~ FLASH_CR_PER; // kasuje flage erase bo nie bede mogl robic innych operacji np. write
	SYSTEM_FLASH_SetAccess(LOCK);
	__enable_irq();
	return SUCCESS;
}
//-----------------------------------------------------------------------------
// Funkcja odczytujaca wybrana strone do bufora z mozliwoscia odczytu na pojedyncze komorki strony
// buffer/in: wskaznik do bufora z danymi do odczytu
// PageNumber/in: zmienna okreslajaca numer strony (bajt)
// CellFrom/in: zmienna okreslajaca pierwsza komorke strony do odczytu
// CellTo/in: zmienna okreslajaca ostatnia komorke strony do odczytu
// return: zmienna statusu, argument ERROR lub SUCCESS
//-----------------------------------------------------------------------------
ErrorStatus SYSTEM_FLASH_ReadPage(uint16_t * buffer, uint8_t PageNumber, uint16_t CellTo)
{
	__disable_irq();
	uint32_t TimeOut;
	uint16_t i;

	FlashAccessStatus_t access = SYSTEM_FLASH_GetAccess();
	if (access == LOCK)
	{
		SYSTEM_FLASH_SetAccess(UNLOCK);
		access = SYSTEM_FLASH_GetAccess();
	}

	if (access == UNLOCK)
	{
		if (PageNumber <= 63 && PageNumber > 50)
		{
			for (i = 0; i < CellTo; i=i+2) // i = i+2 bo co dwa bajty jest write protect ??
			{
				// skalowanie bajtow na indeksy bufora. bo co dwa bajty zapis inaczej jest zabezpieczenie
				// a tak bym mial wpisywane lub odczytywane dane do bufora co 2 co jest bez sensu

				TimeOut = 10000000;
				while ((FLASH->SR & FLASH_SR_BSY)) // sprawdzam czy flash jest zajety
				{
					if (TimeOut > 0)
					{
						TimeOut--;
					}
					else
					{
						FLASH->SR |= FLASH_SR_EOP;
						SYSTEM_FLASH_SetAccess(LOCK);
						return ERROR;
					}
				}
				// odczytuje pierwszy bajt komorki (2048 * half-word)
				CurrentCellAddress = (uint8_t*)(FLASH_START_PAGE + PageNumber * PAGE_SIZE + i);
				// zapisuje odczytany bajt do bufora
				*buffer = *CurrentCellAddress;
				// odczytuje drugi bajt komorki (2048 * half-word)
				CurrentCellAddress = (uint8_t*)(FLASH_START_PAGE + PageNumber * PAGE_SIZE + i + 1);
				//tworze polowe slowa z dwoch odczytanych bajtow danej komorki od 0 do 2048 (cala strona)
				*buffer = *buffer + (*CurrentCellAddress << 8);
				buffer++;
			}
		}
	}

	SYSTEM_FLASH_SetAccess(LOCK);
	__enable_irq();
	return SUCCESS;
}
//-----------------------------------------------------------------------------
// Funkcja zapisujaca wybrana strone do bufora z mozliwoscia zapisu na pojedyncze komorki strony
// buffer/in: wskaznik do bufora z danymi do zapisu
// PageNumber/in: zmienna okreslajaca numer strony (bajt)
// CellFrom/in: zmienna okreslajaca pierwsza komorke strony do zapisu
// CellTo/in: zmienna okreslajaca ostatnia komorke strony do zapisu
// return: zmienna statusu, argument ERROR lub SUCCESS
//-----------------------------------------------------------------------------
ErrorStatus SYSTEM_FLASH_WritePage(uint16_t *buffer, uint8_t PageNumber, uint16_t DataSize)
{
	__disable_irq();
	uint16_t test_data;
	uint32_t TimeOut = 10000000;
	FlashAccessStatus_t access = SYSTEM_FLASH_GetAccess();
	if (access == LOCK)
	{
		SYSTEM_FLASH_SetAccess(UNLOCK);
		access = SYSTEM_FLASH_GetAccess();
	}

	if (access == UNLOCK)
	{
		uint16_t i;
		for (i = 0; i < DataSize; i++)
		{
			TimeOut = 10000000;
			while ((FLASH->SR & FLASH_SR_BSY)) // sprawdzam czy flash jest zajety
			{
				if (TimeOut > 0)
				{
					TimeOut--;
				}
				else
				{
					SYSTEM_FLASH_SetAccess(LOCK);
					return ERROR;
				}
			}

			// sprawdzam czy dana komorka strony jest skasowana, tzn. wartosc pojedynczego bajtu wynosi 0xFF
			test_data = *((uint8_t*)(FLASH_START_PAGE + PageNumber * PAGE_SIZE + i));
			if (test_data != 0xFF)
			{
				// powinno sie tez sprawdzic warning od FPEC, tzn. ustawia on bit PGERR w FLASH->SR
				SYSTEM_FLASH_SetAccess(LOCK);
				return ERROR;
			}
		}

		TimeOut = 10000000;
		while ((FLASH->SR & FLASH_SR_BSY)) // sprawdzam czy flash jest zajety
		{
			if (TimeOut > 0)
			{
				TimeOut--;
			}
			else
			{
				SYSTEM_FLASH_SetAccess(LOCK);
				return ERROR;
			}
		}

		FLASH->CR |= FLASH_CR_PG; // programowanie

		uint16_t y;
		for (y = 0; y < DataSize; y=y+2)
		{
			// skalowanie bajtow na indeksy bufora. bo co dwa bajty zapis inaczej jest zabezpieczenie
			// a tak bym mial wpisywane lub odczytywane dane do bufora co 2 co jest bez sensu

			write_address = (uint16_t*)(FLASH_START_PAGE + PageNumber * PAGE_SIZE + y);

//			FLASH->AR = write_address;
			TimeOut = 10000000;
			while ((FLASH->SR & FLASH_SR_BSY)) // sprawdzam czy flash jest zajety
			{
				if (TimeOut > 0)
				{
					TimeOut--;
				}
				else
				{
					FLASH->SR |= FLASH_SR_EOP;
					FLASH->CR &=~ FLASH_CR_PG;
					SYSTEM_FLASH_SetAccess(LOCK);
					return ERROR;
				}
			}

//			FLASH->AR = buffer[y];
			*write_address = *buffer++;

			FLASH->SR |= FLASH_SR_EOP; // kasuje flage end of operation gdy operacja przebiegla prawidlowo
		}
	}

	FLASH->CR &=~ FLASH_CR_PG; // kasuje flage programowania aby mozna bylo wykonywac inne operacje np. erase a potem znow write
	SYSTEM_FLASH_SetAccess(LOCK);
	__enable_irq();
	return SUCCESS;
}
