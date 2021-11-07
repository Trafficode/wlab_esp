/*  --------------------------------------------------------------------------
 *  wlab_esp: terminal.c
 *  Created on: 18 july 2020
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#include <stdio.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "uart_debug.h"
#include "user_config.h"
#include "wlab_common.h"
#include "wlab_dht.h"
#include "wlab_pt.h"
#include "dht.h"
#include "terminal.h"

static void cmd_test(void *self, uint8_t *param);

static void cmd_help(void *self, uint8_t *param);
static void cmd_push(void *self, uint8_t *param);
static void cmd_dhtr(void *self, uint8_t *param);

cmd_t _cmd[] = {
		{"HELP", cmd_help, "User commands <help>"},
		{"DHTR", cmd_dhtr, "Wlab buffer test <test>"},

		{"PUSH", cmd_push, "Send sensor data to server <push>"},
		{"TEST", cmd_test, "Read dht data <dhtr>"}		
};

uint32_t _cmds_num = sizeof(_cmd)/sizeof(_cmd[0]);
//extern logger_t mlog;

static void cmd_help(void *self, uint8_t *param) {
	uint32_t idx=0;

	cmdline_answer(self, "\nWeatherlab terminal %s\n", CONFIG_VERSION);

	for(idx=0; idx<_cmds_num; idx++) {
		cmdline_answer(self, "\t%s: %s\n", _cmd[idx].name, _cmd[idx].help);
	}
}

static void cmd_push(void *self, uint8_t *param) {
	int rc = -1;
#if CONFIG_SENSOR_PT
	rc = wlab_pt_push();
#elif CONFIG_SENSOR_DHT21
	rc = wlab_dht_push();
#else
	;
#endif
	cmdline_answer(self, "\nrc = %d\n", rc);
}


static void cmd_test(void *self, uint8_t *param) {
	cmdline_answer(self, "TICK ");
	vTaskDelay(10000);
	cmdline_answer(self, "TACK\n", configMAX_PRIORITIES );
}

dht_t dht21;

static void cmd_dhtr(void *self, uint8_t *param) {
	int32_t rc=0;
	int16_t temp=0, rh=0;
	
	rc = dht_read(&dht21, &temp, &rh);
	cmdline_answer(self, "rc %d temp %d rh %d\n", rc, temp, rh);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
