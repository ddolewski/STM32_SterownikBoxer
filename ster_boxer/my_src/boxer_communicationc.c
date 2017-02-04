/*
 * boxer_uart.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_communication.h"
#include "boxer_struct.h"
#include "misc.h"
#include "string_builder.h"
#include "fifo.h"

static void AtnelResetModule(void);
static char recvstr[RX_BUFF_SIZE] = {0};

static volatile char RxBuffer[RX_BUFF_SIZE];
static volatile fifo_t rx_fifo;

static volatile char TxBuffer[TX_BUFF_SIZE];
static volatile fifo_t tx_fifo;

atnel_init_state_t atnelInitProccess = ATNEL_UNINITIALISE;
bool_t atnelAtCmdEnable = FALSE;

void SerialPort_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    fifo_init(&rx_fifo, (void *)RxBuffer, RX_BUFF_SIZE);
    fifo_init(&tx_fifo, (void *)TxBuffer, TX_BUFF_SIZE);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    /* Enable GPIO clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    /* Enable USART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* Connect PXx to USARTx_Tx */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);

    /* Connect PXx to USARTx_Rx */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

    /* Configure USART Tx, Rx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART configuration */
    USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	NVIC_SetPriority(USART2_IRQn, 3);
	NVIC_EnableIRQ(USART2_IRQn);

    /* Enable USART */
    USART_Cmd(USART2, ENABLE);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SerialPort_PutChar(char xSendChar)
{
	fifo_write(&tx_fifo, &xSendChar, 1);
	USART_ITConfig( USART2, USART_IT_TXE, ENABLE );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SerialPort_PutString(char * xString)
{
	//memset(tx_fifo.buf, 0, sizeof(&tx_fifo.buf));
	fifo_write(&tx_fifo, xString, strlen(xString));
	USART_ITConfig( USART2, USART_IT_TXE, ENABLE );
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void USART2_IRQHandler(void)
{
	char txChar = 0;
	char rxChar = 0;

	if ( USART_GetITStatus( USART2, USART_IT_TXE ) != RESET )
	{
		if ( fifo_read(&tx_fifo, &txChar, 1) != 0) //zdejmij kolejny element z kolejki
		{
			USART_SendData( USART2, txChar );
		}
		else
		{
			USART_ITConfig( USART2, USART_IT_TXE, DISABLE ); //kolejka jest pusta, wylaczamy przerwanie i przestajemy nadawac
		}

		USART_ClearITPendingBit(USART2, USART_IT_TXE);
	}

	if ( USART_GetITStatus( USART2, USART_IT_RXNE ) != RESET )
	{
		rxChar = (char)USART_ReceiveData( USART2 );
		fifo_write(&rx_fifo, &rxChar, 1);
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
}

void TransmitSerial_Handler(void)
{
	if (ntpSyncProccess == TRUE)
	{
		if (atnelInitProccess == ATNEL_UNINITIALISE)
		{
			SerialPort_PutString("+++");
			atnelInitProccess = ATNEL_SEND_3PLUS;
		}
		else if (atnelInitProccess == ATNEL_RECV_A)
		{
			SerialPort_PutChar('a');
			atnelInitProccess = ATNEL_SEND_A;
		}
	}
	else
	{
    	if (flagsGlobal.udpSendMsg == TRUE)
    	{
    		char DataToSend[TX_BUFF_SIZE] = {0};
    		PrepareUdpString(displayData.lux, displayData.humiditySHT2x, displayData.tempSHT2x, ds18b20_1.fTemp, ds18b20_2.fTemp, DataToSend);
//    		USARTx_SendString(USART_COMM, (uint8_t*)DataToSend);
    		SerialPort_PutString(DataToSend);
    		flagsGlobal.udpSendMsg = FALSE;
    	}
    	else
    	{
    		if (calibrateFlags.calibrateDone == TRUE)
    		{
//    			USARTx_SendString(USART_COMM, (uint8_t*)"STA CD END"); //calibrate done
    			SerialPort_PutString((uint8_t*)"STA CD END");
    			calibrateFlags.calibrateDone = FALSE;
    		}
    	}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiveSerial_Handler(void)
{
	char recvData = 0;

	if (fifo_read(&rx_fifo, &recvData, 1) > 0) //zdejmij kolejny element z kolejki
	{
		if (ntpSyncProccess == TRUE)
		{
			if (atnelInitProccess == ATNEL_SEND_3PLUS)
			{
				if (recvData == 'a')
				{
					atnelInitProccess = ATNEL_RECV_A;
				}
			}
			else if (atnelInitProccess == ATNEL_SEND_A)
			{
				append(recvstr, recvData);
				char * atnelResponse = strstr(recvstr, "+ok");
				if (atnelResponse != NULL)
				{
					atnelInitProccess = ATNEL_RECV_OK;
				}
			}
		}
		else
		{
			append(recvstr, recvData);
			char * startStrAddr  = strstr(recvstr, "STA");
			char * endStrAddr    = strstr(recvstr, "END");

	//		DEBUG_SendString(recvstr);

			if ((startStrAddr != NULL) && (endStrAddr != NULL))
			{
				char ReceivedString [8][32] = {0};
				char * pch = 0;
				int i = 0;
				pch = strtok (recvstr, " ");
				strcpy(ReceivedString[i], pch);

	//			DEBUG_SendString(recvstr);

				while (pch != NULL)
				{
					if (i < 128)
					{
						i++;
					}
					else
					{
						break;
					}

					pch = strtok (NULL, " ");
					strcpy(ReceivedString[i], pch);
				}

				if (strcmp(ReceivedString[0], "STA") == 0) //start frame preffix
				{
					if (strcmp(ReceivedString[1], "SL") == 0) //set lamp frame command
					{
						if (strcmp(ReceivedString[5], "END") == 0) //end frame suffix
						{
							memset(recvstr, 0, RX_BUFF_SIZE);

							char timeOn   = atoi(ReceivedString[2]);
							char timeOff  = atoi(ReceivedString[3]);
							char newState = *ReceivedString[4];

							if (timeOn + timeOff == 24)
							{
								lastTimeOnHour  = xLightControl.timeOnHours;
								lastTimeOffHour = xLightControl.timeOffHours;
								xLightControl.timeOnHours  = (uint8_t)timeOn;
								xLightControl.timeOffHours = (uint8_t)timeOff;

								light_state_t tempLightState = xLightControl.lightingState;
								xLightControl.lightingState  = newState;

								// jesli nowy stan lampy jest inny od poprzedniego to skasuj liczniki (nowy stan)
								if (tempLightState != xLightControl.lightingState)
								{
									xLightCounters.counterHours   = 0;
									xLightCounters.counterSeconds = 0;
								}

								FLASH_SaveConfiguration();
							}
						}
					}
					else if (strcmp(ReceivedString[1], "ST") == 0) //set temp frame command
					{
						if (strcmp(ReceivedString[3], "END") == 0) //end frame suffix
						{
							memset(recvstr, 0, RX_BUFF_SIZE);

							uint8_t temp = atoi( ReceivedString[2] );

							tempControl.tempCtrlMode = TEMP_AUTO;
							if (tempControl.userTemp >= TEMP_MIN && tempControl.userTemp <= TEMP_MAX)
							{
								tempControl.userTemp = temp;
								FLASH_SaveConfiguration();
							}
						}
					}
					else if (strcmp(ReceivedString[1], "SF") == 0) //set fans frame command
					{
						if (strcmp(ReceivedString[4], "END") == 0) //end frame suffix
						{
							memset(recvstr, 0, RX_BUFF_SIZE);
							uint8_t isProper = TRUE;
							uint8_t fanPull = atoi( ReceivedString[2] );
							uint8_t fanPush = atoi( ReceivedString[3] );

							tempControl.tempCtrlMode = TEMP_MANUAL;

							if ( (fanPull >= PWM_MIN_PERCENT && fanPull <= PWM_MAX_PERCENT) &&
								 (fanPush >= PWM_MIN_PERCENT && fanPush <= PWM_MAX_PERCENT) )
							{
								tempControl.fanPull = fanPull;
								tempControl.fanPush = fanPush;
							}
							else
							{
								isProper = FALSE;
							}

							if (isProper == TRUE)
							{
								FLASH_SaveConfiguration();
							}
						}
					}
					else if (strcmp(ReceivedString[1], "CP") == 0) //set temp frame command
					{
						if (strcmp(ReceivedString[3], "END") == 0) //end frame suffix
						{
							memset(recvstr, 0, RX_BUFF_SIZE);

							calibrateFlags.probeType = (probe_type_t)ReceivedString[2];
							calibrateFlags.processActive = 1;
							calibrateFlags.turnOnBuzzer = 1;
							calibrateFlags.toggleBuzzerState = 1;
						}
					}
					else if (strcmp(ReceivedString[1], "SI") == 0) //set temp frame command
					{
						if (strcmp(ReceivedString[5], "END") == 0) //end frame suffix
						{
							memset(recvstr, 0, RX_BUFF_SIZE);

//							irrigationControl.mode = (irrigation_mode_t)ReceivedString[2];
//							irrigationControl.frequency = (uint8_t)ReceivedString[3];
//							irrigationControl.water = (uint8_t)ReceivedString[4];
//
//							FLASH_SaveConfiguration();
						}
					}
					else if (strcmp(ReceivedString[1], "R") == 0) //reset frame command
					{
						if (strcmp(ReceivedString[2], "END") == 0) //end frame suffix
						{
							MISC_ResetARM();
						}
					}
					else if (strcmp(ReceivedString[1], "DEF") == 0) //default first command word
					{
						if (strcmp(ReceivedString[2], "SETT") == 0) //settings second command word
						{
							if (strcmp(ReceivedString[3], "END") == 0) //end frame suffix
							{
								FLASH_RestoreDefaultConfig();
								FLASH_ClearLightState();
								MISC_ResetARM();
							}
						}
					}
				}
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void AtnelResetModule(void)
{
	GPIOx_ResetPin(WIFI_RST_PORT, WIFI_RST_PIN);
	systimeDelayMs(3500);
	GPIOx_SetPin(WIFI_RST_PORT, WIFI_RST_PIN);
}

ErrorStatus AtnelGetTime(char * xStrTime)
{
	ErrorStatus err = SUCCESS;


	return err;
}
