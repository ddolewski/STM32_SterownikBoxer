#ifndef MISC_H_
#define MISC_H_

#include "global.h"

uint32_t calcCRC32(const uint8_t * xData, uint32_t xLength);
uint8_t calcCRC8(const uint8_t * xData, uint32_t xLength);

uint8_t bin2bcd(uint8_t byte_to_conv);
uint8_t bcd2bin(uint8_t byte_to_conv);

void watchdogInit(uint32_t xTimeSec);
void watchdogFeed(void);

void memSet(void * xData, const uint8_t xValue, const uint32_t xSize);
void memCopy(void * xDst, const void * xSrc, uint32_t xSize);
void dummyFun(uint8_t * xByte);

int32_t StringComparison(const uint8_t * cmdA, const uint8_t * cmdB, int8_t xMode);
int32_t dataCmp(const uint8_t * xDataA, const uint8_t * xDataB, uint32_t xLength);
uint32_t IpFromString(uint8_t * xString, uint32_t * xError);
uint32_t CopyString(uint8_t * xDst, uint8_t * xSrc);
uint32_t MACFromString(uint8_t * xString, uint8_t *xOutput);
int32_t getIntValueFromText(uint8_t * xString);
uint8_t * IntToStr(uint32_t xValue, uint8_t * xBuffer);
uint32_t miscCopyString(uint8_t * xDst, uint8_t * xSrc);
int32_t miscStringComparison(const uint8_t * cmdA, const uint8_t * cmdB, int8_t xMode);
uint8_t miscDataEqual(const uint8_t * xPtrA, const uint8_t * xPtrB, uint32_t xLength);

void MISC_ResetARM(void);

#endif
