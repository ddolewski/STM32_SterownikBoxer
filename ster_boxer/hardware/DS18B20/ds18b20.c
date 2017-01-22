#include "ds18b20.h"

const uint8_t sensor1ROM[8]={40, 255, 8, 153, 100, 20, 3, 10};
const uint8_t sensor2ROM[8]={40, 21, 46, 135, 4, 0, 0, 53};


GPIO_InitTypeDef GPIO_InitStructure;
static inline void ASM_DelayUS(uint32_t us);

static inline void ASM_DelayUS(uint32_t us)
{
	/* So (2^32)/12 micros max, or less than 6 minutes */
	us *= 10;
	us -= 2; //offset seems around 2 cycles
	/* fudge for function call overhead */
	us--;
	__ASM volatile(" mov r0, %[us] \n\t"
	".syntax unified \n\t"
	"1: subs r0, #1 \n\t"
	".syntax divided \n\t"
	" bhi 1b \n\t"
	:
	: [us] "r" (us)
	: "r0");
}

void OneWire_Low(void)
{
	GPIOx_ResetPin(DS18B20_PORT, DS18B20_PIN);
}

void OneWire_High(void)
{
	GPIOx_SetPin(DS18B20_PORT, DS18B20_PIN);
}

void OneWire_In(void)
{
	GPIOx_PinConfig(DS18B20_PORT, Mode_In, 0, 0, OState_PD, DS18B20_PIN);
}

void OneWire_Out(void)
{
	GPIOx_PinConfig(DS18B20_PORT, Mode_Out, 0, OType_PP, OState_PD, DS18B20_PIN);
}
//Funkcja resetuje magistrale i oczekuje na impuls Presence
uint8_t uc1Wire_ResetPulse(void)
{
	uint8_t ucPresenceImpulse;

	OneWire_Out();
	OneWire_Low();
	ASM_DelayUS(600); //minimum 480us
	OneWire_In();
	ASM_DelayUS(50); //15-60us

	if (GPIOx_ReadInputPin(DS18B20_PORT, DS18B20_PIN))
	{
		ucPresenceImpulse = 1;
	}
	else
	{
		ucPresenceImpulse = 0;
	}

	 ASM_DelayUS(300); //60-240us

	if (GPIOx_ReadInputPin(DS18B20_PORT, DS18B20_PIN))
	{
		ucPresenceImpulse = 1;
	}
	else
	{
		ucPresenceImpulse = 0;
	}

	return ucPresenceImpulse;
}

//Funkcja wysyla na magistrale pojedynczy bit
void v1Wire_SendBit(uint8_t cBit)
{
	OneWire_Out();
	OneWire_Low();

	ASM_DelayUS(1);

	if (cBit == 1)
	{
		OneWire_High();
	}

	ASM_DelayUS(50);
	OneWire_High();
}

//Funkcja odbiera bit z magistrali
uint8_t uc1Wire_ReadBit(void)
{
	uint8_t ucBit;

	GPIOx_PinConfig(DS18B20_PORT, Mode_Out, 0, OType_PP, OState_PD, DS18B20_PIN);
	DS18B20_PORT->BRR = DS18B20_PIN;
	ASM_DelayUS(1);

	GPIOx_PinConfig(DS18B20_PORT, Mode_In, 0, 0, OState_PD, DS18B20_PIN);
	ASM_DelayUS(10);

	if (GPIOx_ReadInputPin(DS18B20_PORT, DS18B20_PIN))
	{
		ucBit = 1;
	}
	else
	{
		ucBit = 0;
	}

	return ucBit;
}

//Funkcja wysyla bajt na magistrale
void v1Wire_SendByte(uint8_t ucByteValue)
{
	uint8_t ucCounter;
	uint8_t ucValueToSend;

	for (ucCounter = 0; ucCounter < 8; ucCounter++)
	{
		ucValueToSend = ucByteValue >> ucCounter;
		ucValueToSend &= (1 << 0);
		v1Wire_SendBit(ucValueToSend);
	}

	ASM_DelayUS(100);
}

//Funkcja odbiera bajt z magistrali
uint8_t uv1Wire_ReadByte(void)
{
	uint8_t ucCounter;
	uint8_t ucReadByte = 0;

	for (ucCounter = 0; ucCounter < 8; ucCounter++)
	{
		if (uc1Wire_ReadBit())
		{
			ucReadByte |= (1 << ucCounter);
		}

		ASM_DelayUS(50);
	}

	return ucReadByte;
}

//funkcja odczytuje unikalny kod pojedynczego czujnika
void readROM(uint8_t *buffer)
{
	uint8_t ucReset;
	uint32_t i;
	ucReset = uc1Wire_ResetPulse();
	v1Wire_SendByte(0x33);

	for(i = 0; i < 8; i++)
	{
		buffer[i] = uv1Wire_ReadByte();
	}
}

//funkcja wysyla unikalny kod czujnika
void sendROM(uint8_t * table)
{
	uint32_t i;
	for (i = 0; i < 8; i++)
	{
		v1Wire_SendByte(table[i]);
	}
}

//funkcja inicjalizujaca konwersje temperatury danego czujnika
uint8_t initializeConversion(DS18B20Sensor_t * sensor)
{
	uint8_t ucReset;
	ucReset = uc1Wire_ResetPulse();
	
	if (ucReset == 1)
	{
		v1Wire_SendByte(0x55);
		sendROM(sensor->cROM);
		v1Wire_SendByte(0x44);
	}
	
	return ucReset;
}

uint8_t CRC8(uint8_t *inData, uint8_t len)
{
   uint8_t crc;
   crc = 0;
   for(; len; len--)
   {
      crc ^= *inData++;
      crc ^= (crc << 3) ^ (crc << 4) ^ (crc << 6);
      crc ^= (crc >> 4) ^ (crc >> 5);
   }
   return crc;
}

void readTemperature(DS18B20Sensor_t * sensor) //todo odczytywac caly scratchpad razem z crc - policzyc crc z odebranych 8 bajtow i porwonac z 9
{
	uint8_t ucReset;
	uint8_t crcData[8] = {0};
	ucReset = uc1Wire_ResetPulse();

	v1Wire_SendByte(0x55);
	sendROM(sensor->cROM);
	v1Wire_SendByte(0xBE);
	sensor->cTempL = uv1Wire_ReadByte();
	sensor->cTempH = uv1Wire_ReadByte();
	crcData[0] = sensor->cTempL;
	crcData[1] = sensor->cTempH;

	crcData[2] =  uv1Wire_ReadByte();
	crcData[3] =  uv1Wire_ReadByte();
	crcData[4] =  uv1Wire_ReadByte();
	crcData[5] =  uv1Wire_ReadByte();
	crcData[6] =  uv1Wire_ReadByte();
	crcData[7] =  uv1Wire_ReadByte();
	uint8_t crcDS18B20 = uv1Wire_ReadByte();
	ucReset = uc1Wire_ResetPulse();

	uint8_t crcCalculated = CRC8(crcData, 8);

	if (crcDS18B20 == crcCalculated)
	{
		sensor->fTemp = (float)(sensor->cTempL + (sensor->cTempH << 8))/16;
	}
}

void readTemperatureChar(DS18B20Sensor_t * sensor)
{
	uint8_t ucReset;
	int cLSB[8]	=	{8, 4, 2, 1, 500, 250, 125, 62};
	int cMSB[3]	=	{64, 32, 16};
	
	int iInteger = 0;
	int iFraction = 0;

	ucReset = uc1Wire_ResetPulse();

	v1Wire_SendByte(0x55);
	sendROM(sensor->cROM);
	v1Wire_SendByte(0xBE);
	sensor->cTempL = uv1Wire_ReadByte();
	sensor->cTempH = uv1Wire_ReadByte();
	ucReset = uc1Wire_ResetPulse();

	//czesc calkowita	
	if (sensor->cTempL & 1 << 7)
		iInteger+=	cLSB[0];

	if (sensor->cTempL & 1 << 6)
		iInteger+=	cLSB[1];

	if (sensor->cTempL & 1 << 5)
		iInteger+=	cLSB[2];

	if (sensor->cTempL & 1 << 4)
		iInteger+=	cLSB[3];

	if (sensor->cTempH & 1 << 2)
		iInteger+=	cMSB[0];

	if (sensor->cTempH & 1 << 1)
		iInteger+=	cMSB[1];	

	if (sensor->cTempH & 1 << 0)
		iInteger+=	cMSB[2];
	
	//czesc ulamkowa
	if (sensor->	cTempL & 1 << 3)
		iFraction+=	cLSB[4];	
	if (sensor->	cTempL & 1 << 2)
		iFraction+=	cLSB[5];	
	if (sensor->	cTempL & 1 << 1)
		iFraction+=	cLSB[6];
	if (sensor->	cTempL & 1 << 0)
		iFraction+=	cLSB[7];	
	
	int length = strlen(sensor->cAtempL);
	sensor->cAtempL[length] = '.';
	sensor->cAtempL[length + 1] = '\0';
	
	strcat (sensor->cAtempL, sensor->cAtempH);
}

// MULTI 1Wire Algorithm
//--------------------------------------------------------------------------
// Find the 'first' devices on the 1-Wire bus
// Return True  : device found, ROM number in ROM_NO buffer
//        False : no device present
//
int OWFirst()
{
   // reset the search state
   LastDiscrepancy = 0;
   LastDeviceFlag = False;
   LastFamilyDiscrepancy = 0;

   return OWSearch();
}

//--------------------------------------------------------------------------
// Find the 'next' devices on the 1-Wire bus
// Return True  : device found, ROM number in ROM_NO buffer
//        False : device not found, end of search
//
int OWNext()
{
   // leave the search state alone
   return OWSearch();
}

//--------------------------------------------------------------------------
// Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing
// search state.
// Return True  : device found, ROM number in ROM_NO buffer
//        False : device not found, end of search
//
int OWSearch()
{
   int id_bit_number;
   int last_zero, rom_byte_number, search_result;
   int id_bit, cmp_id_bit;
   unsigned char rom_byte_mask, search_direction;

   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = 0;
   crc8 = 0;

   // if the last call was not the last one
   if (!LastDeviceFlag)
   {
      // 1-Wire reset
//      if (!OWReset())
//      {
//         // reset the search
//         LastDiscrepancy = 0;
//         LastDeviceFlag = False;
//         LastFamilyDiscrepancy = 0;
//         return False;
//      }

  	if (!(DS18B20_PORT->IDR & (1 << 8)))
  	{
        // reset the search
        LastDiscrepancy = 0;
        LastDeviceFlag = False;
        LastFamilyDiscrepancy = 0;
        return False;
  	}


      // issue the search command
//      OWWriteByte(0xF0);
      v1Wire_SendByte(0xF0);

      // loop to do the search
      do
      {
         // read a bit and its complement
         id_bit = uc1Wire_ReadBit();
         cmp_id_bit = uc1Wire_ReadBit();

         // check for no devices on 1-wire
         if ((id_bit == 1) && (cmp_id_bit == 1))
            break;
         else
         {
            // all devices coupled have 0 or 1
            if (id_bit != cmp_id_bit)
               search_direction = id_bit;  // bit write value for search
            else
            {
               // if this discrepancy if before the Last Discrepancy
               // on a previous next then pick the same as last time
               if (id_bit_number < LastDiscrepancy)
                  search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
               else
                  // if equal to last pick 1, if not then pick 0
                  search_direction = (id_bit_number == LastDiscrepancy);

               // if 0 was picked then record its position in LastZero
               if (search_direction == 0)
               {
                  last_zero = id_bit_number;

                  // check for Last discrepancy in family
                  if (last_zero < 9)
                     LastFamilyDiscrepancy = last_zero;
               }
            }

            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1)
              ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
              ROM_NO[rom_byte_number] &= ~rom_byte_mask;

            // serial number search direction write bit
            v1Wire_SendBit(search_direction);

            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;

            // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
            if (rom_byte_mask == 0)
            {
                docrc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
                rom_byte_number++;
                rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

      // if the search was successful then
      if (!((id_bit_number < 65) || (crc8 != 0)))
      {
         // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
         LastDiscrepancy = last_zero;

         // check for last device
         if (LastDiscrepancy == 0)
            LastDeviceFlag = True;

         search_result = True;
      }
   }

   // if no device found then reset counters so next 'search' will be like a first
   if (!search_result || !ROM_NO[0])
   {
      LastDiscrepancy = 0;
      LastDeviceFlag = False;
      LastFamilyDiscrepancy = 0;
      search_result = False;
   }

   return search_result;
}

//--------------------------------------------------------------------------
// Verify the device with the ROM number in ROM_NO buffer is present.
// Return True  : device verified present
//        False : device not present
//
int OWVerify()
{
   unsigned char rom_backup[8];
   int i,rslt,ld_backup,ldf_backup,lfd_backup;

   // keep a backup copy of the current state
   for (i = 0; i < 8; i++)
      rom_backup[i] = ROM_NO[i];
   ld_backup = LastDiscrepancy;
   ldf_backup = LastDeviceFlag;
   lfd_backup = LastFamilyDiscrepancy;

   // set search to find the same device
   LastDiscrepancy = 64;
   LastDeviceFlag = False;

   if (OWSearch())
   {
      // check if same device found
      rslt = True;
      for (i = 0; i < 8; i++)
      {
         if (rom_backup[i] != ROM_NO[i])
         {
            rslt = False;
            break;
         }
      }
   }
   else
     rslt = False;

   // restore the search state
   for (i = 0; i < 8; i++)
      ROM_NO[i] = rom_backup[i];
   LastDiscrepancy = ld_backup;
   LastDeviceFlag = ldf_backup;
   LastFamilyDiscrepancy = lfd_backup;

   // return the result of the verify
   return rslt;
}

//--------------------------------------------------------------------------
// Setup the search to find the device type 'family_code' on the next call
// to OWNext() if it is present.
//
void OWTargetSetup(unsigned char family_code)
{
   int i;

   // set the search state to find SearchFamily type devices
   ROM_NO[0] = family_code;
   for (i = 1; i < 8; i++)
      ROM_NO[i] = 0;
   LastDiscrepancy = 64;
   LastFamilyDiscrepancy = 0;
   LastDeviceFlag = False;
}

//--------------------------------------------------------------------------
// Setup the search to skip the current device type on the next call
// to OWNext().
//
void OWFamilySkipSetup()
{
   // set the Last discrepancy to last family discrepancy
   LastDiscrepancy = LastFamilyDiscrepancy;
   LastFamilyDiscrepancy = 0;

   // check for end of list
   if (LastDiscrepancy == 0)
      LastDeviceFlag = True;
}

// TEST BUILD
static unsigned char dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
      157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
       35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
      190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
       70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
      219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
      101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
      248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
      140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
       17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
      175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
       50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
      202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
       87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
      233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
      116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current
// global 'crc8' value.
// Returns current global crc8 value
//
unsigned char docrc8(unsigned char value)
{
   // See Application Note 27

   // TEST BUILD
   crc8 = dscrc_table[crc8 ^ value];
   return crc8;
}
