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

/**
 * Sends a message to the queue
 * @param msgID message ID from the http_server_message_e enum.
 * @return pdTRUE if an item was successfully sent to the queue, otherwise pdFALSE.
 * @note Expand the parameter list based on your requirements e.g. how you've expanded the http_server_queue_message_t.
 */
BaseType_t http_server_monitor_send_message(http_server_message_e msgID);

#endif /* MAIN_HTTP_SERVER_H_ */