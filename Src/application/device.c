/*
 * device.c
 *
 *  Created on: 31 ene. 2018
 *      Author: dpeinado
 */

#include "deca_device_api.h"
#include "deca_regs.h"
#include "sleep.h"
#include "port.h"

#include "device.h"


#define RX_BUF_LEN 20

static uint8 rx_buffer[RX_BUF_LEN];

/* Hold copy of status register state here for reference so that it can be examined at a debug breakpoint. */

volatile uint32 status_reg = 0;

void get_system_state(anchorData_t *myAnc)
{
	myAnc->todos=true;
	myAnc->nperdidos = 0;
	myAnc->nfuera = 0;
	myAnc->nenlazados=0;
	for(uint8 i=0;i<myAnc->nt;i++){
		if (!myAnc->myTags[i].enlazado)
			myAnc->perdidos[myAnc->nperdidos++]=i;
		else {
			myAnc->enlazados[myAnc->nenlazados++]=i;
			if (myAnc->myTags[i].distance > conf_data.max_distancia)
				myAnc->fuera[myAnc->nfuera++]=i;
		}
	}
	if (myAnc->nperdidos || myAnc->nfuera)
		myAnc->todos=false;
}

void initAnchor(anchorData_t *midata, uint8 nt){
	midata->panID[0]=0x01;
	midata->panID[1]=0x00;
	midata->address[0]=0xF0;
	midata->address[1]=0x00;
	midata->frame_seq = 0;
	midata->superframe_seq = 0;
	midata->todos = false;
	midata->nt = nt;
	for(uint8 i=0;i<nt; i++){
		midata->myTags[i] = (tagDataAnc_t){
			{0x01, 0x00},
			{i, 0x00},
			0,
			false,
			0,
			0.0,
			nt
		};
	}

	rangeFrame = (frameAncPoll_t) {
		{0x41, 0x88},
		0,
		{midata->panID[0], midata->panID[1]},
		{0,0},
		{midata->address[0], midata->address[1]},
		TWR_POLL_CMD,
		{0, 0}
	};

	beaconFrame = (frameAncBeacon_t) {
		{0x41, 0x88},
		0,
		{midata->panID[0], midata->panID[1]},
		{0xFF, 0xFF},
		{midata->address[0], midata->address[1]},
		{TWR_SYNC_BEACON,0},
		{0, 0}
	};

	midata->beacon_tx_ts = (uint64) (MS_FROM_BEACON_TO_RANGE * 63897.76358)>>8;

	dwt_writetodevice(PANADR_ID, PANADR_PAN_ID_OFFSET,2, midata->panID);
	dwt_writetodevice(PANADR_ID, PANADR_SHORT_ADDR_OFFSET, 2, midata->address);
}

void initTag(tagData_t *midata, uint8 nt)
{
	midata->panID[0]=0x01;
	midata->panID[1]=0x00;
	midata->address[0]=conf_data.test_tag;
	midata->address[1]=0;
	midata->frame_seq = 0;
	midata->enlazado = false;
	midata->fallos = 0;
	midata->nTags = nt;
	midata->beaconNumber=0;
	midata->timeRXBeacon = 0;
	midata->enlaceBeacon = false;
	midata->framePeriod = (uint64) (MAX_TAGS*7500*63897.76358) >> 8;

	answerFrame = (frameTagFinal_t )
	{
		{0x41, 0x88},
		0,
		{midata->panID[0], midata->panID[1]},
		{0xF0, 0x0},
		{midata->address[0], midata->address[1]},
		{TWR_ANSW_CMD, 0,0,0,0,0,0,0,0},
		{0, 0}
	};

	dwt_writetodevice(PANADR_ID, PANADR_PAN_ID_OFFSET,2, midata->panID);
	dwt_writetodevice(PANADR_ID, PANADR_SHORT_ADDR_OFFSET, 2, midata->address);
}
//
//uint8 is_range_message(uint8 *buffer, uint16 length)
//{
//	if (
//			(buffer[FRAME_INDX_MESSAGE_DATA] == TWR_POLL_CMD) /*&&
//			(buffer[FRAME_INDX_DEST_ADDRESS] == answerFrame.sourceAddr[0]) &&
//			(buffer[FRAME_INDX_DEST_ADDRESS+1] == answerFrame.sourceAddr[1])*/
//		)
//		return 1;
//	else
//		return 0;
//}
//
uint8 is_answer_message(uint8 *buffer, uint16 length)
{
	if (
			(buffer[FRAME_INDX_MESSAGE_DATA] == TWR_ANSW_CMD) &&
			(buffer[FRAME_INDX_SOURCE_ADDRESS] == rangeFrame.destAddr[0]) &&
			(buffer[FRAME_INDX_SOURCE_ADDRESS+1] == rangeFrame.destAddr[1])
		)
		return 1;
	else
		return 0;
}
//
//uint8 is_beacon_message(uint8 *buffer, uint16 length)
//{
//	if (buffer[FRAME_INDX_MESSAGE_DATA] == TWR_SYNC_BEACON)
//		return 1;
//	else
//		return 0;
//}
//
//uint8 is_general_beacon_message(uint8 *buffer, uint16 length)
//{
//	if (buffer[FRAME_INDX_MESSAGE_DATA] == TWR_SYNC_GENBEACON)
//		return 1;
//	else
//		return 0;
//}


uint8 do_beacon(anchorData_t *myAnc, uint8 myIndx) {

	uint32 range_tx_time;
	beaconFrame.seqNum++;
	beaconFrame.messageData[1] = myIndx;

	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_TX);
	dwt_writetxdata(sizeof(beaconFrame), (uint8 *) &beaconFrame, 0); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(sizeof(beaconFrame), 0, 1); /* Zero offset in TX buffer, ranging. */

	dwt_starttx(DWT_START_TX_IMMEDIATE);
	while( !((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & SYS_STATUS_TXFRS) );
	range_tx_time = dwt_readtxtimestamphi32();
	range_tx_time = range_tx_time + myAnc->beacon_tx_ts;
	dwt_setdelayedtrxtime(range_tx_time);
	return 0;
}

uint8 do_range(anchorData_t *myAnc, uint8 myIndx) {
	rangeFrame.seqNum = myAnc->frame_seq++;
	rangeFrame.destAddr[0] = myAnc->myTags[myIndx].address[0];
	rangeFrame.destAddr[1] = myAnc->myTags[myIndx].address[1];

	dwt_writetxdata(sizeof(rangeFrame), (uint8 *) &rangeFrame, 0); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(sizeof(rangeFrame), 0, 1); /* Zero offset in TX buffer, ranging. */


	//dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
	dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
	while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & SYS_STATUS_TXFRS));

	while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
			(SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

	if (status_reg & SYS_STATUS_RXFCG)
	{
		uint32 frame_len;

		/* Clear good RX frame event in the DW1000 status register. */
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_GOOD);

		/* A frame has been received, read it into the local buffer. */
		frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
		if (frame_len <= RX_BUF_LEN)
		{
			dwt_readrxdata(rx_buffer, frame_len, 0);
		}

		// Aquí es donde iría la validación del frame
		if(is_answer_message(rx_buffer, frame_len))
		{
			uint32 poll_tx_ts, resp_rx_ts, poll_rx_ts, resp_tx_ts;
			int32 rtd_init, rtd_resp;
			float clockOffsetRatio ;

			/* Retrieve poll transmission and response reception timestamps. See NOTE 9 below. */
			poll_tx_ts = dwt_readtxtimestamplo32();
			resp_rx_ts = dwt_readrxtimestamplo32();

			/* Read carrier integrator value and calculate clock offset ratio. See NOTE 11 below. */
			clockOffsetRatio = dwt_readcarrierintegrator() * (FREQ_OFFSET_MULTIPLIER * HERTZ_TO_PPM_MULTIPLIER_CHAN_2 / 1.0e6) ;

			/* Get timestamps embedded in response message. */
			resp_msg_get_ts(&rx_buffer[RESP_MSG_POLL_RX_TS_IDX], &poll_rx_ts);
			resp_msg_get_ts(&rx_buffer[RESP_MSG_RESP_TX_TS_IDX], &resp_tx_ts);

			/* Compute time of flight and distance, using clock offset ratio to correct for differing local and remote clock rates */
			rtd_init = resp_rx_ts - poll_tx_ts;
			rtd_resp = resp_tx_ts - poll_rx_ts;

			myAnc->myTags[myIndx].distance = ((rtd_init - rtd_resp * (1 - clockOffsetRatio)) / 2.0)
					* SPEED_OF_LIGHT * DWT_TIME_UNITS;
			myAnc->myTags[myIndx].enlazado = true;
			myAnc->myTags[myIndx].fallos = 0;
			return 0;
		} else return 1;
	}
	else if (status_reg & SYS_STATUS_ALL_RX_TO)
	{
		myAnc->myTags[myIndx].fallos++;
		if (myAnc->myTags[myIndx].fallos == conf_data.max_fallos_desenlaza)
			myAnc->myTags[myIndx].enlazado = false;

		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		dwt_rxreset();
		status_reg = dwt_read32bitreg(SYS_STATUS_ID);
		return 2;
	} else {
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
		dwt_rxreset();
	}
	return 0;
}


uint8 do_read(uint16 timeout, int mode){
	uint32 frame_len;
	uint8 respuesta = 0;

	dwt_setrxtimeout(timeout);
	dwt_rxenable(mode);
	while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) &
		(SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)));

	if (status_reg & SYS_STATUS_RXFCG){
	    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_GOOD);

	    frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
	    if (frame_len <= RX_BUFFER_LEN)
	    {
	        dwt_readrxdata(rx_buffer, frame_len, 0);
	    }

	    if (rx_buffer[FRAME_INDX_MESSAGE_DATA] == TWR_SYNC_BEACON)
	    	return IS_BEACON_MSG;
	    if (rx_buffer[FRAME_INDX_MESSAGE_DATA] == TWR_POLL_CMD)
	    	return IS_POLL_MSG;


	} else {

		if (status_reg & SYS_STATUS_AFFREJ)
			respuesta = IS_ADDRESS_ERR;
		if (status_reg & SYS_STATUS_ALL_RX_TO)
			respuesta = IS_TIMEOUT;
		if (status_reg & SYS_STATUS_ALL_RX_ERR)
			respuesta = IS_FRAME_ERR;
		if (respuesta){
			dwt_rxreset();
			dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO);
			dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
			return respuesta;
		}
		return IS_ERROR_UNKNOWN;
	}
	return IS_ERROR_UNKNOWN;
}

uint8 do_link(tagData_t *myTag){
	uint8 respuesta;

	respuesta = do_read(9750, DWT_START_RX_IMMEDIATE);
	if (respuesta == IS_POLL_MSG){
		respuesta = do_read(9750, DWT_START_RX_IMMEDIATE);
		if (respuesta != IS_BEACON_MSG){
			dwt_rxreset();
			return -1;
		}
	} else if (respuesta == IS_TIMEOUT){
		status_reg = dwt_read32bitreg(SYS_STATUS_ID);
		dwt_rxreset();
		status_reg = dwt_read32bitreg(SYS_STATUS_ID);
		if (myTag->enlazado){
			myTag->enlazado = false;
			HAL_TIM_Base_Start_IT(&htim3);
		}
		return -1;
	} else if (respuesta != IS_BEACON_MSG){
		if (myTag->enlazado){
			myTag->enlazado = false;
			HAL_TIM_Base_Start_IT(&htim3);
		}
		dwt_rxreset();
		return -1;
	}
	HAL_TIM_Base_Stop_IT(&htim3);
	myTag->beaconNumber = rx_buffer[FRAME_INDX_TAG_NUMBER];
	myTag->timeRXBeacon = dwt_readrxtimestamphi32();
	myTag->enlaceBeacon = true;
	uint8 i, j;
	uint32 cualas;
	i = myTag->address[0];
	j = myTag->beaconNumber;

	if (i<j) {
		cualas = (uint64) ((((MAX_TAGS - j) + i)*7500) * 63897.76358) >> 8;
	} else {
		cualas = (uint64) (((i - j)*7500) * 63897.76358) >> 8;
	}
	dwt_setdelayedtrxtime(myTag->timeRXBeacon+cualas);
	respuesta = do_read(9750, DWT_START_RX_DELAYED);
	if (respuesta == IS_POLL_MSG){
		do_process_polling(myTag);
	} else {
		if (myTag->enlazado){
			myTag->enlazado = false;
			HAL_TIM_Base_Start_IT(&htim3);
		}
		dwt_rxreset();
		return -1;
	}
	return 0;
}

uint8 do_answer(tagData_t *myTag){
	uint8 respuesta;
	respuesta = do_read(9750, DWT_START_RX_DELAYED  | DWT_IDLE_ON_DLY_ERR);
	if (respuesta == IS_POLL_MSG)
    	do_process_polling(myTag);
	else {
		myTag->enlazado = false;
		HAL_TIM_Base_Start_IT(&htim3);
	}
	return 0;
}


uint8 do_process_polling(tagData_t *myTag){
    uint64 poll_rx_ts;
    uint64 resp_tx_ts;
	uint32 resp_tx_time;
	int ret;

	poll_rx_ts = get_rx_timestamp_u64(); // mirar esto, parece que se ha arreglado una cosa y se ha fastidiado la otra
	//poll_rx_ts = dwt_readrxtimestamphi32();
	resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
	dwt_setdelayedtrxtime(resp_tx_time);
	resp_tx_ts = (((uint64)(resp_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
	resp_msg_set_ts((uint8 *) &answerFrame.messageData+1, poll_rx_ts);
	resp_msg_set_ts((uint8 *) &answerFrame.messageData+5, resp_tx_ts);
	answerFrame.seqNum = myTag->frame_seq++;
	dwt_writetxdata(sizeof(answerFrame),(uint8 *) &answerFrame, 0); /* Zero offset in TX buffer. */
	dwt_writetxfctrl(sizeof(answerFrame), 0, 1); /* Zero offset in TX buffer, ranging. */
	ret = dwt_starttx(DWT_START_TX_DELAYED);

	if (ret == DWT_SUCCESS)
	{
		while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
	}
	myTag->enlazado = true;
	resp_tx_ts = ((poll_rx_ts - (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8) + myTag->framePeriod;
	dwt_setdelayedtrxtime( resp_tx_ts );
	return 0;
}


void resp_msg_get_ts(uint8 *ts_field, uint32 *ts)
{
    int i;
    *ts = 0;
    for (i = 0; i < RESP_MSG_TS_LEN; i++)
    {
        *ts += ts_field[i] << (i * 8);
    }
}

uint64 get_rx_timestamp_u64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readrxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

uint64 get_tx_timestamp_u64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readtxtimestamp(ts_tab);
    for (i = 4; i >= 0; i--)
    {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}

void resp_msg_set_ts(uint8 *ts_field, const uint64 ts)
{
    int i;
    for (i = 0; i < RESP_MSG_TS_LEN; i++)
    {
        ts_field[i] = (ts >> (i * 8)) & 0xFF;
    }
}

