/*  --------------------------------------------------------------------------
 *  wlab_esp: dht.c
 *  Created on: 16 may 2020
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#ifndef COMPONENTS_OBJECTS_DHT_H_
#define COMPONENTS_OBJECTS_DHT_H_


// DHT21
//  					_				   ____         _         ___               ___
// \				 / \        /		 \ Tlow  / \ Tlow  /	 \   | \  Ten  /
//  \	 Tbe  /   \ Trel /	    \     /   \     /     \  |  \     /
//   \_____/ Tgo \____/	Treh	 \___/ Thi \___/  Thi  \ |   \___/
//
//																						MIN		TYP		MAX		UNIT
//	Tbe 	  Host the start signal down time 	0.8 	1 		20  	mS
//	Tgo 	  Bus master has released time 			20 		30 		200 	μS
//	Trel 		Response to low time 							75 		80 		85  	μS
//	Treh 		In response to high time 					75 		80 		85  	μS
//	Tlow 		Signal "0", "1" low time 					48 		50 		55  	μS
//	Thi  		Signal "0" high time 							22 		26 		30  	μS
//	Thi 	  Signal "1" high time 							68 		70 		75  	μS
//	Ten 	  Sensor to release the bus time 		45 		50 		55  	μS

#include "logger.h"

typedef enum {
		TYPE_DHT21,
} dht_type_t;

typedef struct {
	struct {
		logger_t *log;
		dht_type_t type;
		void (*data_pin_mode_input)(void);
		void (*data_pin_mode_output)(void);
		int32_t (*data_pin_get)(void);
		void (*data_pin_set)(int32_t level);
	} init;
} dht_t;

int32_t dht_init(dht_t *self);
int32_t dht_read(dht_t *self, int16_t *temp, int16_t *rh);

#endif /* COMPONENTS_OBJECTS_DHT_H_ */
/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
