set MY_PATH="C:\Eclipse_Workspace\ster_boxer\ster_boxer\Debug\ster_boxer.hex"

START "" /w /b "C:\Program Files (x86)\STMicroelectronics\STM32 ST-LINK Utility\ST-LINK Utility\"ST-LINK_CLI.exe -c SWD -P %MY_PATH% -Rst -Run