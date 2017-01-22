################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../my_src/boxer_climate.c \
../my_src/boxer_communicationc.c \
../my_src/boxer_datastorage.c \
../my_src/boxer_display.c \
../my_src/boxer_irrigation.c \
../my_src/boxer_ph.c \
../my_src/boxer_string.c \
../my_src/boxer_timers.c \
../my_src/debug.c \
../my_src/misc.c \
../my_src/string_builder.c \
../my_src/system_exti.c \
../my_src/system_flash.c \
../my_src/system_gpio.c \
../my_src/system_rcc.c \
../my_src/system_timer.c \
../my_src/systime.c \
../my_src/timestamp.c 

OBJS += \
./my_src/boxer_climate.o \
./my_src/boxer_communicationc.o \
./my_src/boxer_datastorage.o \
./my_src/boxer_display.o \
./my_src/boxer_irrigation.o \
./my_src/boxer_ph.o \
./my_src/boxer_string.o \
./my_src/boxer_timers.o \
./my_src/debug.o \
./my_src/misc.o \
./my_src/string_builder.o \
./my_src/system_exti.o \
./my_src/system_flash.o \
./my_src/system_gpio.o \
./my_src/system_rcc.o \
./my_src/system_timer.o \
./my_src/systime.o \
./my_src/timestamp.o 

C_DEPS += \
./my_src/boxer_climate.d \
./my_src/boxer_communicationc.d \
./my_src/boxer_datastorage.d \
./my_src/boxer_display.d \
./my_src/boxer_irrigation.d \
./my_src/boxer_ph.d \
./my_src/boxer_string.d \
./my_src/boxer_timers.d \
./my_src/debug.d \
./my_src/misc.d \
./my_src/string_builder.d \
./my_src/system_exti.d \
./my_src/system_flash.d \
./my_src/system_gpio.d \
./my_src/system_rcc.d \
./my_src/system_timer.d \
./my_src/systime.d \
./my_src/timestamp.d 


# Each subdirectory must supply rules for building sources it contributes
my_src/%.o: ../my_src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -std=c99 -DSTM32F0XX -DUSE_STDPERIPH_DRIVER -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\my_inc" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\hardware" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\my_src" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\st_inc" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\st_src" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\startup" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\system" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer" -O0 -g3 -Wall -c -fmessage-length=0 -mthumb -mcpu=cortex-m0 -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

