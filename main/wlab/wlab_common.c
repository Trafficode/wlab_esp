/*  --------------------------------------------------------------------------
 *  wlab_esp: wlab_common.c
 *  Created on: 24 may 2020
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "wlab_common.h"

void wlab_itostrf(char *dest, int32_t signed_int) {
	if( 0 > signed_int ) {
		signed_int = -signed_int;
		sprintf(dest, "-%d.%d", signed_int/10, abs(signed_int%10));
	} else {
		sprintf(dest, "%d.%d", signed_int/10, abs(signed_int%10));
	}
}

void wlab_buffer_init(buffer_t *buffer) {
	buffer->buff = 0;
	buffer->cnt = 0;
	buffer->max = INT32_MIN;
	buffer->min = INT32_MAX;
	buffer->max_ts = 0;
	buffer->min_ts = 0;
	buffer->sample_ts_val = INT32_MAX;
	buffer->sample_ts = 0;
}

bool wlab_buffer_commit(buffer_t *buffer, int32_t val, uint32_t ts) {
	if(WLAB_SAMPLE_BUFFER_SIZE > buffer->cnt) {
		buffer->sample_buff[buffer->cnt] = (int16_t)val;
		buffer->sample_buff_ts[buffer->cnt] = ts;
	} else {
		return(false);
	}

	if(INT32_MAX == buffer->sample_ts_val) {
		buffer->sample_ts = ts - (ts % (60*CONFIG_WLAB_PUB_PERIOD));
		buffer->sample_ts_val = val;
	}
	if(val > buffer->max) {
		buffer->max = val;
		buffer->max_ts = ts - (ts % 60);
	}
	if(val < buffer->min) {
		buffer->min = val;
		buffer->min_ts = ts - (ts % 60);
	}
	buffer->buff += val;
	buffer->cnt++;
	return(true);
}


/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
