/*
 * boxer_uart.h
 *
 *  Created on: 29 lip 2015
 *      Author: Doles
 */

#ifndef MY_INC_BOXER_COMMUNICATION_H_
#define MY_INC_BOXER_COMMUNICATION_H_

#include "stm32f0xx_gpio.h"
#include "stm32f0xx_usart.h"
#include "boxer_struct.h"
#include "stm32f0xx_rcc.h"
#include "stdint.h"

#define countof(a)             (uint8_t)(sizeof(a) / sizeof(*(a)))

typedef enum
{
	ATNEL_UNINITIALISE,
	ATNEL_SEND_3PLUS,
	ATNEL_RECV_A,
	ATNEL_SEND_A,
	ATNEL_INIT_DONE
}atnel_init_state_t;

typedef enum
{
	AT_NONE_REQ,
	AT_GMT_REQ,
	AT_ENTM_REQ,
}atnel_at_cmd_req_t;

typedef enum
{
	AT_NONE_RESP,
	AT_GMT_RESP,
	AT_ENTM_RESP
}atnel_at_cmd_resp_t;

typedef enum
{
	TRNSP_NONE_REQ,
	TRNSP_MEAS_DATA_REQ,
	TRNSP_CAL_DONE_REQ
}atnel_trnsp_cmd_req_t;

typedef enum
{
	TRNSP_NONE_RESP,
	TRNSP_MEAS_DATA_RESP,
	TRNSP_CAL_DONE_RESP
}atnel_trnsp_cmd_resp_t;

typedef enum
{
	ATNEL_MODE_UNKNOWN,
	ATNEL_MODE_TRANSPARENT,
	ATNEL_MODE_AT_CMD
}atnel_mode_t;

//extern atnel_init_state_t atnelInitProccess;
extern bool_t atnel_wait_change_mode;

extern atnel_at_cmd_resp_t atnel_AtCmdRespType;
extern atnel_at_cmd_req_t atnel_AtCmdReqType;

extern atnel_trnsp_cmd_resp_t atnel_TrCmdRespType;
extern atnel_trnsp_cmd_req_t atnel_TrCmdReqType;

//extern bool_t atnel_sendAtCmd;
//extern bool_t atnelAtCmdEnable;
//extern bool_t atnel_init_next_step;

void SerialPort_PutChar(char xSendChar);
void SerialPort_PutString(char * xString);

void SerialPort_Init(void);
void ReceiveSerial_Handler(void);
void TransmitSerial_Handler(void);
void NtpSendRequest(void);
ErrorStatus AtnelGetTime(char * xStrTime);
void AtnelSetTransparentMode(void);
#endif /* MY_INC_BOXER_COMMUNICATION_H_ */
