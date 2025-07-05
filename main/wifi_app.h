/**
 * wifi_app.h
 * @brief Header file for Wi-Fi application management.
 * Created on: July 2, 2025
 * Author: Greg
 */
 

 #ifndef MAIN_WIFI_APP_H_
 #define MAIN_WIFI_APP_H_

 #include "freertos/FreeRTOS.h"
 #include "freertos/queue.h"
 #include "esp_wifi_types.h"
 #include "esp_netif.h"

 // WIFI application settings
 #define WIFI_AP_SSID                   "ESP32_AP"
 #define WIFI_AP_PASSWORD               "12345678"
 #define WIFI_AP_CHANNEL                1
 #define WIFI_AP_SSID_HIDDEN            false
 #define WIFI_AP_MAX_CONNECTIONS        5
 #define WIFI_AP_BEACON_INTERVAL        100
 #define WIFI_AP_IP_ADDRESS             "192.168.0.1"
 #define WIFI_AP_GATEWAY                "192.168.0.1"
 #define WIFI_AP_NETMASK                "255.255.255.0"
 #define WIFI_AP_BANDWIDTH              WIFI_BW_HT20
 #define WIFI_STA_POWER_SAVE            WIFI_PS_NONE
 #define MAX_SSID_LENGTH                32
 #define MAX_PASSWORD_LENGTH            64
 #define MAX_CONNECTION_RETRIES         5
 #define WIFI_STA_SSID                  "ESP32_STA"
 #define WIFI_STA_PASSWORD              "12345678"

 // netif object for the Station and Access Point
 extern esp_netif_t *esp_netif_sta;
 extern esp_netif_t *esp_netif_ap;


 /**
  * Message IDs for the WIFI application task.
  * @note Expand this based on your application's needs.
  */
typedef enum wifi_app_message
{
    WIFI_APP_MSG_START_HTTP_SERVER = 0, 
    WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER,
    WIFI_APP_MSG_STA_CONNECTED_GOT_IP, 
    WIFI_APP_MSG_STA_DISCONNECTED,
} wifi_app_message_e;

/**
 * Structure for the message queue used in the Wi-Fi application.
 * @note This structure can be expanded with additional fields as needed.
 */
typedef struct wifi_app_queue_message
{
  wifi_app_message_e msgID;
} wifi_app_queue_message_t;

/**
 * Sends a message to the queue
 * @param msgID The message ID from the wifi_app_message_e enum.
 * @return pdTRUE if the message was sent successfully, otherwise pdFALSE.
 * @note Expand the parameter list based on your requirements e.g., how you've expanded the wifi_app_queue_message_t structure.
 */
 BaseType_t wifi_app_send_message(wifi_app_message_e msgID);

 /**
  * Starts the WIFI RTOS task.
  */
 void wifi_app_start(void);

 /**
  * @brief Gets the Wi-Fi configuration.
  */
 wifi_config_t *wifi_app_get_wifi_config(void);

 #endif /* MAIN_WIFI_APP_H_ */