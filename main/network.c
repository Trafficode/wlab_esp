/*  --------------------------------------------------------------------------
 *  wlab_esp: network.c  
 *  Created on: 28 wrz 2019
 *  	Author: Trafficode
 *  ------------------------------------------------------------------------ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_system.h"

#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "mqtt_client.h"
#include "lwip/apps/sntp.h"
#include "esp_event_loop.h"

#include "logger.h"
#include "uart_debug.h"
#include "leds.h"
#include "debug.h"
#include "config.h"
#include "user_config.h"
#include "wlab.h"

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event);

extern logger_t mlog;
static logger_t netlog;

static EventGroupHandle_t net_event_group;
const static int WIFI_CONNECTED_BIT = BIT0;
const static int MQTT_CONNECTED_BIT = BIT1;
const static int MQTT_PUBLISHED_BIT = BIT2;

static SemaphoreHandle_t mqtt_lock;
static char mqtt_pub_buffer[CONFIG_MQTT_MAX_PUBLISH_LEN];

static volatile esp_mqtt_event_id_t last_publish_id = 0;

esp_mqtt_client_handle_t mqttc;
static esp_mqtt_client_config_t mqtt_cfg = {
	.uri = CONFIG_MQTT_BROKER,
	.port = CONFIG_MQTT_BROKER_PORT,
	.event_handle = mqtt_event_handler
};

void wifi_init(void) {
	uint32_t events=0;

	netlog.init.level = loggerLevelDebug;
	netlog.init.name = "netlog";
  logger_init(&netlog, &mlog);

	tcpip_adapter_init();
	net_event_group = xEventGroupCreate();
	ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_WIFI_SSID,
			.password = CONFIG_WIFI_PASS,
		},
	};

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

	logger_info(&netlog, "%s, connect to %s...\n", __FUNCTION__,
								CONFIG_WIFI_SSID);
	ESP_ERROR_CHECK(esp_wifi_start());
	events = xEventGroupWaitBits(net_event_group, WIFI_CONNECTED_BIT, false,
																	true, 8000);
	if(events & WIFI_CONNECTED_BIT) {
		logger_info(&netlog, "%s, connect to %s success\n", __FUNCTION__,
										CONFIG_WIFI_SSID);
	} else {
		// reset device
		logger_error(&netlog, "%s, connect to %s failed, restart...\n",
										__FUNCTION__, CONFIG_WIFI_SSID);
		vTaskDelay(2000);
		esp_restart();
	}

	logger_info(&netlog, "EVENTS: 0x%08X\n", events);
}

void mqtt_init(void) {
	uint32_t events=0;

	mqtt_lock = xSemaphoreCreateMutex();
  if( NULL == mqtt_lock ) {
  	for(;;);
  }

  mqttc = esp_mqtt_client_init(&mqtt_cfg);
	logger_info(&netlog, "%s, connect to %s:%d...\n", __FUNCTION__,
								CONFIG_MQTT_BROKER, CONFIG_MQTT_BROKER_PORT);
	esp_mqtt_client_start(mqttc);

	events = xEventGroupWaitBits(net_event_group, MQTT_CONNECTED_BIT, false,
																		true, 8000);
	if(events & MQTT_CONNECTED_BIT) {
		logger_info(&netlog, "%s, connect to %s:%d success\n", __FUNCTION__,
									CONFIG_MQTT_BROKER, CONFIG_MQTT_BROKER_PORT);
	} else {
		// reset device
		logger_error(&netlog, "%s, connect to %s:%d failed, restart...\n",
									 __FUNCTION__, CONFIG_MQTT_BROKER, CONFIG_MQTT_BROKER_PORT);
		vTaskDelay(2000);
		esp_restart();
	}
}

void sntp_wait(void) {
	int retry = 0;
	const int retry_count = 16;
  time_t now = 0;
  struct tm timeinfo = { 0 };

	logger_cri(&netlog, "%s, initializing sntp...\n", __FUNCTION__);
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();

	while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
		logger_warn(&netlog, "%s, waiting for system time to be set... (%d/%d)\n",
										__FUNCTION__, retry, retry_count);
		vTaskDelay(2000);
		time(&now);
		localtime_r(&now, &timeinfo);
	}
}

int mqtt_printf(const char *topic, uint32_t timeout,  const char *fmt, ...) {
	int rc=0;
	va_list args;
	va_start(args, fmt);

	xSemaphoreTake( mqtt_lock, UINT32_MAX );

	rc = vsnprintf(mqtt_pub_buffer,
								 CONFIG_MQTT_MAX_PUBLISH_LEN,
								 fmt, args);

	if( 0 < rc && CONFIG_MQTT_MAX_PUBLISH_LEN > rc ) {
		xEventGroupClearBits(net_event_group, MQTT_PUBLISHED_BIT);
		last_publish_id = esp_mqtt_client_publish(mqttc,
													topic, mqtt_pub_buffer, rc, 1, 0);
		if(-1 != last_publish_id) {
			rc = xEventGroupWaitBits(net_event_group, MQTT_PUBLISHED_BIT, false,
																				true, timeout);

			if(rc & MQTT_PUBLISHED_BIT) {
				logger_info(&netlog, "%s, publish msg_id=%d success\n", __FUNCTION__,
											last_publish_id);
				rc = 0;		// Return success
			} else {
				logger_error(&netlog, "%s, publish msg_id=%d failed\n", __FUNCTION__,
											last_publish_id);
				rc = -2;	// ACK Timeout
			}
			xEventGroupClearBits(net_event_group, MQTT_PUBLISHED_BIT);
		} else {
			rc = -3; 		// MQTT not connected
		}
	} else {
		logger_error(&netlog, "%s, not enought space in publish buffer\n",
										__FUNCTION__);
		rc = -1;		// Buffer too small
	}

	xSemaphoreGive( mqtt_lock );

	va_end(args);
	return(rc);
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
	/* For accessing reason codes in case of disconnection */
	system_event_info_t *info = &event->event_info;

	switch (event->event_id) {
		case SYSTEM_EVENT_STA_START: {
			logger_info(&netlog, "%s, SYSTEM_EVENT_STA_START\n", __FUNCTION__);
			esp_wifi_connect();
			break;
		}
		case SYSTEM_EVENT_STA_GOT_IP: {
			xEventGroupSetBits(net_event_group, WIFI_CONNECTED_BIT);
			logger_info(&netlog, "%s, SYSTEM_EVENT_STA_GOT_IP\n", __FUNCTION__);
			break;
		}
		case SYSTEM_EVENT_STA_DISCONNECTED: {
			logger_error(&netlog, "%s, SYSTEM_EVENT_STA_DISCONNECTED reason: %d\n",
											__FUNCTION__, info->disconnected.reason);
			if (info->disconnected.reason ==
				WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
				/*Switch to 802.11 bgn mode */
				esp_wifi_set_protocol(ESP_IF_WIFI_STA,
					WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);
			}
			esp_wifi_connect();
			xEventGroupClearBits(net_event_group, WIFI_CONNECTED_BIT);
			break;
		}
		default:
				break;
	}

	return ESP_OK;
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {

	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED: {
			logger_info(&netlog, "%s, MQTT_EVENT_CONNECTED\n", __FUNCTION__);
			xEventGroupSetBits(net_event_group, MQTT_CONNECTED_BIT);
			break;
		}
		case MQTT_EVENT_DISCONNECTED: {
			logger_info(&netlog, "%s, MQTT_EVENT_DISCONNECTED\n", __FUNCTION__);
			xEventGroupClearBits(net_event_group, MQTT_CONNECTED_BIT);
			break;
		}
		case MQTT_EVENT_SUBSCRIBED:	{
			logger_info(&netlog, "%s, MQTT_EVENT_SUBSCRIBED, msg_id=%d\n",
										__FUNCTION__, event->msg_id);
			break;
		}
		case MQTT_EVENT_UNSUBSCRIBED: {
			logger_info(&netlog, "%s, MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n",
										__FUNCTION__, event->msg_id);
			break;
		}
		case MQTT_EVENT_PUBLISHED: {
			logger_info(&netlog, "%s, MQTT_EVENT_PUBLISHED, msg_id=%d\n",
										__FUNCTION__, event->msg_id);
			if(event->msg_id == last_publish_id) {
				logger_info(&netlog, "%s, received properly msg_id=%d\n",
														__FUNCTION__, event->msg_id);
				xEventGroupSetBits(net_event_group, MQTT_PUBLISHED_BIT);
			}
			break;
		}
		case MQTT_EVENT_DATA: {
			logger_info(&netlog, "%s, MQTT_EVENT_DATA\n", __FUNCTION__);
			break;
		}
		case MQTT_EVENT_ERROR: {
			logger_error(&netlog, "%s, MQTT_EVENT_ERROR\n", __FUNCTION__);
			break;
		}
		default: {
			logger_error(&netlog, "%s, MQTT_EVENT - default...\n", __FUNCTION__);
			break;
		}
	}
	return ESP_OK;
}

/*  --------------------------------------------------------------------------
 *  end of file
 *  ------------------------------------------------------------------------ */




