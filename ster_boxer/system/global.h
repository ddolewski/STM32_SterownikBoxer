/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GLOBAL_H
#define __GLOBAL_H

#include "stm32f0xx.h"

//#define FLASH_SAVE_TEST
//#define FLASH_READ_TEST
//#define DISPLAY_PAGE1_TEST
//#define DISPLAY_PAGE2_TEST
//#define NTP_DEBUG
//#define TURN_OFF_FIRST_NTP_REQ
#define DEBUG_TERMINAL_USART
//#define DISPLAY_PAGE3_TEST
//#define BUZZER_OFF_MODE
//#define I2C_OFF_MODE
//#define OWIRE_OFF_MODE
//#define RTC_WRITE_TEST
//#define I2C2_LOGS
#define ONE_WIRE_LOGS
//#define SEND_TRANSMIT_FRAME

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

#define I2C1_SCL 				GPIOx_Pin_6
#define I2C1_SDA 				GPIOx_Pin_7

#define I2C2_SCL 				GPIOx_Pin_10
#define I2C2_SDA 				GPIOx_Pin_11

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#define SIZEOF_TAB(x) (sizeof(x)/sizeof(x[0]))

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
