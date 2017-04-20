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

#define RX_PIN 	GPIO_Pin_3
#define TX_PIN	GPIO_Pin_2



volatile char RxBuffer[RX_BUFF_SIZE] = {0};
volatile fifo_t rx_fifo;

volatile char TxBuffer[TX_BUFF_SIZE] = {0};
volatile fifo_t tx_fifo;

static atnel_init_state_t atnelInitProccess = ATNEL_UNINITIALISE;
atnel_mode_t atnel_Mode = ATNEL_MODE_TRANSPARENT;//ATNEL_MODE_UNKNOWN;

atnel_at_cmd_resp_t atnel_AtCmdRespType = AT_NONE_RESP;
atnel_at_cmd_req_t atnel_AtCmdReqType = AT_NONE_REQ;

atnel_trnsp_cmd_resp_t atnel_TrCmdRespType = TRNSP_NONE_RESP;
atnel_trnsp_cmd_req_t atnel_TrCmdReqType = TRNSP_NONE_REQ;

bool_t atnel_wait_change_mode = FALSE;

static time_complex_t ntpTime = {0};
static char DataToSend[TX_BUFF_SIZE] = {0};
volatile char recvstr[RX_BUFF_SIZE] = {0};

static bool_t ntpSyncProccess = FALSE;
static uint16_t ntpRequestTimer = 0; //pierwsze zapytanie o czas po 10s od wlaczenia
static uint8_t ntp_resp_wait = FALSE;
static uint8_t ntpRetryTimer = 0;
static uint8_t ntpRetryCounter = 0;

void Atnel_SetTransparentMode(void)
{
//#ifdef NTP_DEBUG
//	ntpRequestTimer = 3560;
//#endif
	atnel_Mode = ATNEL_MODE_TRANSPARENT;
	atnelInitProccess = ATNEL_UNINITIALISE;
//	fifo_flush(&rx_fifo);
	memset(recvstr, 0, RX_BUFF_SIZE);
	atnel_wait_change_mode = FALSE;
	ntpSyncProccess = FALSE;
	DEBUG_SendString("\r\ntryb transparentny wlaczony\n\r");
}

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

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

    GPIO_InitStructure.GPIO_Pin = TX_PIN | RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	USART_ClearITPendingBit(USART2, USART_IT_TXE);

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
			USART_ITConfig( USART2, USART_IT_TXE, DISABLE ); //kolejka jest pusta, wylaczamy przerwanie i przestajemy nadawac
		}
	}

	if ( USART_GetITStatus( USART2, USART_IT_RXNE ) != RESET )
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		rxChar = (char)USART_ReceiveData( USART2 );
		fifo_write(&rx_fifo, &rxChar, 1);
	}
}

void Ntp_Handler(void)
{
	// aktualizacja czasu wykonywana jest co godzine
	// pierwsza synchronizacja po uruchomieniu urzadzenia po 10s

	if (ntpSyncProccess == FALSE)
	{
		ntpRequestTimer++;

		if (ntpRequestTimer == 3600) //wyslij zapytanie o czas co godzine
		{
			Ntp_SendRequest();
		}
	}

	if (ntp_resp_wait == TRUE)
	{
		ntpRetryTimer++;
		if (ntpRetryTimer == 5) //czekaj maksymalnie 5s na odpowiedz
		{
			DEBUG_SendString("timeout odpowiedzi ntp\r\n");
			ntpRetryTimer = 0;
			Ntp_SendRequest();

			ntpRetryCounter++;
			DEBUG_SendString("ponowne zapytanie o czas\r\n");
			if (ntpRetryCounter == 3) //do 3 prob potem wylacz komendy AT
			{
				atnel_AtCmdReqType = AT_ENTM_REQ;
				atnel_AtCmdRespType = AT_NONE_RESP;
				ntp_resp_wait = FALSE;
				DEBUG_SendString("BLAD! serwer nie odpowiedzial 3x\r\n");
			}
		}
	}
}

void Ntp_SendRequest(void)
{
	ntpSyncProccess = TRUE;
	ntpRequestTimer = 0;
//	DEBUG_SendString("wysylam zapytanie o czas\r\n");
	atnel_AtCmdReqType = AT_GMT_REQ;
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

			DEBUG_SendString("send '+++'\r\n");
		}
		else if (atnelInitProccess == ATNEL_RECV_A)
		{
			SerialPort_PutChar('a');
			atnelInitProccess = ATNEL_SEND_A;

			DEBUG_SendString("send 'a'\r\n");
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
						DEBUG_SendString("send AT+GMT=2\r\n");
					}
					else
					{
						SerialPort_PutString("AT+GMT=1\r");
						DEBUG_SendString("send AT+GMT=1\r\n");
					}

					atnel_AtCmdReqType = AT_NONE_REQ;
					atnel_AtCmdRespType = AT_GMT_RESP;

					ntp_resp_wait = TRUE;
					break;

				case AT_ENTM_REQ:
					SerialPort_PutString("AT+ENTM\r");
					DEBUG_SendString("send AT+ENTM\r\n");

					atnel_AtCmdReqType = AT_NONE_REQ;
					atnel_AtCmdRespType = AT_ENTM_RESP;
					break;
				default:
					atnel_AtCmdReqType = AT_NONE_REQ;
					break;
				}
			}
		}

		break;

	case ATNEL_MODE_TRANSPARENT:

		switch (atnel_TrCmdReqType)
		{
		case TRNSP_MEAS_DATA_REQ:
		PrepareUdpString(displayData.lux, displayData.humiditySHT2x, displayData.tempSHT2x, ds18b20_1.fTemp, ds18b20_2.fTemp, DataToSend);
		SerialPort_PutString(DataToSend);

//		DEBUG_SendString(DataToSend);
//		DEBUG_SendString("\n\r");

		atnel_TrCmdReqType = TRNSP_NONE_REQ;
		break;

		case TRNSP_CAL_DONE_REQ:
			SerialPort_PutString((uint8_t*)"STA CD END");
			atnel_TrCmdReqType = TRNSP_NONE_REQ;
			calibrateFlags.calibrateDone = FALSE;
			break;

		default:
			atnel_TrCmdReqType = TRNSP_NONE_REQ;
			break;
		}
		break;
	default:
//		DEBUG_SendString("Nieznany tryb atnel wifi\n\r");
		break;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ReceiveSerial_Handler(void)
{
	char recvChar = 0;

	if (fifo_read(&rx_fifo, &recvChar, 1) > 0) //zdejmij kolejny element z kolejki
	{
		switch (atnel_Mode)
		{
		case ATNEL_MODE_AT_CMD:
			if (atnelInitProccess == ATNEL_SEND_3PLUS)
			{
				if (recvChar == 'a')
				{
					atnelInitProccess = ATNEL_RECV_A;
					DEBUG_SendString("recv 'a'\r\n");

					memset(recvstr, 0, RX_BUFF_SIZE);
					fifo_flush(&rx_fifo);
				}
			}
			else if (atnelInitProccess == ATNEL_SEND_A)
			{
				append(recvstr, recvChar);
				char * atnelResponse = strstr(recvstr, "+ok\r\n\r\n"); //\r\n\r\n

				if (atnelResponse != NULL)
				{
					atnelInitProccess = ATNEL_INIT_DONE;
					DEBUG_SendString("recv +ok\r\n");
					memset(recvstr, 0, RX_BUFF_SIZE);
					fifo_flush(&rx_fifo);
				}
			}
			else if (atnelInitProccess == ATNEL_INIT_DONE)
			{
				switch (atnel_AtCmdRespType)
				{
				case AT_GMT_RESP:
					append(recvstr, recvChar);
					char * at_gmt_response = strstr(recvstr, "+ok=20"); //first digits of year - constans ;)

					if (at_gmt_response != NULL)
					{
						uint8_t text_length = strlen(at_gmt_response);

						if (text_length >= 23)
						{
							DEBUG_SendString(at_gmt_response);
							DEBUG_SendString("\n\r");

							char strCopy[32] = {0};
							strcpy(strCopy, at_gmt_response);
							for (int i = 0; i < text_length; i++)
							{
								if (strCopy[i] == '-' || strCopy[i] == ' ' || strCopy[i] == ':' || strCopy[i] == '=' || strCopy[i] == '\r')
								{
									strCopy[i] = ';';
								}
							}

							char splitedString [8][8] = {0};
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

//								DEBUG_SendString(splitStr);
//								DEBUG_SendString("\n\r");
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
							atnel_AtCmdReqType = AT_ENTM_REQ;
							atnel_AtCmdRespType = AT_NONE_RESP;
							memset(recvstr, 0, RX_BUFF_SIZE);
							fifo_flush(&rx_fifo);
							ntp_resp_wait = FALSE;
						}
					}

					break;

				case AT_ENTM_RESP:
					append(recvstr, recvChar);
					char * at_entm_response = strstr(recvstr, "+ok\r\n\r\n");

					if (at_entm_response != NULL)
					{
						DEBUG_SendString(at_entm_response);

						atnel_AtCmdReqType = AT_NONE_REQ;
						atnel_AtCmdRespType = AT_NONE_RESP;
						atnel_wait_change_mode = TRUE;
						memset(recvstr, 0, RX_BUFF_SIZE);
						fifo_flush(&rx_fifo);
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
				append(recvstr, recvChar);
				char * startStrAddr  = strstr(recvstr, "STA");
				DEBUG_SendString(recvstr);
				DEBUG_SendString("\r\n");

//				DEBUG_SendString(rx_fifo.buf);
//				DEBUG_SendString("\r\n");

				if (startStrAddr != NULL)
				{
					char * endStrAddr    = strstr(recvstr, "END");
					if (endStrAddr != NULL)
					{
						char ReceivedString [8][32] = {0};
						char * splitStr = 0;
						int i = 0;
						splitStr = strtok (recvstr, " ");
						strcpy(ReceivedString[i], splitStr);

						DEBUG_SendString(recvstr);

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

						if (strcmp(ReceivedString[0], "STA") == 0) //start frame preffix
						{
							DEBUG_SendString("STA recv\n\r");
							if (strcmp(ReceivedString[1], "SL") == 0) //set lamp frame command
							{
								DEBUG_SendString("SL recv\n\r");
								if (strcmp(ReceivedString[5], "END") == 0) //end frame suffix
								{
									DEBUG_SendString("END recv\n\r");
									memset(recvstr, 0, RX_BUFF_SIZE);
									fifo_flush(&rx_fifo);

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

										FLASH_SaveConfiguration();
									}
								}
							}
							else if (strcmp(ReceivedString[1], "ST") == 0) //set temp frame command
							{
								DEBUG_SendString("ST recv\n\r");
								if (strcmp(ReceivedString[3], "END") == 0) //end frame suffix
								{
									memset(recvstr, 0, RX_BUFF_SIZE);
									fifo_flush(&rx_fifo);
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
								DEBUG_SendString("SF recv\n\r");
								if (strcmp(ReceivedString[4], "END") == 0) //end frame suffix
								{
									memset(recvstr, 0, RX_BUFF_SIZE);
									fifo_flush(&rx_fifo);
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
							else if (strcmp(ReceivedString[1], "CP") == 0) //set calibrate probe command
							{
								DEBUG_SendString("CP recv\n\r");
								if (strcmp(ReceivedString[3], "END") == 0) //end frame suffix
								{
									memset(recvstr, 0, RX_BUFF_SIZE);
									fifo_flush(&rx_fifo);
									uint8_t probeType = atoi( ReceivedString[2] );
									calibrateFlags.probeType = (probe_type_t)probeType;
									calibrateFlags.processActive = TRUE;
									calibrateFlags.turnOnBuzzer = TRUE;
									calibrateFlags.toggleBuzzerState = TRUE;
								}
							}
							else if (strcmp(ReceivedString[1], "SI") == 0) //set temp frame command
							{
								DEBUG_SendString("SI recv\n\r");
								if (strcmp(ReceivedString[5], "END") == 0) //end frame suffix
								{
									memset(recvstr, 0, RX_BUFF_SIZE);
									fifo_flush(&rx_fifo);
		//							irrigationControl.mode = (irrigation_mode_t)ReceivedString[2];
		//							irrigationControl.frequency = (uint8_t)ReceivedString[3];
		//							irrigationControl.water = (uint8_t)ReceivedString[4];
		//
		//							FLASH_SaveConfiguration();
								}
							}
							else if (strcmp(ReceivedString[1], "R") == 0) //reset frame command
							{
								DEBUG_SendString("Reset recv\n\r");
								if (strcmp(ReceivedString[2], "END") == 0) //end frame suffix
								{
									memset(recvstr, 0, RX_BUFF_SIZE);
									fifo_flush(&rx_fifo);
									MISC_ResetARM();
								}
							}
							else if (strcmp(ReceivedString[1], "DEF") == 0) //default first command word
							{
								DEBUG_SendString("DEF recv\n\r");
								if (strcmp(ReceivedString[2], "SETT") == 0) //settings second command word
								{
									if (strcmp(ReceivedString[3], "END") == 0) //end frame suffix
									{
										memset(recvstr, 0, RX_BUFF_SIZE);
										fifo_flush(&rx_fifo);
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

			break;

		default:
//			atnel_Mode = ATNEL_MODE_UNKNOWN;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

