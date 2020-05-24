/*  --------------------------------------------------------------------------
 *  wlab_esp: leds.h  
 *  Created on: 11 sie 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#ifndef COMPONENTS_PERIPHERALS_LEDS_H_
#define COMPONENTS_PERIPHERALS_LEDS_H_

#define LED_TOOGLE_FAST		(0xAAAA)
#define LED_TOOGLE_SLOW		(0xF0F0)
#define LED_ON						(0x0000)
#define LED_OFF						(0xFFFF)

typedef enum {
	LED_MQTT = 2
} led_t;

void leds_init(void);
void led_set_state(led_t led, uint16_t state);

#endif /* COMPONENTS_PERIPHERALS_LEDS_H_ */

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
