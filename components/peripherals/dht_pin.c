/*  --------------------------------------------------------------------------
 *  wlab_esp: dht_pin.c
 *  Created on: 24 may 2020
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#include <stdlib.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "../user_config.h"
#include "dht_pin.h"

void dht_gpio_pin_init(void) {
	gpio_config_t io_dht;

	io_dht.intr_type = GPIO_INTR_DISABLE;
	io_dht.mode = GPIO_MODE_OUTPUT;
	io_dht.pin_bit_mask = 1 << CONFIG_DHT_GPIO_PIN;
	io_dht.pull_down_en = 0;
	io_dht.pull_up_en = 0;
	gpio_config(&io_dht);

	io_dht.pin_bit_mask = 1 << CONFIG_DHT_GPIO_PIN_POWER;
	gpio_config(&io_dht);
}

void dht_gpio_pin_power_set(int32_t level) {
	if(0 == level) {
		gpio_set_level(CONFIG_DHT_GPIO_PIN_POWER, 0);
	} else {
		gpio_set_level(CONFIG_DHT_GPIO_PIN_POWER, 1);
	}
}

int32_t dht_gpio_pin_get(void) {
	return (int32_t)gpio_get_level(CONFIG_DHT_GPIO_PIN);
}

void dht_gpio_pin_set(int32_t level) {
	if(0 == level) {
		gpio_set_level(CONFIG_DHT_GPIO_PIN, 0);
	} else {
		gpio_set_level(CONFIG_DHT_GPIO_PIN, 1);
	}
}

void dht_gpio_pin_mode_input(void) {
	gpio_set_direction(CONFIG_DHT_GPIO_PIN, GPIO_MODE_INPUT);
}

void dht_gpio_pin_mode_output(void) {
	gpio_set_direction(CONFIG_DHT_GPIO_PIN, GPIO_MODE_OUTPUT);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
