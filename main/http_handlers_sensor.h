#ifndef HTTP_HANDLERS_SENSOR_H_
#define HTTP_HANDLERS_SENSOR_H_

#include "esp_http_server.h"

// URI handler for getting DHT sensor readings in plain text format
esp_err_t http_server_get_dht_sensor_readings_json_handler(httpd_req_t *req);


#endif // HTTP_HANDLERS_SENSOR_H_