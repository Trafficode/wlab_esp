/*  --------------------------------------------------------------------------
 *  logger.c    
 *  Author: Trafficode
 *  ------------------------------------------------------------------------ */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "logger.h"

static uint32_t logger_place_padding(
		logger_t *self,
		const char *lvl,
		uint32_t lvl_size
	);

const char _debug[] 	= " debug - ";
const char _info[] 		= "  info - ";
const char _warn[] 		= "  warn - ";
const char _critical[] 	= "   cri - ";
const char _error[] 	= " error - ";

void logger_init(logger_t *self, logger_t *parent) {
	if( NULL == parent ) {
		self->_buff_lock = xSemaphoreCreateMutex();
		if( NULL == self->_buff_lock) {
			for(;;);
		}
	} else {
		self->_buff_lock = parent->_buff_lock;
		self->init.buff = parent->init.buff;
		self->init.buff_size = parent->init.buff_size;
		self->init.putd = parent->init.putd;
	}
}

static uint32_t 
logger_place_padding(logger_t *self, const char *lvl, uint32_t lvl_size) {
	uint32_t curr_len=0;
	unsigned long ms_tick = (uint32_t)xTaskGetTickCount();

	/* [000.000]: */
	ms_tick %= 1000000;
	curr_len = sprintf(	(char *)self->init.buff,
						"[%03lu.%03lu]: %8s - ",
						ms_tick/1000, ms_tick%1000, self->init.name);

	memcpy((char *)self->init.buff+curr_len, lvl, lvl_size);
	curr_len += lvl_size;
	return(curr_len);
}

void logger_error(logger_t *self, const char *fmt, ...) {
	uint32_t curr_len=0, n=0;
	va_list args;
	va_start(args, fmt);

	if( loggerLevelError >= self->init.level ) {
		xSemaphoreTake( self->_buff_lock, UINT32_MAX );

		/* Place timestamp, name and debug level */
		curr_len = logger_place_padding(self, _error, sizeof(_error));
		
		n = vsnprintf(	(char *)(self->init.buff+curr_len),
								self->init.buff_size-curr_len,
								fmt, args);

		if( self->init.buff_size < n+curr_len ) {
			self->init.buff[self->init.buff_size-2] = '$';
			self->init.buff[self->init.buff_size-1] = '\n';
			n = self->init.buff_size;
		} else {
			n += curr_len;
		}

		self->init.putd(self->init.buff, n);

		xSemaphoreGive( self->_buff_lock );
	}

	va_end(args);
}

void logger_cri(logger_t *self, const char *fmt, ...) {
	uint32_t curr_len=0, n=0;
	va_list args;
	va_start(args, fmt);

	if( loggerLevelCritical >= self->init.level ) {
		xSemaphoreTake( self->_buff_lock, UINT32_MAX );

		/* Place timestamp, name and debug level */
		curr_len = logger_place_padding(self, _critical, sizeof(_critical)-1);

		n = vsnprintf(	(char *)(self->init.buff+curr_len),
								self->init.buff_size-curr_len,
								fmt, args);

		if( self->init.buff_size < n+curr_len ) {
			self->init.buff[self->init.buff_size-2] = '$';
			self->init.buff[self->init.buff_size-1] = '\n';
			n = self->init.buff_size;
		} else {
			n += curr_len;
		}

		self->init.putd(self->init.buff, n);

		xSemaphoreGive( self->_buff_lock );
	}

	va_end(args);
}

void logger_warn(logger_t *self, const char *fmt, ...) {
	uint32_t curr_len=0, n=0;
	va_list args;
	va_start(args, fmt);

	if( loggerLevelWarn >= self->init.level ) {
		xSemaphoreTake( self->_buff_lock, UINT32_MAX );

		/* Place timestamp, name and debug level */
		curr_len = logger_place_padding(self, _warn, sizeof(_warn)-1);

		n = vsnprintf(	(char *)(self->init.buff+curr_len),
								self->init.buff_size-curr_len,
								fmt, args);
		
		if( self->init.buff_size < n+curr_len ) {
			self->init.buff[self->init.buff_size-2] = '$';
			self->init.buff[self->init.buff_size-1] = '\n';
			n = self->init.buff_size;
		} else {
			n += curr_len;
		}

		self->init.putd(self->init.buff, n);

		xSemaphoreGive( self->_buff_lock );
	}

	va_end(args);
}

void logger_info(logger_t *self, const char *fmt, ...) {
	uint32_t curr_len=0, n=0;
	va_list args;
	va_start(args, fmt);

	if( loggerLevelInfo >= self->init.level ) {
		xSemaphoreTake( self->_buff_lock, UINT32_MAX );

		/* Place timestamp, name and debug level */
		curr_len = logger_place_padding(self, _info, sizeof(_info)-1);

		n = vsnprintf(	(char *)(self->init.buff+curr_len),
								self->init.buff_size-curr_len,
								fmt, args);
		
		if( self->init.buff_size < n+curr_len ) {
			self->init.buff[self->init.buff_size-2] = '$';
			self->init.buff[self->init.buff_size-1] = '\n';
			n = self->init.buff_size;
		} else {
			n += curr_len;
		}

		self->init.putd(self->init.buff, n);
		xSemaphoreGive( self->_buff_lock );
	}

	va_end(args);
}

void logger_debug(logger_t *self, const char *fmt, ...) {
	uint32_t curr_len=0, n=0;
	va_list args;
	va_start(args, fmt);

	if( loggerLevelDebug >= self->init.level ) {
		xSemaphoreTake( self->_buff_lock, UINT32_MAX );

		/* Place timestamp, name and debug level */
		curr_len = logger_place_padding(self, _debug, sizeof(_debug)-1);

		n = vsnprintf(	(char *)(self->init.buff+curr_len),
								self->init.buff_size-curr_len,
								fmt, args);
		
		if( self->init.buff_size < n+curr_len ) {
			self->init.buff[self->init.buff_size-2] = '$';
			self->init.buff[self->init.buff_size-1] = '\n';
			n = self->init.buff_size;
		} else {
			n += curr_len;
		}
		
		self->init.putd(self->init.buff, n);
		xSemaphoreGive( self->_buff_lock );
	}

	va_end(args);
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

