/*  --------------------------------------------------------------------------
 *  wlab_esp: max31865_pins.h
 *  Created on: 29 sie 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#ifndef COMPONENTS_PERIPHERALS_MAX31865_PINS_H_
#define COMPONENTS_PERIPHERALS_MAX31865_PINS_H_

#define MAX_MOSI_PIN	(14)	// D5
#define MAX_MISO_PIN	(12)	// D6
#define MAX_CS_PIN		(13)	// D7
#define MAX_CLK_PIN		(5)		// D1

void max31865_pins_init(void);
void max31865_pins_cs_write(int32_t _state);
void max31865_pins_mosi_write(int32_t _state);
void max31865_pins_clk_write(int32_t _state);
int32_t max31865_pins_miso_read(void);

#endif /* COMPONENTS_PERIPHERALS_MAX31865_PINS_H_ */
/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
