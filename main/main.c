/**
 * @file main.c
 * @brief Main application entry point for the RGB LED Wi-Fi application.
 * Created on: July 2, 2025
 * Author: Greg
 */

#include "nvs_flash.h"
#include "wifi_app.h"
#include "dht11.h"


void app_main(void)
{
/**
 * rgb_led test functions 
 */
// while (true)
// {
// rgb_led_wifi_app_started();
// vTaskDelay(1000 / portTICK_PERIOD_MS);
// rgb_led_http_server_started();
// vTaskDelay(1000 / portTICK_PERIOD_MS);
// rgb_led_wifi_connected();
// vTaskDelay(1000 / portTICK_PERIOD_MS);
// }

// Initialize NVS (Non-Volatile Storage) for storing Wi-Fi credentials and other settings
esp_err_t ret = nvs_flash_init();
if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
}
ESP_ERROR_CHECK(ret);

// Start WiFi
wifi_app_start();

// Start DHT11 sensor task
DHT11_task_start();

}
