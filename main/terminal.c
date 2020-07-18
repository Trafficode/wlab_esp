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
#include "cmdline.h"
#include "terminal.h"

#define TERM_ANS_BUFF_SIZE 	(256)
#define TERM_RCV_BUFF_SIZE 	(256)

extern logger_t mlog;
static logger_t tlog;

static void cmd_help(void *self, uint8_t *param);

cmd_t _cmd[] = {
		{"HELP", cmd_help, "Show all available commands and their description"}
};
uint32_t _cmds_num = sizeof(_cmd)/sizeof(_cmd[0]);

static void cmd_help(void *self, uint8_t *param) {
	uint32_t idx=0;

	cmdline_answer(self, "\nWeatherlab terminal %s\n", CONFIG_VERSION);
	for(idx=0; idx<_cmds_num; idx++) {
		cmdline_answer(self, "\t%s: %s\n", _cmd[idx].name, _cmd[idx].help);
	}
}

static cmdline_t uterm;
static uint8_t _ans_buff[TERM_ANS_BUFF_SIZE];
static uint8_t _rcv_buff[TERM_RCV_BUFF_SIZE];

static void _terminal_tsk(void *arg) {
	logger_info(&tlog, "_terminal_tsk start\n");
	for(;;) {
		cmdline_commit(&uterm);
	}
}

void terminal_init(void) {
	tlog.init.level = loggerLevelDebug;
	tlog.init.name = "uterm";
	logger_init(&tlog, &mlog);

	uterm.init.name = "uterm";
	uterm.init.getch = uart_debug_getc;
	uterm.init.putd = uart_debug_send;
	uterm.init.use_prompt = true;
	uterm.init.use_echo = true;
	uterm.init.cmds = _cmd;
	uterm.init.cmds_num = _cmds_num;
	uterm.init.ans_buff = _ans_buff;
	uterm.init.ans_buff_size = TERM_ANS_BUFF_SIZE;
	uterm.init.rcv_buff = _rcv_buff;
	uterm.init.rcv_buff_size = TERM_RCV_BUFF_SIZE;
	cmdline_init(&uterm);

	xTaskCreate(_terminal_tsk, "term_task", 2*1024, NULL, 10, NULL);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
