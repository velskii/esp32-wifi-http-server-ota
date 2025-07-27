#include "esp_err.h"
#include "esp_http_server.h"


// Expose global status
extern bool g_is_local_time_set;

// URI handlers
esp_err_t http_server_get_local_time_json_handler(httpd_req_t *req);