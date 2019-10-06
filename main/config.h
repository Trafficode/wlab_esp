/*  --------------------------------------------------------------------------
 *  wlab_esp: config.h  
 *  Created on: 30 sie 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#ifndef MAIN_CONFIG_H_
#define MAIN_CONFIG_H_

// SNTP sync period can be set in sntp_opts.h file
// #define SNTP_UPDATE_DELAY					(20000) // milliseconds


#define CONFIG_MQTT_MAX_PUBLISH_LEN		(256)

#define CONFIG_WLAB_PUB_PERIOD				(10) // minutes

#define CONFIG_WLAB_DESC							("PT100")
#define CONFIG_WLAB_PUB_TOPIC					("/wlabdb")
#define CONFIG_WLAB_AUTH_TOPIC				("/wlabauth")

#define CONFIG_PRINT_BUFF_SIZE				(256)

#endif /* MAIN_CONFIG_H_ */

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
