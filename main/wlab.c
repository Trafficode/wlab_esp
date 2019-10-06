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

#include "wlab.h"

#define WLAB_TEMP_SERIE		(1)

typedef struct {
	int32_t buff;
	uint32_t cnt;
	int32_t min;
	int32_t max;
	uint32_t max_ts;
	uint32_t min_ts;
	uint32_t sample_ts;
	int32_t sample_ts_val;
} buffer_t;

extern logger_t mlog;
extern char MAC_ADDR_STR[13];

static void wlab_task(void *arg);
static void wlab_itostrf(char *dest, int32_t signed_int);
static int wlab_authorize(void);
static int wlab_publish_sample(buffer_t *buffer);

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

static void print_ts(const char *pre, time_t ts) {
	struct tm timeinfo;
	char strftime_buf[64];
	localtime_r(&ts, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	logger_info(&wlog, "%s %s\n", pre, strftime_buf);
}

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

static void wlab_buffer_commit(buffer_t *buffer, int32_t val, uint32_t ts) {
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
	int32_t temp=0, _temp=UINT32_MAX;
	bool msrfailed = false;

	wlab_buffer_init(&temp_buffer);
	wlab_authorize();

	logger_cri(&wlog, "%s, wlab task started\n", __FUNCTION__);
	vTaskDelay(1000);

	for(;;) {
		vTaskDelay(1000);
		time(&now);
		localtime_r(&now, &timeinfo);

		if(!max31865_get(&sens, &temp)) {
			logger_debug(&wlog, "Temperature: %d Min: %d\n", temp, timeinfo.tm_min);
			wlab_buffer_commit(&temp_buffer, temp, now);
			if(UINT32_MAX != _temp) {
				if(8 < abs(_temp-temp) && false == msrfailed) {
					logger_error(&wlog, "%s, pt100 measurment failed\n", __FUNCTION__);
					msrfailed = true;
					vTaskDelay(2000);
					continue;
				} else {
					msrfailed = false;
					_temp = temp;
				}
			} else {
				_temp = temp;
			}
		} else {
			logger_error(&wlog, "%s, Pt100 fault.\n", __FUNCTION__);
			continue;
		}

		if(0x00 == timeinfo.tm_min % CONFIG_WLAB_PUB_PERIOD
				&& timeinfo.tm_min != last_minutes)
		{
			// Send sample here ----------------------------------------------------
			int32_t temp_avg = temp_buffer.buff/temp_buffer.cnt;
			print_ts("Time now: ", now);
			print_ts("Sample time: ", temp_buffer.sample_ts);
			print_ts("Max time: ", temp_buffer.max_ts);
			print_ts("Min time: ", temp_buffer.min_ts);
			logger_info(&wlog, "Min: %d Max: %d Avg: %d\n",
													temp_buffer.min, temp_buffer.max, temp_avg);
			if(60 < temp_buffer.cnt && 0 < temp_buffer.sample_ts) {
				logger_cri(&wlog, "Sample ready to send ...\n");
				if(0 != wlab_publish_sample(&temp_buffer)) {
					logger_info(&wlog, "%s, publish sample failed\n", __FUNCTION__);
				} else {
					logger_info(&wlog, "%s, publish sample success\n", __FUNCTION__);
				}
			}
			// ---------------------------------------------------------------------
			wlab_buffer_init(&temp_buffer);
			last_minutes = timeinfo.tm_min;
		}
	}
}

void wlab_init(void) {
	wlog.init.level = loggerLevelInfo;
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

	if(0 == rc) {
		logger_info(&wlog, "%s, publish sample success\n", __FUNCTION__);
	} else {
		logger_error(&wlog, "%s, publish sample failed\n", __FUNCTION__);
	}

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

	if(0 == rc) {
		logger_info(&wlog, "%s, authorize success\n", __FUNCTION__);
	} else {
		logger_error(&wlog, "%s, authorize failed\n", __FUNCTION__);
	}
	return(rc);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */




