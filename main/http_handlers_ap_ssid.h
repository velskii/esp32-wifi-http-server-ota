#include "esp_err.h"
#include "esp_http_server.h"



// URI handlers
/**
 * apSSID.json handler responds by sending the AP SSID.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
esp_err_t http_server_get_ap_ssid_json_handler(httpd_req_t *req);