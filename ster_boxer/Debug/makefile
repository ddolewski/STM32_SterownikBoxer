################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include system/subdir.mk
-include startup/subdir.mk
-include st_src/subdir.mk
-include my_src/subdir.mk
-include hardware/TSL2561/subdir.mk
-include hardware/SHT2x/subdir.mk
-include hardware/PCF8563/subdir.mk
-include hardware/LCD_KS0108/subdir.mk
-include hardware/DS18B20/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: ster_boxer.elf

# Tool invocations
ster_boxer.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	arm-none-eabi-gcc -T "C:/Procki projekty/Git_Workspace/Eclipse_Workspace/STM32F051_monitor_uprawy/ster_boxer/stm32_flash.ld" -mthumb -mcpu=cortex-m0 -Wl,--gc-sections -o "ster_boxer.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) ster_boxer.elf
	-@echo ' '

post-build:
	-@echo 'Creating hex and bin file...'
	-arm-none-eabi-objcopy -O binary "ster_boxer.elf" "ster_boxer.bin"&arm-none-eabi-objcopy -O ihex "ster_boxer.elf" "ster_boxer.hex"&arm-none-eabi-size ster_boxer.elf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
