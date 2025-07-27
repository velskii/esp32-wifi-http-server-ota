/**
 * @file wifi_app.c
 * @brief Implementation of the Wi-Fi application management.
 * Created on: July 2, 2025
 * Author: Greg
 */

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "inttypes.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/netdb.h"

#include "app_nvs.h"
#include "http_server.h"
#include "http_server_monitor.h"
#include "rgb_led.h"
#include "tasks_common.h"
#include "wifi_app.h"

// Tag used for ESP serial console logging messages
static const char TAG[] = "wifi_app";

// Used for returning the WiFi configuration
wifi_config_t *wifi_config = NULL;

// Used to track the number for retries when a connection attempt fails
static int g_retry_number = 0;

// Wifi application event group handle and status bits
static EventGroupHandle_t wifi_app_event_group;
const int WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT = BIT0; // Bit for indicating that the app is connecting using saved credentials
const int WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT = BIT1; // Bit for indicating that the app is connecting from the HTTP server
const int WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT = BIT2; // Bit for indicating that the user requested a disconnect
const int WIFI_APP_STA_CONNECTED_GOT_IP_BIT = BIT3; // Bit for indicating that the station has connected and got an IP address

//Queue handle used to manipulate the main queue of events
static QueueHandle_t wifi_app_queue_handle;

// netif objects for the Station and Access Point
esp_netif_t *esp_netif_sta = NULL;
esp_netif_t *esp_netif_ap = NULL;

/**
 * @brief Event handler for Wi-Fi and IP events.
 * This function handles various Wi-Fi events such as connection, disconnection, and IP address acquisition.
 * @param arg data, aside from event data, that can be passed to the handler (not used in this implementation).
 * @param event_base The base event type (WIFI_EVENT or IP_EVENT).
 * @param event_id The specific event ID.
 * @param event_data Pointer to the event data (not used in this implementation).
 */
static void wifi_app_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  if (event_base == WIFI_EVENT)
  {
    switch (event_id)
    {
      case WIFI_EVENT_AP_START:
        ESP_LOGI(TAG, "WIFI_EVENT_AP_START");
        break;
      case WIFI_EVENT_AP_STOP:
        ESP_LOGI(TAG, "WIFI_EVENT_AP_STOP");
        break;
      case WIFI_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_AP_STACONNECTED");
        break;
      case WIFI_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_AP_STADISCONNECTED");
        break;

      case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
        // Start the Wi-Fi station
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
      case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
        break;
      case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");

        wifi_event_sta_disconnected_t *wifi_event_sta_disconnected = (wifi_event_sta_disconnected_t *)malloc(sizeof(wifi_event_sta_disconnected_t));
        *wifi_event_sta_disconnected = *((wifi_event_sta_disconnected_t *)event_data);
        printf("WIFI_EVENT_STA_DISCONNECTED, Reason code: %d\n", wifi_event_sta_disconnected->reason);

        if (g_retry_number < MAX_CONNECTION_RETRIES)
        {
          g_retry_number++;
          ESP_LOGI(TAG, "Retrying to connect to the AP, attempt %d", g_retry_number);
          // Retry connecting to the AP
          esp_wifi_connect();
        }
        else
        {
          ESP_LOGE(TAG, "Failed to connect to the AP after %d attempts", g_retry_number);
          wifi_app_send_message(WIFI_APP_MSG_STA_DISCONNECTED);
        }
        break;
      
      default:
        break;
    }
  }
  else if (event_base == IP_EVENT)
  {
    switch (event_id)
    {
      case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
        // Send a message to the queue indicating that the station has connected and got an IP address
        wifi_app_send_message(WIFI_APP_MSG_STA_CONNECTED_GOT_IP);
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Initializes the TCP/IP stack and Wi-Fi configuration.
 */
static void wifi_app_default_wifi_init(void)
{
  // Initialize the TCP/IP stack
  ESP_ERROR_CHECK(esp_netif_init());

  // Initialize the Wi-Fi configuration, operations must be in this order.
  wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

  // Initialize the station netif
  esp_netif_sta = esp_netif_create_default_wifi_sta();

  // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  
  // wifi_config_t sta_config = {
  //   .sta = {
  //       .ssid = WIFI_STA_SSID,
  //       .password = WIFI_STA_PASSWORD,
  //   },
  // };
  // ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config));

  // Initialize the SoftAP
  esp_netif_ap = esp_netif_create_default_wifi_ap();
}

/**
 * @brief Configures the WiFi access point settings and assigns the static IP address to the SoftAP.
 */
static void wifi_app_soft_ap_config(void)
{
  // Configure the SoftAP settings
  wifi_config_t wifi_config = {
      .ap = {
          .ssid = WIFI_AP_SSID,
          .ssid_len = strlen(WIFI_AP_SSID),
          .password = WIFI_AP_PASSWORD,
          .channel = WIFI_AP_CHANNEL,
          .ssid_hidden = WIFI_AP_SSID_HIDDEN,
          .max_connection = WIFI_AP_MAX_CONNECTIONS,
          .beacon_interval = WIFI_AP_BEACON_INTERVAL,
          .authmode = WIFI_AUTH_WPA_WPA2_PSK, 
      },
  };

  // Set the DHCP server options for the SoftAP configuration
  esp_netif_ip_info_t ap_ip_info;
  memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));

  // Must call this first to set the IP address, gateway, and netmask
  esp_netif_dhcps_stop(esp_netif_ap);
  inet_pton(AF_INET, WIFI_AP_IP_ADDRESS, &ap_ip_info.ip);
  inet_pton(AF_INET, WIFI_AP_GATEWAY, &ap_ip_info.gw);
  inet_pton(AF_INET, WIFI_AP_NETMASK, &ap_ip_info.netmask);
  
  // Statically configure the network interface for the SoftAP
  ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));
  // Start the AP DHCP server (for connecting stations, e.g., your mobile phone)
  ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));

  // Set the mode as Access Point and Station
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_set_bandwidth(ESP_IF_WIFI_AP, WIFI_AP_BANDWIDTH));
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_STA_POWER_SAVE));
}


/**
 * @brief Initializes the event handler for the Wi-Fi application and IP events.
 */
static void wifi_app_event_handler_init(void)
{
  // Event loop for the Wifi driver
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // Event handler for the connection events
  esp_event_handler_instance_t instance_wifi_event;
  esp_event_handler_instance_t instance_ip_event;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_wifi_event));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_ip_event));
}

/**
 * Connects to the ESP32 to an external AP using the updated station configuration.
 */
static void wifi_app_connect_sta(void)
{
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_app_get_wifi_config()));
  ESP_ERROR_CHECK(esp_wifi_connect());
}

/**
 * Main task function for the Wi-Fi application.
 * This function handles incoming messages from the queue and performs actions based on the message ID.
 * @param pvParameters Pointer to task parameters (not used).
 */
static void wifi_app_task(void *pvParameters)
{
    wifi_app_queue_message_t msg;
    EventBits_t eventBits;

    // Initialize the event handler
    wifi_app_event_handler_init();

    // Initialize the TCP/IP stack and wifi configuration
    wifi_app_default_wifi_init();

    // SoftAP configuration
    wifi_app_soft_ap_config();

    // Start WIFI
    ESP_ERROR_CHECK(esp_wifi_start());

    // Send first event message to the queue
    wifi_app_send_message(WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS);

    // Loop forever
    while (1)
    {
        // Wait for a message on the queue
        xQueueReceive(wifi_app_queue_handle, &msg, portMAX_DELAY);

        // Handle the message based on its ID
        switch (msg.msgID)
        {
          case WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS:
            ESP_LOGI(TAG, "WIFI_APP_MSG_LOAD_SAVED_CREDENTIALS");
            // Load saved Wi-Fi credentials from NVS
            if (app_nvs_load_sta_creds())
            {
              ESP_LOGI(TAG, "Saved credentials found");
              // Set the bit to indicate that we are connecting using saved credentials
              xEventGroupSetBits(wifi_app_event_group, WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT);
              // Connect to the Wi-Fi network
              wifi_app_connect_sta();
            }
            else
            {
              ESP_LOGW(TAG, "No saved Wi-Fi credentials found, starting HTTP server");
            }

            wifi_app_send_message(WIFI_APP_MSG_START_HTTP_SERVER);
            break;
          case WIFI_APP_MSG_START_HTTP_SERVER:
            // Start the HTTP server
            ESP_LOGI(TAG, "WIFI_APP_MSG_START_HTTP_SERVER");
            http_server_start();
            rgb_led_http_server_started();

            break;
          case WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER:
            ESP_LOGI(TAG, "WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER");

            xEventGroupSetBits(wifi_app_event_group, WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT);

            // Attempt a connection to the Wi-Fi network
            wifi_app_connect_sta();
            // Set current number of retries to zero
            g_retry_number = 0;
            // Let the HTTP server know about the connection attempt
            http_server_monitor_send_message(HTTP_MSG_WIFI_CONNECT_INIT);

            break;
          case WIFI_APP_MSG_STA_CONNECTED_GOT_IP:
            ESP_LOGI(TAG, "WIFI_APP_MSG_STA_CONNECTED_GOT_IP");

            xEventGroupSetBits(wifi_app_event_group, WIFI_APP_STA_CONNECTED_GOT_IP_BIT);

            // Indicate that the station has connected and got an IP address
            rgb_led_wifi_connected();
            http_server_monitor_send_message(HTTP_MSG_WIFI_CONNECT_SUCCESS);

            eventBits = xEventGroupGetBits(wifi_app_event_group);
            // Save STA creds only if connecting from the http server (not loaded from NVS)
            if (eventBits & WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT)
            {
              // Clear the bit, in case we want to disconnect and reconnect, then start the process again
              xEventGroupClearBits(wifi_app_event_group, WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT);
            } else {
              app_nvs_save_sta_creds();
            }

            if (eventBits & WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT)
            {
              xEventGroupClearBits(wifi_app_event_group, WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT);
            }

            break;
          case WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT:
            ESP_LOGI(TAG, "WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT");

            eventBits = xEventGroupGetBits(wifi_app_event_group);
            if (eventBits & WIFI_APP_STA_CONNECTED_GOT_IP_BIT)
            {
              xEventGroupSetBits(wifi_app_event_group, WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT);
  
              g_retry_number = MAX_CONNECTION_RETRIES; // Set the retry number to max to force disconnect
              ESP_ERROR_CHECK(esp_wifi_disconnect());
  
              // Clear the saved Wi-Fi credentials from NVS
              app_nvs_clear_sta_creds();
              rgb_led_http_server_started();
            }
            
            break;
          case WIFI_APP_MSG_STA_DISCONNECTED:
            ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED");

            eventBits = xEventGroupGetBits(wifi_app_event_group);
            if (eventBits & WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT)
            {
              ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: Attempt using saved credentials");
              // Clear the bit, in case we want to disconnect and reconnect, then start the process again
              xEventGroupClearBits(wifi_app_event_group, WIFI_APP_CONNECTING_USING_SAVED_CREDS_BIT);
              app_nvs_clear_sta_creds();
            } 
            else if (eventBits & WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT)
            {
              ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: Attempt using HTTP server credentials");
              xEventGroupClearBits(wifi_app_event_group, WIFI_APP_CONNECTING_FROM_HTTP_SERVER_BIT);
              http_server_monitor_send_message(HTTP_MSG_WIFI_CONNECT_FAIL);
            }
            else if (eventBits & WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT)
            {
              ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: User requested disconnect");
              xEventGroupClearBits(wifi_app_event_group, WIFI_APP_USER_REQUESTED_STA_DISCONNECT_BIT);
              http_server_monitor_send_message(HTTP_MSG_WIFI_USER_DISCONNECT);
            }
            else
            {
              ESP_LOGI(TAG, "WIFI_APP_MSG_STA_DISCONNECTED: Unknown disconnect reason");
            }

            if (eventBits & WIFI_APP_STA_CONNECTED_GOT_IP_BIT)
            {
              xEventGroupClearBits(wifi_app_event_group, WIFI_APP_STA_CONNECTED_GOT_IP_BIT);
            }

            break;
          default:
            break;
        }
    }
}

BaseType_t wifi_app_send_message(wifi_app_message_e msgID)
{
    wifi_app_queue_message_t msg;
    msg.msgID = msgID;

    return xQueueSend(wifi_app_queue_handle, &msg, portMAX_DELAY);
}

wifi_config_t *wifi_app_get_wifi_config(void)
{
  return wifi_config;
}

void wifi_app_start(void)
{
  ESP_LOGI(TAG, "Starting Wi-Fi application...");

  // Start WIFI started LED
  rgb_led_wifi_app_started();

  // Disable default WIFI logging messsages
  esp_log_level_set("wifi", ESP_LOG_NONE);

  // Allocate memory for the Wi-Fi configuration
  wifi_config = (wifi_config_t *)malloc(sizeof(wifi_config_t));
  memset(wifi_config, 0x00, sizeof(wifi_config_t));

  // Create the queue to handle messages
  wifi_app_queue_handle = xQueueCreate(3, sizeof(wifi_app_queue_message_t));

  // Create Wifi application event group
  wifi_app_event_group = xEventGroupCreate();

  // Start the WIFI RTOS task
  xTaskCreatePinnedToCore(
      &wifi_app_task,                // Task function
      "wifi_app_task",              // Name of the task
      WIFI_APP_TASK_STACK_SIZE,     // Stack size in bytes
      NULL,                         // Task input parameter
      WIFI_APP_TASK_PRIORITY,       // Task priority
      NULL,                         // Task handle
      WIFI_APP_TASK_CORE_ID         // Core ID to run the task on
  );
}