#ifndef __tsl2561
#define __tsl2561

#include "stm32f0xx.h"
#include "stm32f0xx_i2c.h"

// TSL2561 address definitions
#define TSL2561_GND_ADDR 				0x52	//address 7bits + write bit "0" -> 0b01010010
#define TSL2561AddressRead_Gnd 			0x53	//address 7bits + read bit "1" -> 0b01010011
#define TSL2561AddressWrite_Float 		0x72	//address 7bits + write bit "0" -> 0b01110010
#define TSL2561AddressRead_Float		0x73	//address 7bits + read bit "1" -> 0b01110011
#define TSL2561AddressWrite_Vcc 		0x92	//address 7bits + write bit "0" -> 0b10010010
#define TSL2561AddressRead_Vcc 			0x93	//address 7bits + read bit "1" -> 0b10010011

// Control Register (power control) definitions
#define ControlRegiser 					0x80	// Power Control Register: bits 1:0 control power)
#define TurnOnBits 						0x03	// 0x03 - turn on
#define TurnOffBits						0x00	// 0x00 - turn off

// Timing register definitions
#define TimingRegister 					0x81					// configure integration time for each conversion, configure gain
#define DefaultIntegrationTimeAndGain 	0x02	// default value - 402ms integration time for each conversion, gain x1
#define IntegrationTime101Gainx1 		0x01	// 101ms integration time for each conversion, gain x1
#define IntegrationTime13_7Gainx1 		0x00	// 13,7ms integration time for each conversion. gain x1
#define IntegrationTime101Gainx16 		0x11	// 101ms integration time for each conversion, gain x16
#define IntegrationTime13_7Gainx16 		0x10	// 13,7ms integration time for each conversion, gain x16
#define IntegrationTime402Gainx16 		0x12	// 402ms integration time for each conversion, gain x16

// Arguments for Calculate function
#define GainX1 							0
#define GainX16 						1
#define IntegrationTime13_7 			0
#define IntegrationTime100 				1
#define IntegrationTime402 				2
#define IntegrationTimeManual 			3
#define T_FN_CL_Package 				0
#define CS_Package 						1

#define LUX_SCALE 						14 		// scale by 2^14
#define RATIO_SCALE 					9 		// scale ratio by 2^9
//???????????????????????????????????????????????????
// Integration time scaling factors
//???????????????????????????????????????????????????
#define CH_SCALE 						10 		// scale channel values by 2^10
#define CHSCALE_TINT0 					0x7517 	// 322/11 * 2^CH_SCALE
#define CHSCALE_TINT1 					0x0fe7 	// 322/81 * 2^CH_SCALE
//???????????????????????????????????????????????????
// T, FN, and CL Package coefficients
//???????????????????????????????????????????????????
// For Ch1/Ch0=0.00 to 0.50
// Lux/Ch0=0.0304?0.062*((Ch1/Ch0)^1.4)
// piecewise approximation
// For Ch1/Ch0=0.00 to 0.125:
// Lux/Ch0=0.0304?0.0272*(Ch1/Ch0)
//
// For Ch1/Ch0=0.125 to 0.250:
// Lux/Ch0=0.0325?0.0440*(Ch1/Ch0)
//
// For Ch1/Ch0=0.250 to 0.375:
// Lux/Ch0=0.0351?0.0544*(Ch1/Ch0)
//
// For Ch1/Ch0=0.375 to 0.50:
// Lux/Ch0=0.0381?0.0624*(Ch1/Ch0)
//
// For Ch1/Ch0=0.50 to 0.61:
// Lux/Ch0=0.0224?0.031*(Ch1/Ch0)
//
// For Ch1/Ch0=0.61 to 0.80:
// Lux/Ch0=0.0128?0.0153*(Ch1/Ch0)
//
// For Ch1/Ch0=0.80 to 1.30:
// Lux/Ch0=0.00146?0.00112*(Ch1/Ch0)
//
// For Ch1/Ch0>1.3:
// Lux/Ch0=0
//???????????????????????????????????????????????????
#define K1T 							0x0040 // 0.125 * 2^RATIO_SCALE
#define B1T 							0x01f2 // 0.0304 * 2^LUX_SCALE
#define M1T 							0x01be // 0.0272 * 2^LUX_SCALE
#define K2T 							0x0080 // 0.250 * 2^RATIO_SCALE
#define B2T 							0x0214 // 0.0325 * 2^LUX_SCALE
#define M2T 							0x02d1 // 0.0440 * 2^LUX_SCALE
#define K3T 							0x00c0 // 0.375 * 2^RATIO_SCALE
#define B3T 							0x023f // 0.0351 * 2^LUX_SCALE
#define M3T 							0x037b // 0.0544 * 2^LUX_SCALE
#define K4T 							0x0100 // 0.50 * 2^RATIO_SCALE
#define B4T 							0x0270 // 0.0381 * 2^LUX_SCALE
#define M4T 							0x03fe // 0.0624 * 2^LUX_SCALE
#define K5T 							0x0138 // 0.61 * 2^RATIO_SCALE
#define B5T 							0x016f // 0.0224 * 2^LUX_SCALE
#define M5T 							0x01fc // 0.0310 * 2^LUX_SCALE
#define K6T 							0x019a // 0.80 * 2^RATIO_SCALE
#define B6T 							0x00d2 // 0.0128 * 2^LUX_SCALE
#define M6T 							0x00fb // 0.0153 * 2^LUX_SCALE
#define K7T 							0x029a // 1.3 * 2^RATIO_SCALE
#define B7T 							0x0018 // 0.00146 * 2^LUX_SCALE
#define M7T 							0x0012 // 0.00112 * 2^LUX_SCALE
#define K8T 							0x029a // 1.3 * 2^RATIO_SCALE
#define B8T 							0x0000 // 0.000 * 2^LUX_SCALE
#define M8T 							0x0000 // 0.000 * 2^LUX_SCALE
//???????????????????????????????????????????????????
// CS package coefficients
//???????????????????????????????????????????????????
// For 0 <= Ch1/Ch0 <= 0.52
// Lux/Ch0 = 0.0315?0.0593*((Ch1/Ch0)^1.4)
// piecewise approximation
// For 0 <= Ch1/Ch0 <= 0.13
// Lux/Ch0 = 0.0315?0.0262*(Ch1/Ch0)
// For 0.13 <= Ch1/Ch0 <= 0.26
// Lux/Ch0 = 0.0337?0.0430*(Ch1/Ch0)
// For 0.26 <= Ch1/Ch0 <= 0.39
// Lux/Ch0 = 0.0363?0.0529*(Ch1/Ch0)
// For 0.39 <= Ch1/Ch0 <= 0.52
// Lux/Ch0 = 0.0392?0.0605*(Ch1/Ch0)
// For 0.52 < Ch1/Ch0 <= 0.65
// Lux/Ch0 = 0.0229?0.0291*(Ch1/Ch0)
// For 0.65 < Ch1/Ch0 <= 0.80
// Lux/Ch0 = 0.00157?0.00180*(Ch1/Ch0)
// For 0.80 < Ch1/Ch0 <= 1.30
// Lux/Ch0 = 0.00338?0.00260*(Ch1/Ch0)
// For Ch1/Ch0 > 1.30
// Lux = 0
//???????????????????????????????????????????????????
#define K1C 							0x0043 // 0.130 * 2^RATIO_SCALE
#define B1C 							0x0204 // 0.0315 * 2^LUX_SCALE
#define M1C 							0x01ad // 0.0262 * 2^LUX_SCALE
#define K2C 							0x0085 // 0.260 * 2^RATIO_SCALE
#define B2C 							0x0228 // 0.0337 * 2^LUX_SCALE
#define M2C 							0x02c1 // 0.0430 * 2^LUX_SCALE
#define K3C 							0x00c8 // 0.390 * 2^RATIO_SCALE
#define B3C 							0x0253 // 0.0363 * 2^LUX_SCALE
#define M3C 							0x0363 // 0.0529 * 2^LUX_SCALE
#define K4C 							0x010a // 0.520 * 2^RATIO_SCALE
#define B4C 							0x0282 // 0.0392 * 2^LUX_SCALE
#define M4C 							0x03df // 0.0605 * 2^LUX_SCALE
#define K5C 							0x014d // 0.65 * 2^RATIO_SCALE
#define B5C 							0x0177 // 0.0229 * 2^LUX_SCALE
#define M5C 							0x01dd // 0.0291 * 2^LUX_SCALE
#define K6C 							0x019a // 0.80 * 2^RATIO_SCALE
#define B6C 							0x0101 // 0.0157 * 2^LUX_SCALE
#define M6C 							0x0127 // 0.0180 * 2^LUX_SCALE
#define K7C 							0x029a // 1.3 * 2^RATIO_SCALE
#define B7C 							0x0037 // 0.00338 * 2^LUX_SCALE
#define M7C 							0x002b // 0.00260 * 2^LUX_SCALE
#define K8C 							0x029a // 1.3 * 2^RATIO_SCALE
#define B8C 							0x0000 // 0.000 * 2^LUX_SCALE
#define M8C 							0x0000 // 0.000 * 2^LUX_SCALE

// Definitions of data from channels (byte protocol)
#define Data0LowByteMode 				0x8C	// Command 8 - 1000 set byte protocol, C - channel0 low data
#define Data0HighByteMode 				0x8D	// Command 8 - 1000 set byte protocol, D - channel0 high data
#define Data1LowByteMode 				0x8E	// Command 8 - 1000 set byte protocol, E - channel1 low data
#define Data1HighByteMode 				0x8F	// Command 8 - 1000 set byte protocol, F - channel1 high data

uint8_t Data0Low_uchar;
uint8_t Data0High_uchar;
uint8_t Data1Low_uchar;
uint8_t Data1High_uchar;
uint32_t Data0Low_uint;
uint32_t Data0High_uint;
uint32_t Data1Low_uint;
uint32_t Data1High_uint;
uint32_t Channel0_uint;
uint32_t Channel1_uint;

ErrorStatus TSL2561_Init(void);
uint8_t TSL2561_ReadByte(I2C_TypeDef* I2Cx, uint16_t SlaveAddr, uint8_t RegisterAddr, ErrorStatus * Error);
uint32_t TSL2561_ReadLux(ErrorStatus * Error);
uint32_t CalculateLux(uint32_t iGain, uint32_t tInt, uint32_t ch0, uint32_t ch1, int32_t iType);

#endif
