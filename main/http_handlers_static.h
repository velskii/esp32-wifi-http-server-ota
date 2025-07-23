#ifndef HTTP_HANDLERS_STATIC_H_
#define HTTP_HANDLERS_STATIC_H_

#include "esp_http_server.h"

// URI handlers for embedded static files
esp_err_t http_server_jquery_handler(httpd_req_t *req);
esp_err_t http_server_index_html_handler(httpd_req_t *req);
esp_err_t http_server_app_css_handler(httpd_req_t *req);
esp_err_t http_server_app_js_handler(httpd_req_t *req);
esp_err_t http_server_favicon_ico_handler(httpd_req_t *req);


#endif /* HTTP_HANDLERS_STATIC_H_ */