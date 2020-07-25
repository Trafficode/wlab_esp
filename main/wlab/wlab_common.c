/*  --------------------------------------------------------------------------
 *  wlab_esp: wlab_common.c
 *  Created on: 24 may 2020
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

#include "logger.h"
#include "wlab_common.h"

extern logger_t mlog;


void wlab_itostrf(char *dest, int32_t signed_int) {
	if( 0 > signed_int ) {
		signed_int = -signed_int;
		sprintf(dest, "-%d.%d", signed_int/10, abs(signed_int%10));
	} else {
		sprintf(dest, "%d.%d", signed_int/10, abs(signed_int%10));
	}
}

bool wlab_buffer_check(buffer_t *buffer) {
	bool rc=false;
	int32_t idx=0, avg=0, _buff=0, _cnt=0;

	avg = buffer->buff/buffer->cnt;

	if(WLAB_MIN_SAMPLES_COUNT < buffer->cnt && 0 < buffer->sample_ts) {
		if(WLAB_EXT2AVG_MAX<buffer->_max-avg || WLAB_EXT2AVG_MAX<avg-buffer->_min)
		{
			for(idx=0; idx<buffer->cnt; idx++) {
				if(WLAB_EXT2AVG_MAX < abs(avg-buffer->sample_buff[idx])) {
					buffer->sample_buff[idx] = 6666;
				} else {
					_buff += (int32_t)buffer->sample_buff[idx];
					_cnt++;
				}
			}
			if(WLAB_MIN_SAMPLES_COUNT < _cnt) {
				buffer->_max = INT32_MIN;
				buffer->_min = INT32_MAX;
				buffer->_max_ts = 0;
				buffer->_min_ts = 0;
				buffer->sample_ts_val = INT32_MAX;

				for(idx=0; idx<buffer->cnt; idx++) {
					if(6666 != buffer->sample_buff[idx]) {
						if(INT32_MAX == buffer->sample_ts_val) {
							buffer->sample_ts_val = buffer->sample_buff[idx];
						}
						if(buffer->sample_buff[idx] > buffer->_max) {
							buffer->_max = buffer->sample_buff[idx];
							buffer->_max_ts = buffer->sample_buff_ts[idx] - \
																		(buffer->sample_buff_ts[idx] % 60);
						}
						if(buffer->sample_buff[idx] < buffer->_min) {
							buffer->_min = buffer->sample_buff[idx];
							buffer->_min_ts = buffer->sample_buff_ts[idx] - \
																		(buffer->sample_buff_ts[idx] % 60);
						}
					}
				}
				buffer->buff = _buff;
				buffer->cnt = _cnt;
				rc = true;
			} else {
				logger_error(&mlog, "%s, not enought samples %d\n",
						__FUNCTION__, _cnt);
			}
		} else {
			rc = true;
		}
	}

	return(rc);
}

void wlab_buffer_init(buffer_t *buffer) {
	buffer->buff = 0;
	buffer->cnt = 0;
	buffer->_max = INT32_MIN;
	buffer->_min = INT32_MAX;
	buffer->_max_ts = 0;
	buffer->_min_ts = 0;
	buffer->sample_ts_val = INT32_MAX;
	buffer->sample_ts = 0;
}

/* wlab_buffer_commit
 * :param threshold
 * when that value exceed average then skip sample
 * when first sample will bad, then no next sample will be added so finally
 * sample count wont be enought
 */
bool wlab_buffer_commit(buffer_t *buffer, int32_t val, uint32_t ts,
												uint32_t threshold)
{
	if((buffer->_max != INT32_MIN) && ((val-threshold)>buffer->_max)) {
		logger_error(&mlog, "%s, Value %d max exceed threshold\n",
												__FUNCTION__, val);
		return(false);
	}

	if((buffer->_min != INT32_MAX) && ((val+threshold)<buffer->_min)) {
		logger_error(&mlog, "%s, Value %d min exceed threshold\n",
												__FUNCTION__, val);
		return(false);
	}

	if(WLAB_SAMPLE_BUFFER_SIZE > buffer->cnt) {
		buffer->sample_buff[buffer->cnt] = (int16_t)val;
		buffer->sample_buff_ts[buffer->cnt] = ts;
	} else {
		logger_error(&mlog, "%s, buffer length exceed\n", __FUNCTION__);
		return(false);
	}

	if(INT32_MAX == buffer->sample_ts_val) {
		/* Mark buffer timestamp as first sample time */
		buffer->sample_ts = ts - (ts % (60*CONFIG_WLAB_PUB_PERIOD));
		buffer->sample_ts_val = val;
	}

	if(val > buffer->_max) {
		buffer->_max = val;
		buffer->_max_ts = ts - (ts % 60);
	}

	if(val < buffer->_min) {
		buffer->_min = val;
		buffer->_min_ts = ts - (ts % 60);
	}

	buffer->buff += val;
	buffer->cnt++;
	return(true);
}


/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
