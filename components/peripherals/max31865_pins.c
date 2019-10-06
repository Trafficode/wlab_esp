/*  --------------------------------------------------------------------------
 *  wlab_esp: max31865_pins.c
 *  Created on: 29 sie 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#include <stdlib.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "max31865_pins.h"

void max31865_pins_init(void) {
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL<<MAX_MOSI_PIN | 1ULL<<MAX_CS_PIN |
    						1ULL<<MAX_CLK_PIN;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL<<MAX_MISO_PIN;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

void max31865_pins_cs_write(int32_t _state) {
	gpio_set_level(MAX_CS_PIN, _state);
}

void max31865_pins_mosi_write(int32_t _state) {
	gpio_set_level(MAX_MOSI_PIN, _state);
}

void max31865_pins_clk_write(int32_t _state) {
	gpio_set_level(MAX_CLK_PIN, _state);
}

int32_t max31865_pins_miso_read(void) {
	return(gpio_get_level(MAX_MISO_PIN));
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */




