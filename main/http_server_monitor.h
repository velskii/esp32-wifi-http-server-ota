#ifndef HTTP_SERVER_MONITOR_H_
#define HTTP_SERVER_MONITOR_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Messages passed from the HTTP server to the monitor task
typedef enum {
  HTTP_MSG_WIFI_CONNECT_INIT = 0,
	HTTP_MSG_WIFI_CONNECT_SUCCESS,
	HTTP_MSG_WIFI_CONNECT_FAIL,
	HTTP_MSG_OTA_UPDATE_SUCCESSFUL,
	HTTP_MSG_OTA_UPDATE_FAILED,
} http_server_message_e;

// Struct for a message
typedef struct {
    http_server_message_e msgID; 
} http_server_queue_message_t;

// Starts the HTTP server monitor task
void http_server_monitor_start(void);

// Sends a message to the HTTP server monitor task
BaseType_t http_server_monitor_send_message(http_server_message_e msgID);

#endif // HTTP_SERVER_MONITOR_H_
