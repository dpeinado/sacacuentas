/*
 * device.h
 *
 *  Created on: 31 ene. 2018
 *      Author: dpeinado
 */

#ifndef APPLICATION_DEVICE_H_
#define APPLICATION_DEVICE_H_

/* Crystal frequency, in hertz. */
#define XTAL_FREQ_HZ 38400000

#define ADDR_BYTE_SIZE           (2)
#define STANDARD_FRAME_SIZE         20
#define TAG_FINAL_MSG_LEN           9              // FunctionCode(1), Range Num (1), Poll_TxTime(5),
#define ANC_POLL_MSG_LEN			1
#define ANC_GENERAL_BEACON_LEN		2
#define FRAME_CONTROL_BYTES         2
#define FRAME_SEQ_NUM_BYTES         1
#define FRAME_PANID                 2
#define FRAME_CRC					2
#define FRAME_SOURCE_ADDRESS        (ADDR_BYTE_SIZE)
#define FRAME_DEST_ADDRESS          (ADDR_BYTE_SIZE)
#define FRAME_INDX_FRAME_CONTROL	0
#define FRAME_INDX_SEQ_NUMBER		2
#define FRAME_INDX_PANID			3
#define FRAME_INDX_DEST_ADDRESS		5
#define FRAME_INDX_SOURCE_ADDRESS	7
#define FRAME_INDX_MESSAGE_DATA		9

#define FRAME_INDX_TAG_NUMBER		10
#define RESP_MSG_POLL_RX_TS_IDX 	10
#define RESP_MSG_RESP_TX_TS_IDX 	14
#define RESP_MSG_TS_LEN 			4

#define TWR_POLL_CMD		0xE0
#define TWR_ANSW_CMD		0xE1
#define TWR_SYNC_BEACON		0xF1
#define TWR_SYNC_GENBEACON	0xFF

/* Speed of light in air, in metres per second. */
#define SPEED_OF_LIGHT 299702547
#define MS_FROM_BEACON_TO_RANGE	2000

#define MAX_TAGS 75
typedef struct {
	uint8 ntags;
	uint8 test_tag;
	uint8 max_fallos_desenlaza;
	float max_distancia;
} confData_t;

confData_t conf_data;

typedef struct
{
    uint8 frameCtrl[2];
    uint8 seqNum;
    uint8 panID[2];
    uint8 destAddr[ADDR_BYTE_SIZE];
    uint8 sourceAddr[ADDR_BYTE_SIZE];
    uint8 messageData[TAG_FINAL_MSG_LEN] ;
    uint8 fcs[2] ;
} frameTagFinal_t ;
frameTagFinal_t answerFrame;

typedef struct
{
	uint8 frameCtrl[2];
	uint8 seqNum;
	uint8 panID[2];
	uint8 destAddr[ADDR_BYTE_SIZE];
    uint8 sourceAddr[ADDR_BYTE_SIZE];
    uint8 messageData;
    uint8 fcs[2] ;
} frameAncPoll_t;

frameAncPoll_t rangeFrame;

typedef struct
{
	uint8 frameCtrl[2];
	uint8 seqNum;
	uint8 panID[2];
	uint8 destAddr[ADDR_BYTE_SIZE];
    uint8 sourceAddr[ADDR_BYTE_SIZE];
    uint8 messageData[ANC_GENERAL_BEACON_LEN] ;
    uint8 fcs[2] ;
} frameAncBeacon_t;
frameAncBeacon_t beaconFrame;

typedef struct
{
	uint8 panID[2];
	uint8 address[2];
	uint8 frame_seq;
	bool  enlazado;
	uint8 fallos;
	float distance;
	uint nTags;
} tagDataAnc_t;

typedef struct
{
	uint8 panID[2];
	uint8 address[2];
	uint8 frame_seq;
	bool  enlazado;
	uint8 fallos;
	uint nTags;
	uint8 beaconNumber;
	uint32 timeRXBeacon;
	bool enlaceBeacon;
	uint32 framePeriod;
} tagData_t;



typedef struct
{
	uint8 panID[2];
	uint8 address[2];
	uint8 frame_seq;
	tagDataAnc_t myTags[MAX_TAGS];
	uint8 superframe_seq;
	bool todos;
	uint8 nt;
	uint8 nenlazados;
	uint8 nperdidos;
	uint8 nfuera;
	uint8 enlazados[MAX_TAGS];
	uint8 perdidos[MAX_TAGS];
	uint8 fuera[MAX_TAGS];
	uint64 beacon_tx_ts;
} anchorData_t;


#define IS_POLL_MSG			1
#define IS_BEACON_MSG		2
#define IS_TIMEOUT			3
#define IS_ADDRESS_ERR		4
#define IS_FRAME_ERR		5
#define IS_ERROR_UNKNOWN	6

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536
//#define MS_TO_DWT_TIME 63897600
/* ANCHOR TIMINGS */
#define POLL_TX_TO_RESP_RX_DLY_UUS 190
#define ANC_RESP_RX_TIMEOUT_UUS 250

/* TAG TIMINGS */
#define POLL_RX_TO_RESP_TX_DLY_UUS 400
//#define TAG_RX_TIMEOUT_UUS 65535
/* Timestamps of frames transmission/reception.
 * As they are 40-bit wide, we need to define a 64-bit int type to handle them. */
typedef unsigned long long uint64;

/* Default antenna delay values for 64 MHz PRF. See NOTE 2 below. */
#define TX_ANT_DLY 16446
#define RX_ANT_DLY 16446

//#define TX_ANT_DLY 16436
//#define RX_ANT_DLY 16436

void initAnchor(anchorData_t *midata, uint8 nt);
void initTag(tagData_t *midata, uint8 nt);
uint8 do_range(anchorData_t *myAnc, uint8 myIndx);
uint8 do_answer(tagData_t *myTag);
void get_system_state(anchorData_t *myAnc);
uint8 do_beacon(anchorData_t *myAnc, uint8 myIndx);
uint8 do_link(tagData_t *myTag);
uint8 do_process_polling(tagData_t *myTag);


uint8 is_range_message(uint8 *buffer, uint16 length);
uint8 is_answer_message(uint8 *buffer, uint16 length);
uint8 is_general_beacon_message(uint8 *buffer, uint16 length);


void resp_msg_get_ts(uint8 *ts_field, uint32 *ts);
uint64 get_rx_timestamp_u64(void);
uint64 get_tx_timestamp_u64(void);
void resp_msg_set_ts(uint8 *ts_field, const uint64 ts);

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;


#endif /* APPLICATION_DEVICE_H_ */
