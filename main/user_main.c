/*  --------------------------------------------------------------------------
 *  wlab_esp: user_main.c
 *  Created on: 29 sie 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "mqtt_client.h"
#include "lwip/apps/sntp.h"

#include "logger.h"
#include "uart_debug.h"
#include "leds.h"
#include "config.h"
#include "user_config.h"
#include "wlab_dht.h"
#include "network.h"
#include "terminal.h"
#include "wlab/wlab_pt.h"


static void main_task(void *arg);

uint8_t MAC_ADDR[6];
char MAC_ADDR_STR[13];
logger_t mlog;

#define CONFIG_PRINT_BUFF_SIZE				(256)
static uint8_t print_buffer[CONFIG_PRINT_BUFF_SIZE];

static void main_task(void *arg) {
// SNTP sync period can be set in sntp_opts.h file
// #define SNTP_UPDATE_DELAY					(20000) // milliseconds
	sntp_wait();

#if CONFIG_SENSOR_PT == 1
	wlab_pt_start();
#elif CONFIG_SENSOR_DHT21 == 1
	wlab_dht_start();
#else
	;
#endif

	for(;;) {
		vTaskDelay(1000);
	}
}

/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
******************************************************************************/
void app_main(void) {
	nvs_flash_init();
	uart_debug_init();

	mlog.init.buff = print_buffer;
	mlog.init.buff_size = CONFIG_PRINT_BUFF_SIZE;
	mlog.init.putd = uart_debug_send;
	mlog.init.level = loggerLevelDebug;
	mlog.init.name = "main";
	logger_init(&mlog, NULL);

	esp_efuse_mac_get_default(MAC_ADDR);
	sprintf(MAC_ADDR_STR, "%02X%02X%02X%02X%02X%02X", MAC_ADDR[5],
			MAC_ADDR[4], MAC_ADDR[3], MAC_ADDR[2], MAC_ADDR[1], MAC_ADDR[0]);
	logger_cri(&mlog, "MAC_ADDR: %s\n", MAC_ADDR_STR);

	leds_init();
	terminal_init();

#if CONFIG_SENSOR_PT == 1
	wlab_pt_init();
#elif CONFIG_SENSOR_DHT21 == 1
	wlab_dht_init();
#else
	;
#endif

	wifi_init();	// function blocking until connection
	mqtt_init();	// function blocking until connection

	xTaskCreate(main_task, "main_task", 2*1024, NULL, 10, NULL);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
