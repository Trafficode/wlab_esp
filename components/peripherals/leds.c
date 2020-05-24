/*  --------------------------------------------------------------------------
 *  wlab_esp: leds.c  
 *  Created on: 11 sie 2019
 *
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

#include "logger.h"
#include "leds.h"

extern logger_t mlog;

static void leds_task(void *arg);

static uint16_t mqtt_led_state = LED_OFF;
static logger_t ledlog;

void leds_init(void) {
    gpio_config_t io_conf;

    ledlog.init.level = loggerLevelDebug;
    ledlog.init.name = "leds";
    logger_init(&ledlog, &mlog);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL<<LED_MQTT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    xTaskCreate(leds_task, "leds_task", 1024, NULL, 10, NULL);
}

void led_set_state(led_t led, uint16_t state) {
	if(LED_MQTT == led) {
		mqtt_led_state = state;
	} else {
		logger_error(&ledlog, "led 0x%04X not registerd\n", led);
	}
}

static void leds_task(void *arg) {
	uint32_t leds_status_idx=0;
	logger_cri(&ledlog, "leds task started\n");
    for(;;) {
    	for(leds_status_idx=0; leds_status_idx<16; leds_status_idx++) {
    		if((1<<leds_status_idx) & mqtt_led_state) {
    			gpio_set_level(LED_MQTT, 1);
    		} else {
    			gpio_set_level(LED_MQTT, 0);
    		}
    		vTaskDelay(125);
    	}
    }
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */



