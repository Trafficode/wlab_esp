/*  --------------------------------------------------------------------------
 *  wlab_esp: dht_pin.h
 *  Created on: 24 may 2020
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#ifndef COMPONENTS_PERIPHERALS_DHT_PIN_H_
#define COMPONENTS_PERIPHERALS_DHT_PIN_H_

#include "driver/gpio.h"

#define DHT_GPIO_PIN		(4)				// D2

void dht_gpio_pin_init(void);
void dht_gpio_pin_set(int32_t level);
void dht_gpio_pin_mode_input(void);
void dht_gpio_pin_mode_output(void);

int32_t dht_gpio_pin_get(void);

#endif /* COMPONENTS_PERIPHERALS_DHT_PIN_H_ */
/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
