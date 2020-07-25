/*  --------------------------------------------------------------------------
 *  wlab_esp: wlab_dht21.h
 *  Created on: 29 sie 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/hw_timer.h"

#include "logger.h"
#include "dht_pin.h"
#include "dht.h"
#include "leds.h"
#include "wlab_common.h"
#include "mqtt_client.h"
#include "../network.h"
#include "../user_config.h"

#include "wlab_dht.h"

static void wlab_dht_task(void *arg);

extern logger_t mlog;
extern char MAC_ADDR_STR[13];

const char *auth_template =	\
		"{\"timezone\":\"%s\",\"longitude\":%s,\"latitude\":%s,\"serie\":" \
		"{\"Temperature\":%d,\"Humidity\":%d}," \
		"\"name\":\"%s\",\"description\":\"%s\", \"uid\":\"%s\"}";

const char *data_template = \
		"{\"UID\":\"%s\",\"TS\":%d,\"SERIE\":{\"Temperature\":" \
		"{\"f_avg\":%s,\"f_act\":%s,\"f_min\":%s,\"f_max\":%s," \
		"\"i_min_ts\":%u,\"i_max_ts\":%u}," \
    "\"Humidity\":{\"f_avg\":%s,\"f_act\":%s,\"f_min\":%s," \
    "\"f_max\":%s,\"i_min_ts\":%u,\"i_max_ts\":%u}}}";

static int wlab_dht_authorize(void);
static int wlab_dht_publish_sample(buffer_t *temp, buffer_t *rh);

static logger_t dhtlog;
static dht_t dht21;

static buffer_t temp_buffer, rh_buffer;

void wlab_dht_init(void) {
	dhtlog.init.level = loggerLevelInfo;
	dhtlog.init.name = "wlabdht";
	logger_init(&dhtlog, &mlog);

	dht_gpio_pin_init();

	dht21.init.data_pin_get = dht_gpio_pin_get;
	dht21.init.data_pin_mode_input = dht_gpio_pin_mode_input;
	dht21.init.data_pin_mode_output = dht_gpio_pin_mode_output;
	dht21.init.data_pin_set = dht_gpio_pin_set;
	dht21.init.log = &dhtlog;
	dht21.init.type = TYPE_DHT21;
	dht_init(&dht21);
}

void wlab_dht_start(void) {
	xTaskCreate(wlab_dht_task, "wlabdht_task", 2*1024, NULL, 10, NULL);
}

static void wlab_dht_task(void *arg) {
	time_t now=0;
	struct tm timeinfo;
	uint32_t last_minutes=0;
	int16_t temp=0, rh=0;

	wlab_buffer_init(&temp_buffer);
	wlab_buffer_init(&rh_buffer);

	wlab_dht_authorize();

	logger_cri(&dhtlog, "%s, task started\n", __FUNCTION__);
	vTaskDelay(1000);

	for(;;) {
		vTaskDelay(1000*CONFIG_WLAB_MEASURE_PERIOD);
		time(&now);
		localtime_r(&now, &timeinfo);

		if(!dht_read(&dht21, &temp, &rh)) {
			logger_info(&dhtlog, "%s, temp:%d rh:%d\n", __FUNCTION__, temp, rh);
			if(500 < temp || 1000 < rh) {
				logger_error(&dhtlog, "%s, Failed to validate data.\n", __FUNCTION__);
				continue;
			}
		} else {
			logger_error(&dhtlog, "%s, DHT21 failed.\n", __FUNCTION__);
			continue;
		}

		if(0x00 == timeinfo.tm_min % CONFIG_WLAB_PUB_PERIOD
				&& timeinfo.tm_min != last_minutes)
		{
			int32_t temp_avg = temp_buffer.buff/temp_buffer.cnt;
			logger_info(&dhtlog, "temp - min: %d max: %d avg: %d\n",
													temp_buffer._min, temp_buffer._max, temp_avg);

			int32_t rh_avg = rh_buffer.buff/rh_buffer.cnt;
			logger_info(&dhtlog, "rh - min: %d max: %d avg: %d\n",
													rh_buffer._min, rh_buffer._max, rh_avg);

			logger_cri(&dhtlog, "Sample ready to send ...\n");
			int rc = wlab_dht_publish_sample(&temp_buffer, &rh_buffer);
			if(0 != rc) {
				logger_error(&dhtlog, "%s, publish sample failed rc:%d\n",
							__FUNCTION__, rc);
				led_set_state(LED_MQTT, LED_TOOGLE_SLOW);
			} else {
				logger_info(&dhtlog, "%s, publish sample success\n", __FUNCTION__);
			}

			wlab_buffer_init(&temp_buffer);
			wlab_buffer_init(&rh_buffer);
			last_minutes = timeinfo.tm_min;
		} else {
			wlab_buffer_commit(&temp_buffer, temp, now, 8);
			wlab_buffer_commit(&rh_buffer, rh, now, 40);
		}
	}
}

static int wlab_dht_publish_sample(buffer_t *temp, buffer_t *rh) {
	int rc=0;
	int32_t temp_avg=0, rh_avg=0;
	char tavg_str[8], tact_str[8], tmin_str[8], tmax_str[8];
	char rhavg_str[8], rhact_str[8], rhmin_str[8], rhmax_str[8];

	temp_avg = temp->buff/temp->cnt;
	wlab_itostrf(tavg_str, temp_avg);
	wlab_itostrf(tact_str, temp->sample_ts_val);
	wlab_itostrf(tmin_str, temp->_min);
	wlab_itostrf(tmax_str, temp->_max);

	rh_avg = rh->buff/rh->cnt;
	wlab_itostrf(rhavg_str, rh_avg);
	wlab_itostrf(rhact_str, rh->sample_ts_val);
	wlab_itostrf(rhmin_str, rh->_min);
	wlab_itostrf(rhmax_str, rh->_max);

	rc = mqtt_printf(
			CONFIG_WLAB_PUB_TOPIC,
			6000,
			data_template,
			MAC_ADDR_STR,
			temp->sample_ts,
			tavg_str,
			tact_str,
			tmin_str,
			tmax_str,
			temp->_min_ts,
			temp->_max_ts,
			rhavg_str,
			rhact_str,
			rhmin_str,
			rhmax_str,
			rh->_min_ts,
			rh->_max_ts
	);
	return(rc);
}

static int wlab_dht_authorize(void) {
	int rc=0;
	logger_info(&dhtlog, "%s, wlab_authorize()\n", __FUNCTION__);

	rc = mqtt_printf(
			CONFIG_WLAB_AUTH_TOPIC,
			4000,
			auth_template,
			CONFIG_WLAB_TIMEZONE,
			CONFIG_WLAB_LATITIUDE,
			CONFIG_WLAB_LONGITUDE,
			WLAB_TEMP_SERIE,
			WLAB_HUMIDITY_SERIE,
			CONFIG_WLAB_NAME,
			CONFIG_WLAB_DHT_DESC,
			MAC_ADDR_STR
	);
	return(rc);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
