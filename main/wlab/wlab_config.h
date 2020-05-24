/*  --------------------------------------------------------------------------
 *  wlab_esp: wlab_config.h
 *  Created on: 24 may 2020
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#ifndef WLAB_CONFIG_H_
#define WLAB_CONFIG_H_

#define CONFIG_WLAB_PT_DESC						("PT100")
#define CONFIG_WLAB_DHT_DESC					("DHT21")

#define CONFIG_WLAB_PUB_PERIOD				(10) 	// minutes
#define CONFIG_WLAB_MEASURE_PERIOD		(4)		// secs

#define CONFIG_WLAB_PUB_TOPIC					("/wlabdb")
#define CONFIG_WLAB_AUTH_TOPIC				("/wlabauth")

#endif /* WLAB_CONFIG_H_ */

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
