set MY_PATH="C:\Procki projekty\Git_Workspace\Eclipse_Workspace\STM32F051_monitor_uprawy\ster_boxer\Debug\ster_boxer.hex"

START "" /w /b "C:\Procki projekty\STM32\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\"ST-LINK_CLI.exe -c SWD -P "%MY_PATH%" -Rst -Run