#include <boxer_bool.h>
#include "string_builder.h"
#include "string.h"

static int mod10(int xNumber);

void append(char* s, char c)
{
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

char* ftoa(float xNumber, char * xBuffer, uint8_t xAccuracy)
{
	int tmp, i;
	bool_t minus = FALSE;
	uint8_t offset = 0;
	float xValueF;

	xValueF = xNumber;

	if(xNumber < 0)
	{
		minus = TRUE;
		xNumber = -xNumber;		//przypisanie - pozniejsze operacje na liczbach dodatnich
	}

	if (minus)		//jesli liczba byla ujemna
	{
		if(xNumber < 1) 				//oraz nie ma czesci calkowitej
		{
			for (i=0; i < xAccuracy; i++)
			{
				xValueF *= 10.0;
			}

			if((int32_t)xValueF < 0)
			{
				*xBuffer = '-';
				offset = 1;
			}
			else
			{
				offset = 0;
			}
		}
		else
		{
			*xBuffer = '-';
			offset = 1;
		}
	}

	tmp = (int)xNumber;
	xBuffer = itoa(tmp, xBuffer + offset);
	xNumber = xNumber - tmp;		//czessc dziesietna

	for (i=0; i < xAccuracy; i++)
	{
		xNumber *= 10.0;
	}

	*xBuffer ='.';
	xBuffer = itoa(xNumber, xBuffer + 1);

	return xBuffer;
}

char* itoa(int xNumber, char* xBuffer)
{
	uint8_t itoaxBufferSize = 15;
	int tmp[itoaxBufferSize];
	bool_t minus = FALSE;
	unsigned char i, k;

	if (xNumber <0)
	{
		minus = TRUE;
		xNumber = -xNumber;
	}

	i = itoaxBufferSize-1;
	for( ; i >= 0; i--)
	{
		tmp[i] = mod10(xNumber);
		xNumber = xNumber/10;
		if (!xNumber)
		{
			break;
		}
	}

	if(minus)
	{
		i--;
		tmp[i] = '-' - 0x30;
	}

	k=i;
	for ( ; i < itoaxBufferSize; i++)
	{
		xBuffer[i-k] = tmp[i]+0x30;
	}


	xBuffer[i-k] = '\0';

	return xBuffer +i-k;
}

uint32_t atoi(char *str)
{
	uint32_t res = 0;
    uint32_t i;
    for (i = 0; str[i] != '\0'; ++i)
        res = res*10 + str[i] - '0';

    return res;
}

static int mod10(int xNumber)
{
	int data;
	data = xNumber/10;
	data *= 10;
	return xNumber - data;
}
