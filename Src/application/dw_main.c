/*! ----------------------------------------------------------------------------
 *  @file    main.c
 *  @brief   Single-sided two-way ranging (SS TWR) initiator example code.
 *
 *           This is a simple code example which acts as the initiator in a SS TWR distance measurement exchange. This application sends a "poll"
 *           frame (recording the TX time-stamp of the poll), after which it waits for a "response" message from the "DS TWR responder" example
 *           code (companion to this application) to complete the exchange. The response message contains the remote responder's time-stamps of poll
 *           RX, and response TX. With this data and the local time-stamps, (of poll TX and response RX), this example application works out a value
 *           for the time-of-flight over-the-air and, thus, the estimated distance between the two devices, which it writes to the LCD.
 *
 * @attention
 *
 * Copyright 2015 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "deca_device_api.h"
#include "deca_regs.h"
#include "sleep.h"
#include "port.h"
#include "device.h"

/* Default communication configuration. We use here EVK1000's mode 4. See NOTE 1 below. */
static dwt_config_t config = {
    2,               /* Channel number. */
    DWT_PRF_64M,     /* Pulse repetition frequency. */
    DWT_PLEN_128,    /* Preamble length. Used in TX only. */
    DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
    9,               /* TX preamble code. Used in TX only. */
    9,               /* RX preamble code. Used in RX only. */
    0,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_6M8,      /* Data rate. */
    DWT_PHRMODE_STD, /* PHY header mode. */
    (129 + 8 - 8)    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
};


//#define ALL_MSG_SN_IDX 2

#define DUMMY_BUFFER_LEN 1500

static uint8 dummy_buffer[DUMMY_BUFFER_LEN] __attribute__ ((aligned (4)));

int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

int _write(int file, char *ptr, int len)
{
	int DataIdx;
	for (DataIdx = 0; DataIdx < len; DataIdx++)
		__io_putchar(*ptr++);
	return len;
}




/* Inicializa al dw1000. No he hecho nada para asegurarme que no está en sleep mode
 * el parámetro lpnmode sirve para activar la salida GPIO5 y CPIO6 para depurar con
 * el osciloscopio. El delay de la antena está puesto a capón.
 */


void resucitaSPI(void){

	HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_RESET);
	usleep(600);
	HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_SET);
	while (! (HAL_GPIO_ReadPin(DW_RESET_GPIO_Port, DW_RESET_Pin)) );
	usleep(300);
	dwt_spicswakeup(dummy_buffer, DUMMY_BUFFER_LEN);
}

void initDW(int lpnmode)
{
	resucitaSPI();
	reset_DW1000(); /* Target specific drive of RSTn line into DW1000 low for a period. */
    port_set_dw1000_slowrate();
    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR)
    {
    	_Error_Handler("Inicializacion" , 56);
    }
    port_set_dw1000_fastrate();
    if(lpnmode){
    	dwt_setlnapamode(1, 1);
    }
    dwt_configure(&config);
    dwt_setrxantennadelay(RX_ANT_DLY);
    dwt_settxantennadelay(TX_ANT_DLY);

    /* Configure sleep and wake-up parameters. */
     dwt_configuresleep(DWT_PRESRV_SLEEP | DWT_LOADOPSET | DWT_CONFIG, DWT_WAKE_CS | DWT_SLP_EN);
     uint32 txgrain_mode = dwt_read32bitoffsetreg(PMSC_ID, PMSC_TXFINESEQ_OFFSET);
     dwt_write32bitoffsetreg(PMSC_ID, PMSC_TXFINESEQ_OFFSET, PMSC_TXFINESEQ_DISABLE);

}

static anchorData_t myAnc;
static tagData_t myTag;
static uint8 myerror;
static uint8 TagIdx = 0;
static bool tengo_datos = false;
static bool tengo_estado = false;
static bool a_dormir = false;


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	volatile uint8 respondido;
    if (htim->Instance == htim2.Instance)
    {
    	//	ESTO ES PARA EL ANCHOR
    	myerror = do_beacon(&myAnc, TagIdx);
    	if (TagIdx < myAnc.nt)
    		myerror = do_range(&myAnc, TagIdx);
    	TagIdx++;
    	if(myerror)
    		tengo_estado = true;
    	if(TagIdx == MAX_TAGS){
    		tengo_datos = true;
    		TagIdx = 0;
    		myAnc.superframe_seq++;
    	}
    }
    else if (htim->Instance == htim3.Instance)
    {
    	//	ESTO ES PARA EL TAG
    	if ((respondido = do_link(&myTag)) == IS_TIMEOUT){
    		a_dormir = true;
    	} else {
    		printf("\nasfd");
    	}
    	TagIdx++;
    	if (TagIdx > 1){
    		printf("joder");
    	}
    }
}

//#define ANCHOR

int dw_main(void)
{
	conf_data.ntags = 5;
	conf_data.test_tag = 2;
	conf_data.max_fallos_desenlaza = 5;
	conf_data.max_distancia = 1.5;

#ifdef ANCHOR
	initDW(1);
	dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
	dwt_setrxtimeout(ANC_RESP_RX_TIMEOUT_UUS);
	initAnchor(&myAnc, conf_data.ntags);
	HAL_TIM_Base_Start_IT(&htim2);
    while (1) {
    	if(tengo_datos){
    		tengo_datos = false;
    		get_system_state(&myAnc);
    		char dist_str[30] = {0};
    		if (!myAnc.todos){
    			sprintf(dist_str, "\n%d\t Enlazados = %d\t Perdidos = %d\t Fuera = %d",
    					myAnc.superframe_seq,
    					myAnc.nenlazados,
						myAnc.nperdidos,
						myAnc.nfuera);
    			printf("%s", dist_str);
    			printf("\t\tEnlazados: [");
    			for (uint8 i = 0; i < myAnc.nenlazados; i++){
    				printf("%d, ", myAnc.enlazados[i]);
    			}
    			printf("]");
    		} else
    		{
    			tengo_datos = false;
    			printf("\nEstamos todos");
    		}
    	}
    	if(tengo_estado){
        	if (myerror== 1)
        		printf("\nBeacon");
//        	else if (myerror == 2)
//        		printf("\nError");
        	tengo_estado = false;
    	}
    }
#else
	initDW(1);
    initTag(&myTag, conf_data.ntags, 950, 700);
    dwt_enableframefilter(DWT_FF_DATA_EN | DWT_FF_ACK_EN);

    HAL_TIM_Base_Start_IT(&htim3);
	while (1)
	{
		if (myTag.enlazado) {
			if(do_answer(&myTag)){
				dwt_entersleep();
				HAL_GPIO_TogglePin(DW_DEBB_GPIO_Port, DW_DEBB_Pin);
				HAL_Delay(myTag.answer_sleep_ms);
				resucitaSPI();
				HAL_GPIO_TogglePin(DW_DEBB_GPIO_Port, DW_DEBB_Pin);
			}
		} else if (a_dormir) {
			a_dormir = false;
			dwt_entersleep();
			HAL_GPIO_TogglePin(DW_DEBB_GPIO_Port, DW_DEBB_Pin);
			HAL_Delay(myTag.link_sleep_ms);
			resucitaSPI();
			HAL_GPIO_TogglePin(DW_DEBB_GPIO_Port, DW_DEBB_Pin);
		}
	}
#endif

}
