/*******************************************************************************
Biblioteka funkcji roznych - ogolnego przeznaczenia
mikrokontroler: STM32F407, 160 MHz
data utworzenia: 2012-07-19
data modyfikacji: 
autor: Tomasz Nowik @ RGB Technology

opis:
Rozne funkcje
********************************************************************************/

#include "misc.h"
#include "boxer_bool.h"

//-------------------------------------------------------------------
// FUNKCJA OPOXNIAJACA NIESKALOWANA
// xValue/in: bezwymiarowe opoznienie
//-------------------------------------------------------------------
void delay(volatile int xValue)
{
	while (xValue--);
}

//-------------------------------------------------------------------
// Reset mikrokontrolera
//-------------------------------------------------------------------
void resetMCU(void)
{
	delay(10);
	SCB->AIRCR = (0x5FA << 16) | (1<<2); // resetuje rdzen i peryferia (pelny reset uC)	
	while (1);
}

//-------------------------------------------------------------------
// Reset mikrokontrolera z opoznieniem zeby ewentualne napisy wypisaly sie do konca
// a pakiety zostaly wyslane
//-------------------------------------------------------------------
void delayNresetMCU(void)
{
	delay(100000);
	resetMCU();
}

//-------------------------------------------------------------------
// Funkcja do obliczania sumy kontrolnej CRC32
// xData/in: wskaxnik do tablicy; length/in: ilosc bajtow, z ktorych ma zostac policzone crc
// return: wyznaczone crc32
//-------------------------------------------------------------------
uint32_t calcCRC32(const uint8_t * xData, uint32_t xLength)
{
	uint32_t CRC32=0;
	uint8_t offset=0;
	while (xLength--)
	{
		CRC32 ^= ((uint32_t)*xData++)<<(offset++);
		if (offset > 24) offset = 0;
	}
	return CRC32;
}

//-------------------------------------------------------------------
// Obliczenie sumy CRC 8bitowej
// xData/in: wskaxnik do tablicy; length/in: ilosc bajtow, z ktorych ma zostac policzone crc
// return: wyznaczone crc8
//-------------------------------------------------------------------
uint8_t calcCRC8(const uint8_t * xData, uint32_t xLength)
{
	uint8_t CRC8 = 0;
	while (xLength--)
	{
		CRC8 ^= (*xData++);
	}
	return CRC8;
}

//-------------------------------------------------------------------
// FUNKCJA KONWERSJI BIN-->BCD
//-------------------------------------------------------------------
uint8_t bin2bcd(uint8_t byte_to_conv)
{
	uint8_t byte_converted;
	byte_converted = byte_to_conv/10;
	byte_converted <<= 4;
	byte_to_conv %= 10;
	byte_converted += byte_to_conv;
	return byte_converted;
}

//-------------------------------------------------------------------
// FUNKCJA KONWERSJI KODU BCD-->BIN
//-------------------------------------------------------------------
uint8_t bcd2bin(uint8_t byte_to_conv)
{
	uint8_t byte_converted;
	byte_converted = byte_to_conv & 0x0F;
	byte_to_conv >>= 4;
	byte_to_conv *= 10;
	byte_converted += byte_to_conv;
	return byte_converted;
}

//-----------------------------------------------------------------------------
// Inicjalizacja watchdoga
// xTimeSec/in: ilosc sekund timeoutu
//-----------------------------------------------------------------------------
void watchdogInit(uint32_t xTimeSec)
{
#ifdef WATCHDOG_ON
	//static const uint8_t watchdogTimeoutSec = 1;//timeout w sekundach - 1sek (max 31)
	if (xTimeSec < 1)
	{
		xTimeSec = 1;
	} 
	else if (xTimeSec > 31)
	{
		xTimeSec = 31;
	}
	__disable_irq();//Wylaczenie przerwan 	
	IWDG->KR = 0x5555;
	IWDG->PR = 6;//./256 - timeout od 8ms do 32768ms
	IWDG->RLR = 128 * xTimeSec;//timeout 
	IWDG->KR = 0xCCCC;
	__enable_irq();//Wlaczenie przerwan 	
	watchdogFeed();
#endif
}

//-----------------------------------------------------------------------------
// Nakarmienie watchdoga
//-----------------------------------------------------------------------------
void watchdogFeed(void)
{
	__disable_irq();//Wylaczenie przerwan 	
	IWDG->KR = 0x5555;
	IWDG->KR = 0xAAAA;
	__enable_irq();//Wlaczenie przerwan 	
}

//-----------------------------------------------------------------------------
// Porownanie dwoch stringow
// xMode: 0 - cmdA nie musi byc zakonczone 0; 1 - A i B musza byc identyczne
// return: 0 - nie sa identyczne; 1 - sa identyczne
//-----------------------------------------------------------------------------
int32_t StringComparison(const uint8_t * cmdA, const uint8_t * cmdB, int8_t xMode)
{
	int i=0;

	while (*cmdA != 0 && *cmdB != 0)
	{
		if (*cmdA != *cmdB) 
		{
			return 0;
		}
		cmdA++;
		cmdB++;
		i++;
		if (i > 400) 
		{
			return 0;//jezeli stringu sa nie zakonczone zerem (lub dluzsze niz 400 znakow)
		}
	}

	if (xMode == 0)
	{
		if (*cmdB != 0) 
		{
			return 0;
		}
	}
	else if (xMode == 1)
	{
		if ((*cmdA != 0) || (*cmdB != 0)) 
		{
			return 0;
		}
	}

	return 1;
}

//-----------------------------------------------------------------------------
// porownanie dwoch blokow danych
// xDataA/in, xDataB/in - obszary do porownania
// xLength/in - dlugosc blokow
// return: 0 - nie sa identyczne; 1 - sa identyczne
//-----------------------------------------------------------------------------
int32_t dataCmp(const uint8_t * xDataA, const uint8_t * xDataB, uint32_t xLength)
{
	while (xLength--)
	{
		if (*xDataA++ != *xDataB++)
		{
			return 0;
		}
	}
	return 1;
}

//-----------------------------------------------------------------------------
// Wypelnienie obszaru pamieci wartoscia
// xData/out: obszar pamieci do zapisania
// xValue/in: wartosc do wypelnienia bloku pamieci
// xSize/in: rozmiar blokow
//-----------------------------------------------------------------------------
void memSet(void * xData, const uint8_t xValue, uint32_t xSize)
{
	uint8_t * data = (uint8_t *) xData;
	while (xSize--)
	{
		*data++ = xValue;
	}
}

//-----------------------------------------------------------------------------
// Kopiowanie blokow pamieci
// xDst/out: obszar, na ktory maja byc skopiowane dane
// xSrc/in: obszar, z ktorego maja byc skopiowane dane
// xSize/in: rozmiar buforow do skopiowania
//-----------------------------------------------------------------------------
void memCopy(void * xDst, const void * xSrc, uint32_t xSize)
{
	uint8_t * dst = (uint8_t *) xDst;
	uint8_t * src = (uint8_t *) xSrc;
	while (xSize--)
	{
		*dst++ = *src++;
	}
}

void dummyFun(uint8_t * xByte)
{
	
}

//-----------------------------------------------------------------------------
// Funkcja zamieniajaca stringa na adres IP
// xString/in: string do przetworzenia; xError/out: wskaznik na zmienna gdzie blad bedzie zapisany
// return: zwracany IP jest w postaci unsigned int; jezeli wystapi blad to 0; xError: jezeli != 0 to blad
//-----------------------------------------------------------------------------
uint32_t IpFromString(uint8_t * xString, uint32_t * xError)
{
	int32_t i;
	uint32_t tempIP=0, temp;
	
	i = 24;
	*xError = 1;

	do {
		if ((*xString<'0') || (*xString>'9')) return 0;
		temp=0;
		do {
			temp = temp * 10;
			temp += ((*xString++) - '0');			
		} while ((*xString>='0') && (*xString<='9'));

		if (temp > 255) return 0;

		tempIP |= temp<<i;
		xString++;
		i-=8;

	} while(i >= 0);

	*xError = 0;
	return tempIP;
}

//-----------------------------------------------------------------------------
// Skopiowanie stringa z xScr do xDst (max 1000 znakow)
//-----------------------------------------------------------------------------
uint32_t CopyString(uint8_t * xDst, uint8_t * xSrc)
{	
	uint8_t xLength = 0;
	do
	{
		if (*xSrc == 0) 
		{
			*xDst = 0;
			return xLength;
		}
		*xDst++ = *xSrc++;
	} while(xLength++ < 1000);
	
	return 1000;
}

//-----------------------------------------------------------------------------
// Zamiana wartosci Hex na DEC
//-----------------------------------------------------------------------------
uint32_t getHex(uint8_t xValue)
{
	if ((xValue >= '0') & (xValue <= '9'))
	{
		xValue -= '0';
	} 
	else if ((xValue >= 'a') & (xValue <= 'f'))
	{
		xValue -= 'a'-10;
	}
	else if ((xValue >= 'A') & (xValue <= 'F'))
	{
		xValue -= 'A'-10;
	}
	else
	{
		return 0xFF;
	}

	return xValue;
}

//-----------------------------------------------------------------------------
// Funkcja zamieniajaca stringa na adres MAC
// xString/in: string do przetworzenia; xError/out: wskaznik na zmienna gdzie blad bedzie zapisany
// return: zwracany IP jest w postaci unsigned int; jezeli wystapi blad to 0; xError: jezeli != 0 to blad
//-----------------------------------------------------------------------------
uint32_t MACFromString(uint8_t * xString, uint8_t *xOutput)
{
	int32_t i;
	int32_t temp;
	i = 6*2+5;
	for(i=0;i<6;i++)
	{
		temp = getHex(*xString++)<<4;
		if ((temp>>4) == 0xFF) return 1;
		temp |= getHex(*xString++);
		if ((temp & 0xFF) == 0xFF) return 1;
		
		*xOutput++ = temp;
		temp = *xString++;
		
		if (i != 5)
		{
			if ((temp != '.') && (temp != ',') && (temp != '-') && (temp != ':')) return 1;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Pobranie liczby ze stringa. za liczba mozne znajdowac sie tekst
// return: wartosc odczytana ze stringa
//-----------------------------------------------------------------------------
int32_t getIntValueFromText(uint8_t * xString)
{
	int Value = 0;
	char temp = 0;
	int i=0;

	if (xString == 0) return -1;//sprawdzenie czy wskaznik gdzies pokazuje

	if ((((*xString) < '0') || ((*xString) > '9')) && (*xString != 0))//dlaczego tak?
	{
		return -1;
	}

	while(((*xString) >= '0') && ((*xString) <= '9'))
	{
		Value *= 10;
		temp = *xString++ - '0';
		Value += temp;
		if (i++ > 10) return -1;
	}
	return Value;
}

//-----------------------------------------------------------------------------
// Zamiana integera na stringa
// xValue - wartosc int wejsciowa
// xBuffer - bufor wyjsciowy
// return: wskaznik na bufor wyjsciowy podany na wejsciu
//-----------------------------------------------------------------------------
uint8_t * IntToStr(uint32_t xValue, uint8_t * xBuffer)
{
	uint32_t div = 1000000000;
	int32_t v;
	int32_t first=0;
	uint8_t * output = xBuffer;
	
	if (xValue)
	{
		while(div)
		{
			v = xValue / div;
			if (v)
			{
				*xBuffer++ = '0' + v;
				first = 1;
			}
			else if (first)
			{
				*xBuffer++ = '0';
			}
			xValue -= v * div;
			div /= 10;
		}
	}
	else
	{	
		*xBuffer++ = '0';
	}

	*xBuffer = 0;
	return output;
}
//-----------------------------------------------------------------------------
// Skopiowanie stringa z xScr do xDst o dlugosci xLength
// xDst/out: wskaznik na string docelowy
// xSrc/in: wskaznik na string zrodlowy
// return: dlugosc skopiowanego ciagu
//-----------------------------------------------------------------------------
uint32_t miscCopyString(uint8_t * xDst, uint8_t * xSrc)
{
	uint16_t xLength = 0;
	do
	{
		if (*xSrc == 0)
		{
			*xDst = 0;
			return xLength;
		}
		*xDst++ = *xSrc++;
	} while(xLength++ < 1000);

	return 1000;
}

//-----------------------------------------------------------------------------
// Porownanie dwoch stringow
// xMode: 0 - cmdA nie musi byc zakonczone 0; 1 - A i B musza byc identyczne
// return: 0 - nie sa identyczne; 1 - sa identyczne
//-----------------------------------------------------------------------------
int32_t miscStringComparison(const uint8_t * cmdA, const uint8_t * cmdB, int8_t xMode)
{
	int i=0;

	while (*cmdA != 0 && *cmdB != 0)
	{
		if (*cmdA != *cmdB)
		{
			return 0;
		}
		cmdA++;
		cmdB++;
		i++;
		if (i > 400)
		{
			return 0;//jezeli stringu sa nie zakonczone zerem (lub dluzsze niz 200 znakow)
		}
	}

	if (xMode == 0)
	{
		if (*cmdB != 0)
		{
			return 0;
		}
	}
	else if (xMode == 1)
	{
		if ((*cmdA != 0) || (*cmdB != 0))
		{
			return 0;
		}
	}

	return 1;
}

uint8_t miscDataEqual(const uint8_t * xPtrA, const uint8_t * xPtrB, uint32_t xLength)
{
	uint8_t dataEqual = TRUE;

	while(xLength--)
	{
		if(*xPtrA != *xPtrB)
		{
			dataEqual = FALSE;
			break;
		}
		xPtrA++;
		xPtrB++;
	}
	return dataEqual;
}

void MISC_ResetARM(void)
{
	NVIC_SystemReset();
	while (TRUE)
	{
		__NOP();
	}
}
