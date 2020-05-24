/*  --------------------------------------------------------------------------
 *  wlab_esp: wlab_common.h
 *  Created on: 24 may 2020
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#ifndef WLAB_COMMON_H_
#define WLAB_COMMON_H_

#include "wlab_config.h"

#define WLAB_TEMP_SERIE					(1)
#define WLAB_HUMIDITY_SERIE			(2)

/* Sometimes max return weird value not fitted to the other if this value
 * will be more than WLAB_EXT2AVG_MAX then skip */
#define WLAB_EXT2AVG_MAX					(32)
#define WLAB_MIN_SAMPLES_COUNT		(24)

#define WLAB_SAMPLE_BUFFER_SIZE \
					(8+((60*CONFIG_WLAB_PUB_PERIOD)/CONFIG_WLAB_MEASURE_PERIOD))

typedef struct {
	int32_t buff;
	int32_t cnt;
	int32_t min;
	int32_t max;
	uint32_t max_ts;
	uint32_t min_ts;
	uint32_t sample_ts;
	int32_t sample_ts_val;

	int16_t sample_buff[WLAB_SAMPLE_BUFFER_SIZE];
	uint32_t sample_buff_ts[WLAB_SAMPLE_BUFFER_SIZE];
} buffer_t;

void wlab_itostrf(char *dest, int32_t signed_int);

void wlab_buffer_init(buffer_t *buffer);
bool wlab_buffer_commit(buffer_t *buffer, int32_t val, uint32_t ts);
bool wlab_buffer_check(buffer_t *buffer);

#endif /* WLAB_COMMON_H_ */

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
