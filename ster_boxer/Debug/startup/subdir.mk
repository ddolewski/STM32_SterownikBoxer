################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../startup/startup_stm32f0xx.S 

OBJS += \
./startup/startup_stm32f0xx.o 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Assembler'
	arm-none-eabi-as -mthumb -mcpu=cortex-m0 -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\hardware" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\my_inc" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\my_src" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\st_inc" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\st_src" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\startup" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\system" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

