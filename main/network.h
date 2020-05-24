/*  --------------------------------------------------------------------------
 *  wlab_esp: network.h  
 *  Created on: 28 wrz 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#ifndef MAIN_NETWORK_H_
#define MAIN_NETWORK_H_

#define CONFIG_MQTT_MAX_PUBLISH_LEN		(512)

void mqtt_init(void);
void wifi_init(void);
void sntp_wait(void);

int mqtt_printf(const char *topic, uint32_t timeout,  const char *fmt, ...);

#endif /* MAIN_NETWORK_H_ */

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
