#ifndef HTTP_HANDLERS_WIFI_H_
#define HTTP_HANDLERS_WIFI_H_

#include "esp_http_server.h"

// Connection status for the Wi-Fi connection 
typedef enum http_server_wifi_connect_status
{
	NONE = 0,
	HTTP_WIFI_STATUS_CONNECTING,
	HTTP_WIFI_STATUS_CONNECT_FAILED,
	HTTP_WIFI_STATUS_CONNECT_SUCCESS,
	HTTP_WIFI_STATUS_DISCONNECTED,
} http_server_wifi_connect_status_e;

// Expose global status
extern int g_wifi_connect_status;

// URI handlers
esp_err_t http_server_wifi_connect_json_handler(httpd_req_t *req);
esp_err_t http_server_wifi_connect_status_json_handler(httpd_req_t *req);
esp_err_t http_server_get_wifi_connect_info_json_handler(httpd_req_t *req);
esp_err_t http_server_wifi_disconnect_json_handler(httpd_req_t *req);


#endif // HTTP_HANDLERS_WIFI_H_