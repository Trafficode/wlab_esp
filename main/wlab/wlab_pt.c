/*  --------------------------------------------------------------------------
 *  wlab_esp: wlab_pt.c
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
#include "mqtt_client.h"
#include "../network.h"
#include "../user_config.h"

#include "leds.h"
#include "wlab_common.h"
#include "wlab_pt.h"

#define WLAB_TEMP_SERIE		(1)

extern logger_t mlog;
extern char MAC_ADDR_STR[13];

static void wlab_pt_task(void *arg);
static int wlab_pt_authorize(void);
static int wlab_pt_publish_sample(buffer_t *buffer);

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

static buffer_t temp_buffer;

static void wlab_pt_task(void *arg) {
	time_t now=0;
	struct tm timeinfo;
	uint32_t last_minutes=0;
	int32_t temp=0;

	wlab_buffer_init(&temp_buffer);
	wlab_pt_authorize();

	logger_cri(&wlog, "%s, wlab task started\n", __FUNCTION__);
	vTaskDelay(1000);

	for(;;) {
		vTaskDelay(1000*CONFIG_WLAB_MEASURE_PERIOD);
		time(&now);
		localtime_r(&now, &timeinfo);

		if(!max31865_get(&sens, &temp)) {
			logger_info(&wlog, "temp: %d\n", temp);
		} else {
			logger_error(&wlog, "%s, PT100 failed.\n", __FUNCTION__);
			continue;
		}

		if(0x00 == timeinfo.tm_min % CONFIG_WLAB_PUB_PERIOD
				&& timeinfo.tm_min != last_minutes)
		{
			int32_t temp_avg = temp_buffer.buff/temp_buffer.cnt;
			logger_info(&wlog, "Min: %d Max: %d Avg: %d\n",
													temp_buffer._min, temp_buffer._max, temp_avg);

			if(wlab_buffer_check(&temp_buffer)) {
				logger_cri(&wlog, "Sample ready to send ...\n");
				if(0 != wlab_pt_publish_sample(&temp_buffer)) {
					logger_error(&wlog, "%s, publish sample failed\n", __FUNCTION__);
					led_set_state(LED_MQTT, LED_TOOGLE_SLOW);
				} else {
					logger_info(&wlog, "%s, publish sample success\n", __FUNCTION__);
				}
			} else {
				logger_error(&wlog, "%s, check buffer failed\n", __FUNCTION__);
			}

			wlab_buffer_init(&temp_buffer);
			last_minutes = timeinfo.tm_min;
		} else {
			wlab_buffer_commit(&temp_buffer, temp, now, 4);
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

void wlab_pt_start(void) {
	xTaskCreate(wlab_pt_task, "wlab_task", 2*1024, NULL, 10, NULL);
}

int wlab_pt_push(void) {
	int32_t temp_avg=0;
	int rc=0;
	time_t now, tmp;
	time(&now);
	if(0 == temp_buffer.cnt) {
		logger_error(&wlog, "buffer.cnt = 0, wait a bit...\n");
	} else {
		tmp = temp_buffer.sample_ts;
		temp_buffer.sample_ts = now;
		temp_avg = temp_buffer.buff/temp_buffer.cnt;
		logger_info(&wlog,
			"temp - min: %d max: %d avg: %d, cnt: %d\n",
			temp_buffer._min,
			temp_buffer._max,
			temp_avg,
			temp_buffer.cnt
		);
		rc = wlab_pt_publish_sample(&temp_buffer);
		temp_buffer.sample_ts = tmp;
	}

	return(rc);
}

static int wlab_pt_publish_sample(buffer_t *buffer) {
	int rc=0;
	int32_t temp_avg=0;
	char tavg_str[8], tact_str[8], tmin_str[8], tmax_str[8];

	temp_avg = buffer->buff/buffer->cnt;

	wlab_itostrf(tavg_str, temp_avg);
	wlab_itostrf(tact_str, buffer->sample_ts_val);
	wlab_itostrf(tmin_str, buffer->_min);
	wlab_itostrf(tmax_str, buffer->_max);

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
			buffer->_min_ts,
			buffer->_max_ts
	);
	return(rc);
}

static int wlab_pt_authorize(void) {
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
			CONFIG_WLAB_PT_DESC,
			MAC_ADDR_STR
	);
	return(rc);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
