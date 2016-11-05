#ifndef SYSTEM_EXTI_H_
#define SYSTEM_EXTI_H_
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stm32f0xx.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXTI0				(uint32_t)0x00000001	// Line 0
#define EXTI1				(uint32_t)0x00000002	// Line 1
#define EXTI2				(uint32_t)0x00000004	// Line 2
#define EXTI3				(uint32_t)0x00000008	// Line 3
#define EXTI4				(uint32_t)0x00000010	// Line 4
#define EXTI5				(uint32_t)0x00000020	// Line 5
#define EXTI6				(uint32_t)0x00000040	// Line 6
#define EXTI7				(uint32_t)0x00000080	// Line 7
#define EXTI8				(uint32_t)0x00000100	// Line 8
#define EXTI9				(uint32_t)0x00000200	// Line 9
#define EXTI10				(uint32_t)0x00000400	// Line 10
#define EXTI11				(uint32_t)0x00000800	// Line 11
#define EXTI12				(uint32_t)0x00001000	// Line 12
#define EXTI13				(uint32_t)0x00002000	// Line 13
#define EXTI14				(uint32_t)0x00004000	// Line 14
#define EXTI15				(uint32_t)0x00008000	// Line 15
#define EXTI16				(uint32_t)0x00010000 	// PVD output
#define EXTI17				(uint32_t)0x00020000	// RTC alarm event
#define EXTI18				(uint32_t)0x00040000	// USB wakeup event
#define EXTI19				(uint32_t)0x00080000	// RTC Tamper i TimeStamp event
#define EXTI20				(uint32_t)0x00100000	// RESERVED
#define EXTI21				(uint32_t)0x00200000	// Comparator output 1
#define EXTI22				(uint32_t)0x00400000	// Comparator output 2
#define EXTI23				(uint32_t)0x00800000	// I2C1 wakeup event
#define EXTI24				(uint32_t)0x01000000	// RESERVED
#define EXTI25				(uint32_t)0x02000000	// USART1 wakeup event
#define EXTI26				(uint32_t)0x04000000	// RESERVED
#define EXTI27				(uint32_t)0x08000000	// CEC wakeup event
#define EXTI28				(uint32_t)0x10000000	// RESERVED
#define EXTI29				(uint32_t)0x20000000	// RESERVED
#define EXTI30				(uint32_t)0x40000000	// RESERVED
#define EXTI31				(uint32_t)0x80000000	// VDDIO2 supply comparator output

#define AFIO_EXTI_SPECIAL_LINE		0xA // w przypadu konfiguracji lini specjalnych (od EXTI16 w górê)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	Edge_Falling = 0x01,
	Edge_Rising = 0x02,
	Edge_Both = 0x03
}Edge_t;

typedef enum
{
	Mode_SoftwareInterruptEvent = 0x01,
	Mode_HardwareInterrupt = 0x02,
	Mode_HardwareEvent = 0x03
}Mode_t;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorStatus EXTI_ConfigLine(uint32_t EXTIx, uint16_t AFIO_EXTICRx_EXTIy_Pz, uint8_t Edge, uint8_t Mode);
void EXTI_ClearITFlag(uint32_t EXTIx);
uint8_t EXTI_GetITFlag(uint32_t EXTIx);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif /* SYSTEM_EXTI_H_ */
