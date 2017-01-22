################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../st_src/stm32f0xx_adc.c \
../st_src/stm32f0xx_crc.c \
../st_src/stm32f0xx_dma.c \
../st_src/stm32f0xx_exti.c \
../st_src/stm32f0xx_flash.c \
../st_src/stm32f0xx_gpio.c \
../st_src/stm32f0xx_i2c.c \
../st_src/stm32f0xx_misc.c \
../st_src/stm32f0xx_pwr.c \
../st_src/stm32f0xx_rcc.c \
../st_src/stm32f0xx_syscfg.c \
../st_src/stm32f0xx_tim.c \
../st_src/stm32f0xx_usart.c 

O_SRCS += \
../st_src/stm32f0xx_gpio.o \
../st_src/stm32f0xx_rcc.o 

OBJS += \
./st_src/stm32f0xx_adc.o \
./st_src/stm32f0xx_crc.o \
./st_src/stm32f0xx_dma.o \
./st_src/stm32f0xx_exti.o \
./st_src/stm32f0xx_flash.o \
./st_src/stm32f0xx_gpio.o \
./st_src/stm32f0xx_i2c.o \
./st_src/stm32f0xx_misc.o \
./st_src/stm32f0xx_pwr.o \
./st_src/stm32f0xx_rcc.o \
./st_src/stm32f0xx_syscfg.o \
./st_src/stm32f0xx_tim.o \
./st_src/stm32f0xx_usart.o 

C_DEPS += \
./st_src/stm32f0xx_adc.d \
./st_src/stm32f0xx_crc.d \
./st_src/stm32f0xx_dma.d \
./st_src/stm32f0xx_exti.d \
./st_src/stm32f0xx_flash.d \
./st_src/stm32f0xx_gpio.d \
./st_src/stm32f0xx_i2c.d \
./st_src/stm32f0xx_misc.d \
./st_src/stm32f0xx_pwr.d \
./st_src/stm32f0xx_rcc.d \
./st_src/stm32f0xx_syscfg.d \
./st_src/stm32f0xx_tim.d \
./st_src/stm32f0xx_usart.d 


# Each subdirectory must supply rules for building sources it contributes
st_src/%.o: ../st_src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -std=c99 -DSTM32F0XX -DUSE_STDPERIPH_DRIVER -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\my_inc" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\hardware" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\my_src" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\st_inc" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\st_src" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\startup" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\system" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer" -O0 -g3 -Wall -c -fmessage-length=0 -mthumb -mcpu=cortex-m0 -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

