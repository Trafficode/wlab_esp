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
#include "wlab_dht.h"

static void wlab_dht_task(void *arg);

extern logger_t mlog;
extern char MAC_ADDR_STR[13];

static logger_t dhtlog;
static dht_t dht21;

void wlab_dht_init(void) {
	dhtlog.init.level = loggerLevelDebug;
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
	xTaskCreate(wlab_dht_task, "wlabdht_task", 2048, NULL, 10, NULL);
}

static void wlab_dht_task(void *arg) {
	int16_t temp=0, rh=0;

	logger_cri(&dhtlog, "%s, wlab dht task started\n", __FUNCTION__);
	for(;;) {
		vTaskDelay(4000);
		if(true == dht_read(&dht21, &temp, &rh)) {
			logger_cri(&dhtlog, "%s, temp=%d rh=%d\n", __FUNCTION__, temp, rh);
		} else {
			logger_cri(&dhtlog, "%s, dht read failed...\n", __FUNCTION__);
		}
	}
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
