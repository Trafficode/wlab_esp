/*  --------------------------------------------------------------------------
 *  max31865.c
 *  Author: Trafficode
 *  ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdint.h>

#include "logger.h"
#include "max31865.h"

static uint8_t _addr_read(max31865_t *self, uint8_t _addr);
static void _addr_write(max31865_t *self, uint8_t _addr, uint8_t _value);

static int32_t _adc_convert(max31865_t *self, uint16_t _adc_value);

static void max31865_delay(void) {
	vTaskDelay(1);
}

int32_t max31865_init(max31865_t *self) {
	self->init.cs_pin_write(1);
	self->init.clk_pin_write(1);
	max31865_delay();

	_addr_write(self, 0x80, 0xC2);

	return(0);
}

int32_t max31865_get(max31865_t *self, int32_t *_tmul) {
	uint16_t lsb=0, msb=0, raw=0, fault=0;
	int32_t rc=0;

	fault = _addr_read(self, 0x07);
	if(0 != fault) {
		logger_error(self->init.log, "%s, max31865 -> fault: %02X\n", __FUNCTION__,
										fault);
		rc = -1;
		goto exit;
	}

	msb = (uint16_t)_addr_read(self, 0x01);
	lsb = (uint16_t)_addr_read(self, 0x02);

	raw = (msb<<8) | lsb;
	raw /= 2;
	logger_debug(self->init.log, "max31865 -> raw: %04X\n", raw);
	if(0 == raw) {
		rc = 1;	/* Read 0 Ohms */
		logger_error(self->init.log, "max31865 -> read 0 ohms\n");
	} else {
		*_tmul = _adc_convert(self, raw);
	}

exit:
	return(rc);
}

static uint8_t _addr_read(max31865_t *self, uint8_t _addr) {
	uint8_t _value=0;
	int32_t idx=0, bit=0;

	self->init.cs_pin_write(0);
	self->init.clk_pin_write(0);
	max31865_delay();

	for(idx=0; idx<8; idx++) {	// Write address
		bit = _addr >> (7 - idx);
		bit = bit & 1;
		self->init.mosi_pin_write(bit);
		max31865_delay();
		self->init.clk_pin_write(1);
		max31865_delay();
		self->init.clk_pin_write(0);
		max31865_delay();
	}

	for(idx=0; idx<8; idx++) {	// Read data
		self->init.clk_pin_write(1);
		max31865_delay();
		if(0 != self->init.miso_pin_read()) {
			_value |= (1<<(7-idx));
		}
		max31865_delay();
		self->init.clk_pin_write(0);
		max31865_delay();
	}

	self->init.cs_pin_write(1);
	self->init.clk_pin_write(1);
	max31865_delay();
	return(_value);
}

static void _addr_write(max31865_t *self, uint8_t _addr, uint8_t _value) {
	int32_t idx=0, bit=0;

	self->init.cs_pin_write(0);
	self->init.clk_pin_write(0);
	max31865_delay();

	for(idx=0; idx<8; idx++) {	// Write address
		bit = _addr >> (7 - idx);
		bit = bit & 1;
		self->init.mosi_pin_write(bit);
		max31865_delay();
		self->init.clk_pin_write(1);
		max31865_delay();
		self->init.clk_pin_write(0);
		max31865_delay();
	}

	for(idx=0; idx<8; idx++) {	// Read data
		bit = _value >> (7 - idx);
		bit = bit & 1;
		self->init.mosi_pin_write(bit);
		max31865_delay();
		self->init.clk_pin_write(1);
		max31865_delay();
		self->init.clk_pin_write(0);
		max31865_delay();
	}

	self->init.cs_pin_write(1);
	self->init.clk_pin_write(1);
	max31865_delay();
}

static int32_t _adc_convert(max31865_t *self, uint16_t _adc_value) {
	float temp=0;
	float R0 = ((float)_adc_value*self->init.refr)/32768;
	temp = -242.02 + 2.2228*R0 + 2.5859e-3*R0*R0 - 4.8260e-6*R0*R0*R0 -\
			2.8183e-8*R0*R0*R0*R0 + 1.5243e-10*R0*R0*R0*R0*R0;
	return (int32_t)(10*temp);
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
