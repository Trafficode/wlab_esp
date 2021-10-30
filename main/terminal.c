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
#include "terminal.h"

static void cmd_help(void *self, uint8_t *param);
static void cmd_push(void *self, uint8_t *param);
static void cmd_test(void *self, uint8_t *param);

cmd_t _cmd[] = {
		{"HELP", cmd_help, "User commands <help>"},
		{"PUSH", cmd_push, "Send sensor data to server <push>"},
		{"TEST", cmd_test, "Wlab buffer test <test>"}
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
	int32_t idx = 0, samples[8] = {40, 10, 20, 50, 44, 41, 42, 1};
	buffer_t buff;

	cmdline_answer(self, "Test wlab buffer_size=%d\n", sizeof(buff));
	cmdline_answer(self, "WLAB_SAMPLE_BUFFER_SIZE: %d\n",
												WLAB_SAMPLE_BUFFER_SIZE);
	wlab_buffer_init(&buff);

	for(idx=0; idx<8; idx++) {
		if(wlab_buffer_commit(&buff, samples[idx], 100+idx, 8)) {
			cmdline_answer(self, " Sample %d append success\n", idx);
		} else {
			cmdline_answer(self, " Sample %d append failed\n", idx);
		}
	}
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
