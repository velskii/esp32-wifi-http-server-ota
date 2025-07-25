#ifndef HTTP_SERVER_MONITOR_H_
#define HTTP_SERVER_MONITOR_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Messages passed from the HTTP server to the monitor task
typedef enum htpp_server_message {
  HTTP_MSG_WIFI_CONNECT_INIT = 0,
	HTTP_MSG_WIFI_CONNECT_SUCCESS,
	HTTP_MSG_WIFI_CONNECT_FAIL,
	HTTP_MSG_WIFI_USER_DISCONNECT,
	HTTP_MSG_OTA_UPDATE_SUCCESSFUL,
	HTTP_MSG_OTA_UPDATE_FAILED,
	HTTP_MSG_TIME_SERVICE_INITIALIZED,
} http_server_message_e;

// Struct for a message queue item
typedef struct http_server_queue_message {
    http_server_message_e msgID; 
} http_server_queue_message_t;

// Starts the HTTP server monitor task
void http_server_monitor(void *pvParameters);


/**
 * Sends a message to the HTTP server monitor task
 * @param msgID message ID from the http_server_message_e enum.
 * @return pdTRUE if an item was successfully sent to the queue, otherwise pdFALSE.
 * @note Expand the parameter list based on your requirements e.g. how you've expanded the http_server_queue_message_t.
 */
BaseType_t http_server_monitor_send_message(http_server_message_e msgID);

#endif // HTTP_SERVER_MONITOR_H_
