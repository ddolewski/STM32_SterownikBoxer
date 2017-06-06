/*
 * boxer_uart.c
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#include "boxer_communication.h"
#include "boxer_climate.h"
#include "boxer_ph.h"
#include "boxer_display.h"
#include "boxer_light.h"
#include "boxer_datastorage.h"
#include "boxer_string.h"
#include "misc.h"
#include "string_builder.h"
#include "fifo.h"
#include "pcf8563.h"

#define RX_PIN 		GPIO_Pin_3
#define TX_PIN		GPIO_Pin_2

#define NTP_REQTIME(H)			(3600*H)

static uint8_t response_timeout 	= 0;
bool_t atnel_wait_for_response 		= FALSE;

volatile static bool_t echoOff 		= FALSE;
static volatile int rxIdx 			= 0;
volatile bool_t rxRecvFlag 			= FALSE;

static volatile char RxBuffer[RX_BUFF_SIZE] = {0};
static volatile char TxBuffer[TX_BUFF_SIZE] = {0};
static volatile fifo_t tx_fifo 				= {NULL, 0, 0};

static atnel_init_state_t atnelInitProccess = ATNEL_UNINITIALISE;
atnel_mode_t atnel_Mode 					= ATNEL_MODE_TRANSPARENT;

atnel_at_cmd_resp_t atnel_AtCmdRespType 	= AT_NONE_RESP;
atnel_at_cmd_req_t atnel_AtCmdReqType 		= AT_NONE_REQ;

atnel_trnsp_cmd_resp_t atnel_TrCmdRespType 	= TRNSP_NONE_RESP;
atnel_trnsp_cmd_req_t atnel_TrCmdReqType 	= TRNSP_NONE_REQ;

bool_t atnel_wait_change_mode 		= FALSE;
static time_complex_t ntpTime 		= {0};

static bool_t ntpSyncProccess 		= FALSE;
static uint16_t ntpRequestTimer 	= 0; //pierwsze zapytanie o czas po 10s od wlaczenia
static uint8_t ntp_resp_wait 		= FALSE;
static uint8_t ntpResponseTimer 		= 0;
static uint8_t ntpRetryCounter 		= 0;

static uint8_t atnelWaitCounter 	= 0;
static uint8_t dataCounter 			= 0;
static bool_t atnelReset 			= FALSE;
static uint8_t resetMaxCounterValue = 5;

static void ClearRxBuff(void);

void Atnel_SetTransparentMode(void)
{
#ifdef NTP_DEBUG
	ntpRequestTimer = 3560;
#endif
	atnel_Mode = ATNEL_MODE_TRANSPARENT;
	atnelInitProccess = ATNEL_UNINITIALISE;

	ClearRxBuff();

	atnel_wait_change_mode = FALSE;
	ntpSyncProccess = FALSE;

	_printString("\r\ntryb transparentny wlaczony\n\r");
}

void SerialPort_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    fifo_init(&tx_fifo, (void *)TxBuffer, TX_BUFF_SIZE);

    USART_InitStructure.USART_BaudRate 				= 115200;
    USART_InitStructure.USART_WordLength 			= USART_WordLength_8b;
    USART_InitStructure.USART_StopBits 				= USART_StopBits_1;
    USART_InitStructure.USART_Parity 				= USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode 					= USART_Mode_Rx | USART_Mode_Tx;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

    GPIO_InitStructure.GPIO_Pin 	= TX_PIN | RX_PIN;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_ORE, ENABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

	USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	USART_ClearITPendingBit(USART2, USART_IT_TXE);
	USART_ClearITPendingBit(USART2, USART_IT_ORE);
	USART_ClearITPendingBit(USART2, USART_IT_IDLE);

	NVIC_SetPriority(USART2_IRQn, 1);
	NVIC_EnableIRQ(USART2_IRQn);

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
		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		if ( fifo_read(&tx_fifo, &txChar, 1) != 0) //zdejmij kolejny element z kolejki
		{
			USART_SendData( USART2, txChar );
		}
		else
		{
			fifo_flush(&tx_fifo);
			USART_ITConfig( USART2, USART_IT_TXE, DISABLE ); //kolejka jest pusta, wylaczamy przerwanie i przestajemy nadawac
		}
	}

	if ( USART_GetITStatus( USART2, USART_IT_RXNE ) != RESET )
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		rxChar = (char)USART_ReceiveData( USART2 );

		RxBuffer[rxIdx] = rxChar;
		rxIdx++;

		if (rxIdx == RX_BUFF_SIZE)
		{
			ClearRxBuff();
		}

		if (atnel_Mode == ATNEL_MODE_TRANSPARENT)
		{
			if (RxBuffer[0] != 'S')
			{
				if (RxBuffer[1] != 'T')
				{
					if (RxBuffer[2] != 'A')
					{
						ClearRxBuff();
					}
				}
			}
		}

		rxRecvFlag = TRUE;
	}

	if ( USART_GetITStatus( USART2, USART_IT_IDLE ) != RESET )
	{
		USART_ClearITPendingBit(USART2, USART_IT_IDLE);
		rxChar = (char)USART_ReceiveData( USART2 );
	}

	if ( USART_GetITStatus( USART2, USART_IT_ORE ) != RESET )
	{
		USART2->ICR |= USART_ICR_ORECF;
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		rxChar = (char)USART_ReceiveData( USART2 );
	}
}

void AtnelWiFi_Handler(void)
{
	if (atnel_Mode == ATNEL_MODE_TRANSPARENT)
	{
		dataCounter++;
		if (dataCounter == 3)
		{
			dataCounter = 0;
			atnel_TrCmdReqType = TRNSP_MEAS_DATA_REQ;
		}
	}

	if (atnel_wait_for_response == TRUE)
	{
		response_timeout++;
		if (response_timeout == 10)
		{
			atnel_wait_for_response = FALSE;
			response_timeout = 0;
			AtnelWifi_ResetModule();
			atnel_Mode = ATNEL_MODE_UNKNOWN;
			atnel_wait_change_mode = TRUE;
			atnelReset = TRUE;
		}
	}

	if (atnel_wait_change_mode == TRUE)
	{
		atnelWaitCounter++;

		if (atnelReset == TRUE)
		{
			resetMaxCounterValue = 60;
		}
		else
		{
			resetMaxCounterValue = 10;
		}

		if (atnelWaitCounter == resetMaxCounterValue)
		{
			atnelWaitCounter = 0;
			atnel_wait_change_mode = FALSE;
			atnelReset = FALSE;
			Atnel_SetTransparentMode();
		}
	}
}

void Ntp_Handler(void)
{
	// aktualizacja czasu wykonywana jest co godzine
	// pierwsza synchronizacja po uruchomieniu urzadzenia po 10s

	if (ntpSyncProccess == FALSE)
	{
		ntpRequestTimer++;

		if (ntpRequestTimer == NTP_REQTIME(12)) //todo wyslij zapytanie o czas co godzine
		{
			Ntp_SendRequest();
		}
	}

	if (ntp_resp_wait == TRUE)
	{
		ntpResponseTimer++;
		if (ntpResponseTimer == 5) //czekaj maksymalnie 3s na odpowiedz
		{
			_printString("timeout odpowiedzi ntp\r\n");
			ntpResponseTimer = 0;
			Ntp_SendRequest();

			ntpRetryCounter++;
			_printString("ponowne zapytanie o czas\r\n");
			if (ntpRetryCounter == 5) //do 5 prob potem wylacz komendy AT
			{
				atnel_AtCmdReqType = AT_ENTM_REQ;
				atnel_AtCmdRespType = AT_NONE_RESP;
				ntp_resp_wait = FALSE;
				_printString("BLAD! serwer nie odpowiedzial 5x\r\n");
			}
		}
	}
}

void Ntp_SendRequest(void)
{
	ntpSyncProccess = TRUE;
	ntpRequestTimer = 0;
	if (echoOff == FALSE)
	{
		atnel_AtCmdReqType = AT_E_REQ;
	}
	else
	{
		atnel_AtCmdReqType = AT_GMT_REQ;
	}

	atnel_Mode = ATNEL_MODE_AT_CMD;
}

void TransmitSerial_Handler(void)
{
	bool_t isDst = FALSE;

	switch (atnel_Mode)
	{
	case ATNEL_MODE_AT_CMD:

		if (atnelInitProccess == ATNEL_UNINITIALISE)
		{
			SerialPort_PutString("+++");
			atnelInitProccess = ATNEL_SEND_3PLUS;
			atnel_wait_for_response = TRUE;

			_printString("send '+++'\r\n");
		}
		else if (atnelInitProccess == ATNEL_RECV_A)
		{
			SerialPort_PutChar('a');
			atnelInitProccess = ATNEL_SEND_A;
			atnel_wait_for_response = TRUE;

			_printString("send 'a'\r\n");
		}
		else if (atnelInitProccess == ATNEL_INIT_DONE)
		{
			if (atnel_wait_change_mode == FALSE)
			{
				switch (atnel_AtCmdReqType)
				{
				case AT_GMT_REQ:
					isDst = timeCheckDstStatus(&xRtcFullDate);

					if (isDst == TRUE)
					{
						SerialPort_PutString("AT+GMT=2\r");
						_printString("\r\nsend AT+GMT=2\r\n");
					}
					else
					{
						SerialPort_PutString("AT+GMT=1\r");
						_printString("\r\nsend AT+GMT=1\r\n");
					}

					atnel_AtCmdReqType 	= AT_NONE_REQ;
					atnel_AtCmdRespType = AT_GMT_RESP;

					ntp_resp_wait = TRUE;
					break;

				case AT_ENTM_REQ:
					SerialPort_PutString("AT+ENTM\r");
					_printString("send AT+ENTM\r\n");

					atnel_AtCmdReqType 		= AT_NONE_REQ;
					atnel_AtCmdRespType 	= AT_ENTM_RESP;
					atnel_wait_for_response = TRUE;
					break;

				case AT_E_REQ:
					SerialPort_PutString("AT+E=off\r");
					_printString("send AT+E=off\r\n");

					atnel_AtCmdReqType 		= AT_NONE_REQ;
					atnel_AtCmdRespType 	= AT_E_RESP;
					atnel_wait_for_response = TRUE;
					break;

				default:
					atnel_AtCmdReqType = AT_NONE_REQ;
					break;
				}
			}
		}

		break;

	case ATNEL_MODE_TRANSPARENT:

		if (atnel_wait_change_mode == FALSE)
		{
			switch (atnel_TrCmdReqType)
			{
			case TRNSP_MEAS_DATA_REQ:
				__NOP();
				char DataToSend[TX_BUFF_SIZE] = {0};
				memset(DataToSend, 0, TX_BUFF_SIZE);
				PrepareUdpString(displayData.lux, displayData.humiditySHT2x, displayData.temp_middle_t, sensorTempUp.fTemp, sensorTempDown.fTemp, DataToSend);
				SerialPort_PutString(DataToSend);

	#ifdef SEND_TRANSMIT_FRAME
				_printString(DataToSend);
				_printString("\r\n");
	#endif

				atnel_TrCmdReqType = TRNSP_NONE_REQ;
			break;

			case TRNSP_CAL_DONE_REQ:
				SerialPort_PutString("STA CD END");
				atnel_TrCmdReqType = TRNSP_NONE_REQ;
				break;

			default:
				atnel_TrCmdReqType = TRNSP_NONE_REQ;
				break;
			}
		}

		break;

	default:
		break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiveSerial_Handler(void)
{
	if (rxRecvFlag == TRUE)
	{
		rxRecvFlag = FALSE;

		switch (atnel_Mode)
		{
		case ATNEL_MODE_AT_CMD:
			if (atnelInitProccess == ATNEL_SEND_3PLUS)
			{
				char * atnelResponse = strstr((void*)RxBuffer, "a");
				if (atnelResponse != NULL)
				{
					atnelInitProccess 		= ATNEL_RECV_A;
					atnel_wait_for_response = FALSE;
					response_timeout = 0;

					ClearRxBuff();

					_printString("recv 'a'\r\n");
				}
			}
			else if (atnelInitProccess == ATNEL_SEND_A)
			{
				char * atnelResponse = strstr((void*)RxBuffer, "+ok\r\n\r\n");

				if (atnelResponse != NULL)
				{
					atnelInitProccess 		= ATNEL_INIT_DONE;
					atnel_wait_for_response = FALSE;
					response_timeout = 0;

					ClearRxBuff();

					_printString("recv +ok\r\n");
				}
			}
			else if (atnelInitProccess == ATNEL_INIT_DONE)
			{
				switch (atnel_AtCmdRespType)
				{
				case AT_GMT_RESP:
					__NOP();
					char * at_gmt_response = strstr((void*)RxBuffer, "+ok=20");

					if (at_gmt_response != NULL)
					{
						uint8_t text_length = strlen(at_gmt_response);

						if (text_length >= 23)
						{
							_printString(at_gmt_response);
							_printString("\n\r");

							char strCopy[32] = {0};
							strcpy(strCopy, at_gmt_response);
							for (int i = 0; i < text_length; i++)
							{
								if (strCopy[i] == '-' || strCopy[i] == ' ' || strCopy[i] == ':' || strCopy[i] == '=' || strCopy[i] == '\r')
								{
									strCopy[i] = ';';
								}
							}

							char splitedString [8][8] = {{0},{0}};
							char * splitStr = 0;
							int i = 0;
							splitStr = strtok (strCopy, ";");
							strcpy(splitedString[i], splitStr);

							while (splitStr != NULL)
							{
								if (i < 8)
								{
									i++;
								}
								else
								{
									break;
								}

								splitStr = strtok (NULL, ";");
								strcpy(splitedString[i], splitStr);
							}

							static time_complex_t timeUtc = {2000, 1, 1, 1, 0, 0, 0};

							ntpTime.year  = atoi( splitedString[1] );
							ntpTime.month = atoi( splitedString[2] );
							ntpTime.mday  = atoi( splitedString[3] );
							ntpTime.hour  = atoi( splitedString[4] );
							ntpTime.min	  = atoi( splitedString[5] );
							ntpTime.sec   = atoi( splitedString[6] );

							timeLocalToUtcConv(&ntpTime, &timeUtc);

	#ifndef DEBUG_TERMINAL_USART
							PCF8563_WriteTime(&timeUtc, I2C1);
	#endif
							atnel_AtCmdReqType 	= AT_ENTM_REQ;
							atnel_AtCmdRespType = AT_NONE_RESP;
							ntp_resp_wait 		= FALSE;

							ClearRxBuff();
						}
					}

					break;

				case AT_ENTM_RESP:
					__NOP();
					char * at_entm_response = strstr((void*)RxBuffer, "+ok\r\n\r\n");

					if (at_entm_response != NULL)
					{
						_printString(at_entm_response);

						atnel_AtCmdReqType 		= AT_NONE_REQ;
						atnel_AtCmdRespType 	= AT_NONE_RESP;
						atnel_wait_change_mode 	= TRUE;
						atnel_wait_for_response = FALSE;
						response_timeout 		= 0;

						echoOff = FALSE;

						ClearRxBuff();
					}

					break;

				case AT_E_RESP:
					__NOP();
					char * at_echo_response = strstr((void*)RxBuffer, "+ok");

					if (at_echo_response != NULL)
					{
						_printString(at_echo_response);

						atnel_AtCmdReqType 		= AT_GMT_REQ;
						atnel_AtCmdRespType 	= AT_NONE_RESP;
						atnel_wait_change_mode 	= FALSE;

						echoOff = TRUE;

						ClearRxBuff();
					}

					break;

				default:
					atnel_AtCmdRespType = AT_NONE_RESP;
					break;
				}
			}
			break;
		case ATNEL_MODE_TRANSPARENT:

			if (atnel_wait_change_mode == FALSE)
			{
				char * startStrAddr  = strstr((void*)RxBuffer, "STA");

				if (startStrAddr != NULL)
				{
					char * endStrAddr    = strstr((void*)RxBuffer, "END");
					if (endStrAddr != NULL)
					{
						_printString((void*)RxBuffer);

						char ReceivedString [8][32] = {{0},{0}};
						char * splitStr = 0;
						int i = 0;
						splitStr = strtok ((void*)RxBuffer, " ");
						strcpy(ReceivedString[i], splitStr);

						while (splitStr != NULL)
						{
							if (i < RX_BUFF_SIZE)
							{
								i++;
							}
							else
							{
								break;
							}

							splitStr = strtok (NULL, " ");
							strcpy(ReceivedString[i], splitStr);
						}

						if (strcmp(ReceivedString[0], "STA") == 0)
						{
							if (strcmp(ReceivedString[1], "SL") == 0)
							{
								if (strcmp(ReceivedString[5], "END") == 0)
								{
									_printString("\r\n");

									ClearRxBuff();

									char timeOn   = atoi(ReceivedString[2]);
									char timeOff  = atoi(ReceivedString[3]);
									char newState = *ReceivedString[4];

									if (timeOn + timeOff == 24)
									{
										xLastTimeOnHour  = xLightControl.timeOnHours;
										xLastTimeOffHour = xLightControl.timeOffHours;
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

										if (timeOn == 24 || timeOff == 24)
										{
											xLightCounters.counterHours   = 0;
											xLightCounters.counterSeconds = 0;
										}

										FLASH_ClearLightState();
										FLASH_SaveConfiguration();
									}
								}
								else
								{
									ClearRxBuff();
								}
							}
							else if (strcmp(ReceivedString[1], "ST") == 0)
							{
								if (strcmp(ReceivedString[4], "END") == 0)
								{
									_printString("\r\n");

									ClearRxBuff();

									uint8_t temp = atoi( ReceivedString[2] );

									if (tempControl.userTemp >= TEMP_MIN && tempControl.userTemp <= TEMP_MAX)
									{
										tempControl.tempCtrlMode = TEMP_AUTO;
										tempControl.userTemp = temp;
										FLASH_SaveConfiguration();
									}
								}
								else
								{
									ClearRxBuff();
								}
							}
							else if (strcmp(ReceivedString[1], "SF") == 0)
							{
								if (strcmp(ReceivedString[4], "END") == 0)
								{
									_printString("\r\n");

									ClearRxBuff();

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
								else
								{
									ClearRxBuff();
								}
							}
							else if (strcmp(ReceivedString[1], "CP") == 0)
							{
								if (strcmp(ReceivedString[3], "END") == 0)
								{
									_printString("\r\n");

									ClearRxBuff();

									uint8_t probeType 					= atoi( ReceivedString[2] );
									calibrateFlags.probeType 			= (probe_type_t)probeType;
									calibrateFlags.processActive 		= TRUE;
									calibrateFlags.waitForNextBuffer 	= TRUE;
									adcAverageMeasCounter = 0;
									GLCD_ClearScreen();
								}
								else
								{
									ClearRxBuff();
								}
							}
							else if (strcmp(ReceivedString[1], "SI") == 0)
							{
								if (strcmp(ReceivedString[5], "END") == 0)
								{
									_printString("\r\n");

									ClearRxBuff();
		//							irrigationControl.mode = (irrigation_mode_t)ReceivedString[2];
		//							irrigationControl.frequency = (uint8_t)ReceivedString[3];
		//							irrigationControl.water = (uint8_t)ReceivedString[4];
		//
		//							FLASH_SaveConfiguration();
								}
								else
								{
									ClearRxBuff();
								}
							}
							else if (strcmp(ReceivedString[1], "R") == 0)
							{
								_printString("Reset recv\n\r");
								if (strcmp(ReceivedString[2], "END") == 0)
								{
									_printString("\r\n");

									FLASH_SaveLightCounters();
									ClearRxBuff();
									MISC_ResetARM();
								}
								else
								{
									ClearRxBuff();
								}
							}
							else if (strcmp(ReceivedString[1], "DEF") == 0)
							{
								_printString("DEF recv\n\r");
								if (strcmp(ReceivedString[2], "SETT") == 0)
								{
									if (strcmp(ReceivedString[3], "END") == 0)
									{
										_printString("\r\n");

										ClearRxBuff();
										FLASH_RestoreDefaultConfig();
										FLASH_ClearLightState();
										MISC_ResetARM();
									}
									else
									{
										ClearRxBuff();
									}
								}
								else
								{
									ClearRxBuff();
								}
							}
							else
							{
								ClearRxBuff();
							}
						}
						else
						{
							ClearRxBuff();
						}
					}
				}
			}

			break;

		default:
			break;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void ClearRxBuff(void)
{
	rxIdx = 0;
	memset((void*)RxBuffer, 0, RX_BUFF_SIZE);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AtnelWifi_ResetModule(void)
{
	GPIOx_ResetPin(WIFI_RST_PORT, WIFI_RST_PIN);
	systimeDelayMs(5000);
	GPIOx_SetPin(WIFI_RST_PORT, WIFI_RST_PIN);
}
