/*  --------------------------------------------------------------------------
 *  wlab_esp: wlab.c
 *  Created on: 29 sie 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#include <stdlib.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "logger.h"
#include "max31865_pins.h"
#include "max31865.h"
#include "config.h"
#include "user_config.h"
#include "mqtt_client.h"
#include "network.h"
#include "leds.h"

#include "wlab.h"

#define WLAB_TEMP_SERIE		(1)

#define WLAB_SAMPLE_BUFFER_SIZE \
					(8+((60*CONFIG_WLAB_PUB_PERIOD)/CONFIG_WLAB_MEASURE_PERIOD))

/* Sometimes max return weird value not fitted to the other if this value
 * will be more than WLAB_EXT2AVG_MAX then skip */
#define WLAB_EXT2AVG_MAX				(24)
#define WLAB_MIN_SAMPLES_COUNT	(24)

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

extern logger_t mlog;
extern char MAC_ADDR_STR[13];

static void wlab_task(void *arg);
static void wlab_itostrf(char *dest, int32_t signed_int);
static int wlab_authorize(void);
static int wlab_publish_sample(buffer_t *buffer);
static bool wlab_buffer_check(buffer_t *buffer);

static logger_t wlog;
static max31865_t sens;


const char *auth_template =	\
		"{\"timezone\":\"%s\",\"longitude\":%s,\"latitude\":%s,\"serie\":" \
		"{\"Temperature\":%d}," \
		"\"name\":\"%s\",\"description\":\"%s\", \"uid\":\"%s\"}";

const char *data_template = \
		"{\"UID\":\"%s\",\"TS\":%d,\"SERIE\":{\"Temperature\":" \
		"{\"f_avg\":%s,\"f_act\":%s,\"f_min\":%s,\"f_max\":%s," \
		"\"i_min_ts\":%u,\"i_max_ts\":%u}}}";


static void wlab_buffer_init(buffer_t *buffer) {
	buffer->buff = 0;
	buffer->cnt = 0;
	buffer->max = INT32_MIN;
	buffer->min = INT32_MAX;
	buffer->max_ts = 0;
	buffer->min_ts = 0;
	buffer->sample_ts_val = INT32_MAX;
	buffer->sample_ts = 0;
}

static bool wlab_buffer_check(buffer_t *buffer) {
	bool rc=false;
	int32_t idx=0, avg=0, _buff=0, _cnt=0;

	avg = buffer->buff/buffer->cnt;

	if(WLAB_MIN_SAMPLES_COUNT < buffer->cnt && 0 < buffer->sample_ts) {
		if(WLAB_EXT2AVG_MAX<buffer->max-avg || WLAB_EXT2AVG_MAX<avg-buffer->min) {
			for(idx=0; idx<buffer->cnt; idx++) {
				if(WLAB_EXT2AVG_MAX < abs(avg-buffer->sample_buff[idx])) {
					buffer->sample_buff[idx] = 6666;
				} else {
					_buff += (int32_t)buffer->sample_buff[idx];
					_cnt++;
				}
			}
			if(WLAB_MIN_SAMPLES_COUNT < _cnt) {
				buffer->max = INT32_MIN;
				buffer->min = INT32_MAX;
				buffer->max_ts = 0;
				buffer->min_ts = 0;
				buffer->sample_ts_val = INT32_MAX;

				for(idx=0; idx<buffer->cnt; idx++) {
					if(6666 != buffer->sample_buff[idx]) {
						if(INT32_MAX == buffer->sample_ts_val) {
							buffer->sample_ts_val = buffer->sample_buff[idx];
						}
						if(buffer->sample_buff[idx] > buffer->max) {
							buffer->max = buffer->sample_buff[idx];
							buffer->max_ts = buffer->sample_buff_ts[idx] - \
																		(buffer->sample_buff_ts[idx] % 60);
						}
						if(buffer->sample_buff[idx] < buffer->min) {
							buffer->min = buffer->sample_buff[idx];
							buffer->min_ts = buffer->sample_buff_ts[idx] - \
																		(buffer->sample_buff_ts[idx] % 60);
						}
					}
				}
				buffer->buff = _buff;
				buffer->cnt = _cnt;
				rc = true;
			} else {
				logger_error(&wlog, "%s, not enought samples %d\n",
						__FUNCTION__, _cnt);
			}
		} else {
			rc = true;
		}
	}

	return(rc);
}

static void wlab_buffer_commit(buffer_t *buffer, int32_t val, uint32_t ts) {
	if(WLAB_SAMPLE_BUFFER_SIZE > buffer->cnt) {
		buffer->sample_buff[buffer->cnt] = (int16_t)val;
		buffer->sample_buff_ts[buffer->cnt] = ts;
	} else {
		logger_error(&wlog, "%s, sample buffer idx exceed\n", __FUNCTION__);
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
}

static void wlab_task(void *arg) {
	time_t now=0;
	struct tm timeinfo;
	uint32_t last_minutes=0;
	buffer_t temp_buffer;
	int32_t temp=0;

	wlab_buffer_init(&temp_buffer);
	wlab_authorize();

	logger_cri(&wlog, "%s, wlab task started\n", __FUNCTION__);
	vTaskDelay(1000);

	for(;;) {
		vTaskDelay(1000*CONFIG_WLAB_MEASURE_PERIOD);
		time(&now);
		localtime_r(&now, &timeinfo);

		if(!max31865_get(&sens, &temp)) {
			logger_info(&wlog, "Temperature: %d\n", temp);
		} else {
			logger_error(&wlog, "%s, Pt100 fault.\n", __FUNCTION__);
			continue;
		}

		if(0x00 == timeinfo.tm_min % CONFIG_WLAB_PUB_PERIOD
				&& timeinfo.tm_min != last_minutes)
		{
			int32_t temp_avg = temp_buffer.buff/temp_buffer.cnt;
			logger_info(&wlog, "Min: %d Max: %d Avg: %d\n",
													temp_buffer.min, temp_buffer.max, temp_avg);
			if(wlab_buffer_check(&temp_buffer)) {
				logger_cri(&wlog, "Sample ready to send ...\n");
				if(0 != wlab_publish_sample(&temp_buffer)) {
					logger_error(&wlog, "%s, publish sample failed\n", __FUNCTION__);
					led_set_state(LED_MQTT, LED_TOOGLE_SLOW);
				} else {
					logger_info(&wlog, "%s, publish sample success\n", __FUNCTION__);
				}
			} else {
				logger_error(&wlog, "%s, check buffer failed\n", __FUNCTION__);
			}
			// ---------------------------------------------------------------------
			wlab_buffer_init(&temp_buffer);
			last_minutes = timeinfo.tm_min;
		} else {
			wlab_buffer_commit(&temp_buffer, temp, now);
		}
	}
}

void wlab_pt_init(void) {
	wlog.init.level = loggerLevelDebug;
	wlog.init.name = "wlab";
	logger_init(&wlog, &mlog);

	max31865_pins_init();

	sens.init.refr = 430;
	sens.init.log = &wlog;
	sens.init.cs_pin_write = max31865_pins_cs_write;
	sens.init.clk_pin_write = max31865_pins_clk_write;
	sens.init.mosi_pin_write = max31865_pins_mosi_write;
	sens.init.miso_pin_read = max31865_pins_miso_read;
	max31865_init(&sens);
}

void wlab_start(void) {
	xTaskCreate(wlab_task, "wlab_task", 2048, NULL, 10, NULL);
}

static void wlab_itostrf(char *dest, int32_t signed_int) {
	if( 0 > signed_int ) {
		signed_int = -signed_int;
		sprintf(dest, "-%d.%d", signed_int/10, abs(signed_int%10));
	} else {
		sprintf(dest, "%d.%d", signed_int/10, abs(signed_int%10));
	}
}

static int wlab_publish_sample(buffer_t *buffer) {
	int rc=0;
	int32_t temp_avg=0;
	char tavg_str[8], tact_str[8], tmin_str[8], tmax_str[8];

	temp_avg = buffer->buff/buffer->cnt;

	wlab_itostrf(tavg_str, temp_avg);
	wlab_itostrf(tact_str, buffer->sample_ts_val);
	wlab_itostrf(tmin_str, buffer->min);
	wlab_itostrf(tmax_str, buffer->max);

	rc = mqtt_printf(
			CONFIG_WLAB_PUB_TOPIC,
			4000,
			data_template,
			MAC_ADDR_STR,
			buffer->sample_ts,
			tavg_str,
			tact_str,
			tmin_str,
			tmax_str,
			buffer->min_ts,
			buffer->max_ts
	);
	return(rc);
}

static int wlab_authorize(void) {
	int rc=0;
	logger_info(&wlog, "%s, wlab_authorize()\n", __FUNCTION__);

	rc = mqtt_printf(
			CONFIG_WLAB_AUTH_TOPIC,
			4000,
			auth_template,
			CONFIG_WLAB_TIMEZONE,
			CONFIG_WLAB_LATITIUDE,
			CONFIG_WLAB_LONGITUDE,
			WLAB_TEMP_SERIE,
			CONFIG_WLAB_NAME,
			CONFIG_WLAB_DESC,
			MAC_ADDR_STR
	);
	return(rc);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
