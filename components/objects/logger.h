/*  --------------------------------------------------------------------------
 *  logger.h    
 *  Author: Trafficode
 *  ------------------------------------------------------------------------ */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

typedef enum logger_level {
	loggerLevelDebug,
	loggerLevelInfo,
	loggerLevelWarn,
	loggerLevelCritical,
	loggerLevelError
} loglevel_t;

typedef struct {
	/* Initialize object in this structure */
	struct {
		const char *name;
		loglevel_t level;

		/* Do not init when used as copy */
		int (*putd)(uint8_t *, uint16_t);
		uint16_t buff_size;
		uint8_t *buff;
	} init;
	
	/* Private */
	SemaphoreHandle_t _buff_lock;
} logger_t;

void logger_init(logger_t *self, logger_t *parent);

void logger_error(logger_t *self, const char *fmt, ...);
void logger_cri(logger_t *self, const char *fmt, ...);
void logger_warn(logger_t *self, const char *fmt, ...);
void logger_info(logger_t *self, const char *fmt, ...);
void logger_debug(logger_t *self, const char *fmt, ...);

#endif /* LOGGER_H_ */

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
