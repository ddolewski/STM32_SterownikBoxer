################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../main.c 

OBJS += \
./main.o 

C_DEPS += \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -std=c99 -DSTM32F0XX -DUSE_STDPERIPH_DRIVER -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\my_inc" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\hardware" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\my_src" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\st_inc" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\st_src" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\startup" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\system" -I"C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer" -O0 -g3 -Wall -c -fmessage-length=0 -mthumb -mcpu=cortex-m0 -fdata-sections -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


