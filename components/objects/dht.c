/*  --------------------------------------------------------------------------
 *  wlab_esp: dht.c
 *  Created on: 16 may 2020
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "dht.h"
#include "logger.h"

#define T_BIT_L_MAX		(75)		/* T_BIT_L [50] us */
#define T_BIT_H_MAX		(100)		/* T_BIT_H 0: 26us 1:70us */

#define T_GO_MAX			(240)		/* T_GO [20, 30, 200] us */
#define T_REL_MAX			(100)		/* T_REL [75, 80, 85] us */
#define T_REH_MAX			(100)		/* T_REH [75, 80, 85] us */

static uint8_t dht_checksum(dht_t *self, uint16_t temp, uint16_t rh);
static bool dht_read_bit(dht_t *self, int32_t *bit);
static bool dht_pin_state_wait(
	dht_t *self,
	uint32_t timeout,
	uint32_t exp_pin_state,
	uint32_t *duration
);

#define log(self)										(self->init.log)
#define data_pin_mode_output(self)	(self->init.data_pin_mode_output())
#define data_pin_mode_input(self)		(self->init.data_pin_mode_input())
#define data_pin_get(self)					(self->init.data_pin_get())
#define data_pin_set(self, state)		(self->init.data_pin_set(state))


int32_t dht_init(dht_t *self) {
	self->failed_counter = 0;
	self->init.power_pin_set(1);
	return(0);
}

int32_t dht_read(dht_t *self, int16_t *temp, int16_t *rh) {
	int32_t rc=1;
	int32_t bit=0, bcnt=0;
	uint8_t crc=0, computed_crc=0;
	bool bits_read_rc = true;

	*rh = 0;
	*temp = 0;

	logger_info(log(self), "%s\n", __FUNCTION__);

	if(4 == self->failed_counter) {
		logger_error(log(self), "reset sensor...\n");
		self->init.power_pin_set(0);
		vTaskDelay(2000);
		self->init.power_pin_set(1);
		self->failed_counter = 0;
	}

	data_pin_mode_output(self);
	vTaskDelay(2);
	data_pin_mode_input(self);
	do {
		self->failed_counter++;
		if(false == dht_pin_state_wait(self, T_GO_MAX, 0, NULL)) {
			logger_error(log(self), "T_GO_MAX exceed!, break...\n");
			break;
		}

		if(false == dht_pin_state_wait(self, T_REL_MAX, 1, NULL)) {
			logger_error(log(self), "T_REL_MAX exceed!, break...\n");
			break;
		}

		if(false == dht_pin_state_wait(self, T_REH_MAX, 0, NULL)) {
			logger_error(log(self), "T_REH_MAX exceed!, break...\n");
			break;
		}
		taskENTER_CRITICAL();
		for(bcnt=0; bcnt<40; bcnt++) {
			if(true == dht_read_bit(self, &bit)) {
				if(16 > bcnt) {
					*rh |= (bit << (15-bcnt));
				} else if(32 <= bcnt) {
					crc |= (bit << (7-(bcnt-32)));
				} else {
					*temp |= (bit << (15-(bcnt-16)));
				}
			} else {
				logger_error(log(self), "%s, bit %d read error\n",
						__FUNCTION__, bcnt);
				bits_read_rc = false;
				break;
			}
		}
		taskEXIT_CRITICAL();

		if(false == bits_read_rc) {
			break;
		}

		computed_crc = dht_checksum(self, *temp, *rh);
		if(crc != computed_crc || crc == 0) {
			logger_error(log(self), "%s, bad crc received 0x%02X, computed 0x%02X\n",
					__FUNCTION__, crc, computed_crc);
			break;
		}

		if((1<<15) & *temp) {
			*temp &= ~(1 << 15);
			*temp = -*temp;
		}
		self->failed_counter = 0;
		rc = 0;
	} while(0);

	return(rc);
}

static bool dht_read_bit(dht_t *self, int32_t *bit) {
	bool rc=false;
	uint32_t duration=0;

	do {
		if(false == dht_pin_state_wait(self, T_BIT_L_MAX, 1, &duration)) {
			logger_error(log(self), "T_REH_MAX exceed!, break...\n");
			break;
		}

		if(false == dht_pin_state_wait(self, T_BIT_H_MAX, 0, &duration)) {
			logger_error(log(self), "T_REH_MAX exceed!, break...\n");
			break;
		}

		*bit = (20 < duration) ? 1:0;
		rc = true;
	} while(0);

	return(rc);
}

static uint8_t dht_checksum(dht_t *self, uint16_t temp, uint16_t rh) {
	uint8_t checksum = 0;
	checksum = (uint8_t)(rh&0xFF) + (uint8_t)((rh>>8) & 0xFF) +
				(uint8_t)(temp&0xFF) + (uint8_t)((temp>>8) & 0xFF);
	return checksum;
}

static bool dht_pin_state_wait(
	dht_t *self,
	uint32_t timeout,
	uint32_t exp_pin_state,
	uint32_t *duration
) {
	bool rc=false;
	for (uint32_t i=0; i<timeout; i++) {
		os_delay_us(1);
		if (exp_pin_state == data_pin_get(self)) {
			if(NULL != duration) {
				*duration = i;
			}
			rc = true;
			break;
		}
	}
  return(rc);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
