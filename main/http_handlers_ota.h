#ifndef HTTP_HANDLERS_OTA_H_
#define HTTP_HANDLERS_OTA_H_

#include "esp_err.h"
#include "esp_http_server.h"

// OTA status codes
#define OTA_UPDATE_PENDING      0
#define OTA_UPDATE_SUCCESSFUL   1
#define OTA_UPDATE_FAILED       -1

// Expose these for external use
extern int g_fw_update_status;

// Handler for OTA update
esp_err_t http_server_OTA_update_handler(httpd_req_t *req);

// Handler for OTA status
esp_err_t http_server_OTA_status_handler(httpd_req_t *req);

// Called from monitor when OTA update is successful
void http_server_fw_update_reset_timer(void);

// Timer callback to reset ESP32
void http_server_fw_update_reset_callback(void *arg);


#endif // HTTP_HANDLERS_OTA_H_