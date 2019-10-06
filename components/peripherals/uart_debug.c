/*  --------------------------------------------------------------------------
 *  wlab_esp: uart_debug.c  
 *  Created on: 4 sie 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#include <stdio.h>
#include <inttypes.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/uart.h"
#include "uart_debug.h"

#define INPUT_BUF_SIZE 		(256)
#define OUTPUT_BUF_SIZE 	(256)

static SemaphoreHandle_t uart_lock;

void uart_debug_init(void) {
    uart_config_t uart_config = {
        .baud_rate = UART_DEBUG_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, INPUT_BUF_SIZE * 2, 0, 0, NULL);

    /* Create a mutex type semaphore. */
    uart_lock = xSemaphoreCreateMutex();
    if( uart_lock == NULL ) {
    	for(;;);
    }
}

int uart_debug_send(uint8_t *data, uint16_t size) {
	xSemaphoreTake( uart_lock, UINT32_MAX );
	uart_write_bytes(UART_NUM_0, (const char *) data, size);
	xSemaphoreGive( uart_lock );
	return(0);
}

int uart_debug_getc(uint8_t *ch, uint32_t timeout) {
	uint32_t len = uart_read_bytes(	UART_NUM_0,
									(uint8_t *)ch,
									1,
									timeout);
	return(len==1 ? 1:0);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */




