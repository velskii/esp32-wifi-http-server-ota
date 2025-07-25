#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include "dht.h"
#include "dht11.h"
#include "tasks_common.h"

/**
 * DHT11 Sensor task
 */
static void DHT11_task(void *pvParameter)
{
	// printf("Starting DHT task\n\n");

	for (;;)
	{
		// printf("=== Reading DHT ===\n");
		int16_t temperature = 0;
		int16_t humidity = 0;

		esp_err_t res = dht_read_data(DHT_TYPE_DHT11, DHT_GPIO_PIN, &humidity, &temperature);
		if (res == ESP_OK) {
				printf("🌡️ Temp: %.1f°C\n", temperature / 10.0);
				printf("💧 Hum: %.1f%%\n", humidity / 10.0);
		} else {
				printf("❌ DHT read error: %d\n", res);
		}


		// Wait at least 2 seconds before reading again
		// The interval of the whole process must be more than 2 seconds
		vTaskDelay(4000 / portTICK_PERIOD_MS);
	}
}

void DHT11_task_start(void)
{
	xTaskCreatePinnedToCore(&DHT11_task, "DHT11_task", DHT11_TASK_STACK_SIZE, NULL, DHT11_TASK_PRIORITY, NULL, DHT11_TASK_CORE_ID);
}
