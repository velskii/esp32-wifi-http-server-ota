/** @file http_server.c
 *  Created on: July 2, 2025
 *  Author: Greg
 */

#include "esp_log.h"
#include "esp_http_server.h"

#include "http_server.h"
#include "http_handlers_static.h"
#include "http_handlers_wifi.h"
#include "http_handlers_ota.h"
#include "http_handlers_sensor.h"
#include "http_server_monitor.h"
#include "http_handlers_sntp.h"
#include "tasks_common.h"

static const char TAG[] = "http_server";

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t task_http_server_monitor = NULL;

// HTTP server message queue, used to manipulate the main queue of events
QueueHandle_t http_server_monitor_queue_handle = NULL;

/**
 * Sets up the HTTP server configuration and starts the server.
 * @return the HTTP server instance handle if successful, NULL otherwise.
 */
static httpd_handle_t http_server_configure(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Create HTTP server monitor task
    xTaskCreatePinnedToCore(
        http_server_monitor,
        "http_server_monitor",
        HTTP_SERVER_MONITOR_TASK_STACK_SIZE,
        NULL,
        HTTP_SERVER_MONITOR_TASK_PRIORITY,
        &task_http_server_monitor,
        HTTP_SERVER_MONITOR_TASK_CORE_ID
    );

    // Create the message queue
    http_server_monitor_queue_handle = xQueueCreate(3, sizeof(http_server_queue_message_t));

    // The core that the HTTP server will run on
    config.core_id = HTTP_SERVER_TASK_CORE_ID;
    // Adjust the default priority to 1 less than the wifi application task
    config.task_priority = HTTP_SERVER_TASK_PRIORITY;
    // Bump up the stack size (because the default is 4096)
    config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;
    // Increase uri handlers
    config.max_uri_handlers = 20;
    // Increase the timeout limits
    config.recv_wait_timeout = 10;
    config.send_wait_timeout = 10;

    ESP_LOGI(TAG, "Starting server on port %d, with task priority %d", config.server_port, config.task_priority);

    // Start the HTTP server
    if (httpd_start(&http_server_handle, &config) == ESP_OK)
    {
        ESP_LOGI(TAG, "Registering URI handlers");

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/",
            .method = HTTP_GET,
            .handler = http_server_index_html_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/jquery-3.3.1.min.js",
            .method = HTTP_GET,
            .handler = http_server_jquery_handler,
            .user_ctx = NULL
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/app.css",
            .method = HTTP_GET,
            .handler = http_server_app_css_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/app.js",
            .method = HTTP_GET,
            .handler = http_server_app_js_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/favicon.ico",
            .method = HTTP_GET,
            .handler = http_server_favicon_ico_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/OTAupdate",
            .method = HTTP_POST,
            .handler = http_server_OTA_update_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/OTAstatus",
            .method = HTTP_POST,
            .handler = http_server_OTA_status_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/dhtSensor.json",
            .method = HTTP_GET,
            .handler = http_server_get_dht_sensor_readings_json_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/wifiConnect.json",
            .method = HTTP_POST,
            .handler = http_server_wifi_connect_json_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/wifiConnectStatus",
            .method = HTTP_POST,
            .handler = http_server_wifi_connect_status_json_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/wifiConnectInfo.json",
            .method = HTTP_GET,
            .handler = http_server_get_wifi_connect_info_json_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/wifiDisconnect.json",
            .method = HTTP_DELETE,
            .handler = http_server_wifi_disconnect_json_handler
        });

        httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
            .uri = "/localTime.json",
            .method = HTTP_GET,
            .handler = http_server_get_local_time_json_handler,
        });

        // httpd_register_uri_handler(http_server_handle, &(httpd_uri_t){
        //     .uri = "/apSSID.json",
        //     .method = HTTP_GET,
        //     .handler = http_server_get_ap_ssid_json_handler,
        // });

        return http_server_handle;
    }

    return NULL;
}

void http_server_start(void)
{
    if (http_server_handle == NULL)
    {
        http_server_handle = http_server_configure();
    }
}

void http_server_stop(void)
{
    if (http_server_handle)
    {
        httpd_stop(http_server_handle);
        http_server_handle = NULL;
        ESP_LOGI(TAG, "HTTP server stopped");
    }

    if (task_http_server_monitor)
    {
        vTaskDelete(task_http_server_monitor);
        task_http_server_monitor = NULL;
        ESP_LOGI(TAG, "HTTP server monitor task deleted");
    }
}
