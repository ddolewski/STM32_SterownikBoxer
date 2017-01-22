#include "string_builder.h"

static int mod10(int xNumber);

char* ftoa(float xNumber, char * xBuffer, uint8_t xAccuracy)
{
	int tmp, i;
	uint8_t minus = False;
	uint8_t offset = 0;
	float xValueF;

	xValueF = xNumber;

	if(xNumber < 0)
	{
		minus = True;
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
	unsigned char tmp[itoaxBufferSize];
	uint8_t minus = False;
	char i, k;

	if (xNumber <0)
	{
		minus = True;
		xNumber = -xNumber;
	}

	i= itoaxBufferSize-1;
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

uint32_t atoi(uint8_t *str)
{
	uint32_t res = 0; // Initialize result
    uint32_t i;
    // Iterate through all characters of input string and update result
    for (i = 0; str[i] != '\0'; ++i)
        res = res*10 + str[i] - '0';

    // return result.
    return res;
}

//-----------------------------------------------------------------------------
// Funkcja konwertujaca liczbe int na string
// xValue/in: liczba do konwersji na string
// xString/in: wskaznik do skonwertowanego stringa
// xRadix/in: system liczbowy, 2 - binarnie, 10 - decymalnie, itd.
//-----------------------------------------------------------------------------
/*
int itoa(int xValue, char *xString, int xRadix)
{
    char tmp[16];
    char *tp = tmp;
    int i;
    unsigned v;

    int sign = ((xRadix == 10) && (xValue < 0));
    if (sign)
    {
    	 v = -xValue;
    }
    else
    {
        v = (unsigned)xValue;
    }

    while (v || tp == tmp)
    {
        i = v % xRadix;
        v /= xRadix;
        if (i < 10)
        {
        	*tp++ = i + '0';
        }
        else
        {
        	*tp++ = i + 'a' - 10;
        }
    }

    int len = tp - tmp;

    if (sign)
    {
        *xString++ = '-';
        len++;
    }

    while (tp > tmp)
    {
        *xString++ = *--tp;
    }

    return len;
}
*/
static int mod10(int xNumber)
{
	int data;
	data = xNumber/10;
	data *= 10;
	return xNumber - data;
}
