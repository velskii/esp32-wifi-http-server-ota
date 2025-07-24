/** @file http_server.h
 *  Created on: July 2, 2025
 *  Author: Greg
 */

#ifndef MAIN_HTTP_SERVER_H_
#define MAIN_HTTP_SERVER_H_

#include "esp_err.h"

// Starts the HTTP server.
void http_server_start(void);

// Stops the HTTP server.
void http_server_stop(void);

#endif /* MAIN_HTTP_SERVER_H_ */