/*  --------------------------------------------------------------------------
 *  wlab_esp: uart_debug.h  
 *  Created on: 4 sie 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#ifndef COMPONENTS_PERIPHERALS_UART_DEBUG_H_
#define COMPONENTS_PERIPHERALS_UART_DEBUG_H_

#define UART_DEBUG_BAUDRATE	(115200)

void uart_debug_init(void);

int uart_debug_send(
		uint8_t *data,
		uint16_t size
	);

/* 	uart_debug_getc
 *	return: 0 success,
 */
int uart_debug_getc(
		uint8_t *ch,
		uint32_t timeout
	);

#endif /* COMPONENTS_PERIPHERALS_UART_DEBUG_H_ */

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
