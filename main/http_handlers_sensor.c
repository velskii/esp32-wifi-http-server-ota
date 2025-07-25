#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_http_server.h"

#include "http_handlers_sensor.h"
#include "dht.h"
#include "dht11.h"

static const char *TAG = "HTTP_HANDLERS_SENSOR";

/**
 * Handles the request for the DHT sensor readings in JSON format.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK if successful, otherwise ESP_FAIL if timeout occurs and the update cannot be started.
 */
esp_err_t http_server_get_dht_sensor_readings_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/dhtSensor.json requested");
	
	char dhtSensorJSON[100];
	int16_t temperature = 0;
	int16_t humidity = 0;

	if (dht_read_data(DHT_TYPE_DHT11, DHT_GPIO_PIN, &humidity, &temperature) == ESP_OK)
	{
		snprintf(dhtSensorJSON, sizeof(dhtSensorJSON),
							"{\"temperature\": %.1f, \"humidity\": %.1f }",
							temperature / 10.0, humidity / 10.0);
	}
	else
	{
		snprintf(dhtSensorJSON, sizeof(dhtSensorJSON),
							"{\"error\": \"DHT sensor read failed\"}");
	}
	
	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, dhtSensorJSON, strlen(dhtSensorJSON));

	return ESP_OK;
}