/*  --------------------------------------------------------------------------
 *  max31865.h
 *  Author: Trafficode
 *  ------------------------------------------------------------------------ */
#ifndef __MAX31865__
#define __MAX31865__

#include <stdint.h>
#include <stdbool.h>
#include "logger.h"

typedef struct {
	struct {
		logger_t *log;	/* Logger to debug */

		uint32_t refr;	/* Reference resistor value in ohms */

		/* _state: { 0, 1 } */
		void (*cs_pin_write)(int32_t _state);
		void (*clk_pin_write)(int32_t _state);
		void (*mosi_pin_write)(int32_t _state);
		int32_t (*miso_pin_read)(void);
	} init;
} max31865_t;

/* 	int32_t max31865_init(max31865_t *self)
 *	return:
 *	0 -> success
 */
int32_t max31865_init(
		max31865_t *self	/* object references */
		);

/* 	int32_t max31865_get(max31865_t *self, uint16_t *_tmul)
 *	return:
 *	0 -> success
 *	1 -> Read 0 Ohms
 */
int32_t max31865_get(
		max31865_t *self,	/* object references */
		int32_t *_tmul		/* return 10*temperature */
		);

#endif /* __MAX31865__ */
/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
