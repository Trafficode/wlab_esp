/*  --------------------------------------------------------------------------
 *  wlab_esp: user_config.h
 *  Created on: 28 wrz 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#ifndef USER_CONFIG_H_
#define USER_CONFIG_H_

/**
 * 1 STEP
 * Customize Your own configuration
 * 2 STEP
 * Change file name from user_config_template.h to user_config.h
 * 3 STEP
 * Build binary and program
 */

#define CONFIG_VERSION			("wlab_esp_1.1")

#define CONFIG_SENSOR_DHT21		(1)
#define CONFIG_SENSOR_PT		(0)

#define CONFIG_DHT_GPIO_PIN		(4)		// D2
#define CONFIG_DHT_GPIO_PIN_POWER	(12)		// D6

#define CONFIG_WIFI_SSID		("ssid")
#define CONFIG_WIFI_PASS		("pass")

/* Set custom mac will couse that mac wont be taken from
 * hw address but from that definition */
#define CONFIG_CUSTOM_MAC		("")
#define CONFIG_WLAB_NAME		("Station Name")

#define CONFIG_MQTT_BROKER		("mqtt://hostname")
#define CONFIG_MQTT_BROKER_PORT		(1883)

#define CONFIG_WLAB_LATITIUDE		("40")
#define CONFIG_WLAB_LONGITUDE		("30")
#define CONFIG_WLAB_TIMEZONE		("Europe/Warsaw")

#endif /* USER_CONFIG_H_ */

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
