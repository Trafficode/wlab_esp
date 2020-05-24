/*  --------------------------------------------------------------------------
 *  wlab_esp: user_config.h
 *  Created on: 28 wrz 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

// 1 STEP
// Customize Your own configuration

// 2 STEP
// Change file name from user_config_template.h to user_config.h

// 3 STEP
// Build binary and program

#ifndef MAIN_USER_CONFIG_H_
#define MAIN_USER_CONFIG_H_

#define CONFIG_WIFI_SSID					("BALMONT_2.99230")
#define CONFIG_WIFI_PASS					("UUUJHPUU")

#define CONFIG_MQTT_BROKER						("mqtt://iot.eclipse.org")
#define CONFIG_MQTT_BROKER_PORT				(1883)
#define CONFIG_WLAB_LATITIUDE					("45.001")
#define CONFIG_WLAB_LONGITUDE					("25.001")
#define CONFIG_WLAB_NAME							("Name")
#define CONFIG_WLAB_TIMEZONE					("Europe/Warsaw")

#endif /* MAIN_USER_CONFIG_H_ */

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
