/*  --------------------------------------------------------------------------
 *  wlab_esp: user_config.h
 *  Created on: 28 wrz 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */

#ifndef MAIN_USER_CONFIG_H_
#define MAIN_USER_CONFIG_H_

#define CONFIG_VERSION								("wlab_esp_1.0")

#define CONFIG_SENSOR_DHT21						(1)
#define CONFIG_DHT_GPIO_PIN						(4)			// D2
#define CONFIG_DHT_GPIO_PIN_POWER			(12)		// D6

// Zlocien
//#define CONFIG_WIFI_SSID							("BALMONT_2.99230")
//#define CONFIG_WIFI_PASS							("UUUJHPUU")
//#define CONFIG_CUSTOM_MAC							("")
//#define CONFIG_WLAB_NAME							("Flatek")

// ADUDA
#define CONFIG_WIFI_SSID							("WLAN1-002333")
#define CONFIG_WIFI_PASS							("814183DF326FD69")
#define CONFIG_CUSTOM_MAC							("A020A61259E8")
#define CONFIG_WLAB_NAME							("Krakers")

#define CONFIG_MQTT_BROKER						("mqtt://194.42.111.14")
#define CONFIG_MQTT_BROKER_PORT				(1883)

#define CONFIG_WLAB_LATITIUDE					("50.019556")
#define CONFIG_WLAB_LONGITUDE					("20.057409")
#define CONFIG_WLAB_TIMEZONE					("Europe/Warsaw")

#define CONFIG_SENSOR_PT							(0)

#endif /* MAIN_USER_CONFIG_H_ */

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */
