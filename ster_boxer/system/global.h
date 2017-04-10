/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GLOBAL_H
#define __GLOBAL_H

// STM32F051 Core M0 library
#include <timestamp.h>
#include "string.h"
#include "stm32f0xx.h"
#include "debug.h"
#include "misc.h"

/* Includes ------------------------------------------------------------------*/
// System perpipheral
#include "system_gpio.h"
#include "system_rcc.h"
#include "system_exti.h"
#include "system_timer.h"
#include "system_flash.h"

#include "boxer_ph.h"
#include "boxer_string.h"
#include "boxer_struct.h"
#include "boxer_climate.h"
#include "boxer_datastorage.h"
#include "boxer_communication.h"
#include "boxer_irrigation.h"
#include "boxer_timers.h"
#include "boxer_display.h"

#include "string_builder.h"
#include "hardware/SHT2x/sht2x.h"
#include "hardware/PCF8563/pcf8563.h"
#include "hardware/TSL2561/tsl2561.h"
#include "hardware/DS18B20/ds18b20.h"
#include "systime.h"
#include "misc.h"

//#define FLASH_SAVE_TEST
//#define FLASH_READ_TEST
//#define DISPLAY_PAGE1_TEST
//#define DISPLAY_PAGE2_TEST
//#define NTP_DEBUG
#define DEBUG_TERMINAL_USART
//#define DISPLAY_PAGE3_TEST
//#define BUZZER_OFF_MODE
//#define I2C_OFF_MODE
//#define OWIRE_OFF_MODE
//#define RTC_WRITE_TEST

#define SOFTWARE_VERSION		"2.7"

#define WIFI_RST_PORT			GPIOC
#define WIFI_RST_PIN			GPIOx_Pin_3

#define WIFI_PWR_PORT			GPIOF
#define WIFI_PWR_PIN			GPIOx_Pin_4

#define SOLENOID_PORT			GPIOF
#define SOLENOID_PIN			GPIOx_Pin_6

#define WATER_LEVEL_PORT		GPIOB
#define WATER_LEVEL_PIN			GPIOx_Pin_2

#define SOIL_MOIST_PORT			GPIOC
#define SOIL_MOIST_PIN			GPIOx_Pin_2

#define SOIL_MOIST_EN			GPIOC
#define SOIL_MOIST_EN_PIN		GPIOx_Pin_4

#define LAMP_PORT				GPIOF
#define LAMP_PIN				GPIOx_Pin_7

#define BUZZER_PORT				GPIOC
#define BUZZER_PIN				GPIOx_Pin_11

#define SYS_CLK_1MS				(uint32_t)(SystemCoreClock / 1000)

#define I2C1_SCL 				GPIOx_Pin_6
#define I2C1_SDA 				GPIOx_Pin_7

#define I2C2_SCL 				GPIOx_Pin_10
#define I2C2_SDA 				GPIOx_Pin_11

#define RSTCommand 				0x01
#define CalibrationCommand 		0x02
#define SaveFactorsCommand 		0x03

#define CHANNEL0 				0
#define CHANNEL1 				1
#define CHANNEL2 				2
#define CHANNEL3 				3

#define UC					(uint8_t *)

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#define SIZEOF_TAB(x) (sizeof(x)/sizeof(x[0]))

#define ASM_REV32	__REV

#define ASM_REV16	__REV16

typedef void (*fun_ptr_t)(void);

#define COMMAND(NAME)  {#NAME, cmd##NAME}

#define MAX(X,Y) 	((X)>(Y)?(X):(Y))
#define MIN(X,Y) 	((X)<(Y)?(X):(Y))
#define ABS_DIF(X,Y)(MAX(X,Y) - MIN(X,Y))
/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function which reports
  *         the name of the source file and the source line number of the call
  *         that failed. If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

#endif /* __STM32F0XX_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
