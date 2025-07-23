#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_ota_ops.h"
#include "esp_timer.h"
#include "esp_app_format.h"

#include "http_handlers_ota.h"
#include "http_server_monitor.h"

static const char TAG[] = "http_server_OTA";

int g_fw_update_status = OTA_UPDATE_PENDING;

// ESP32 timer configuration passed to esp_timer_create() to reset the ESP32 after a successful OTA update
const esp_timer_create_args_t fw_update_reset_args = {
		.callback = &http_server_fw_update_reset_callback,
		.arg = NULL,
		.dispatch_method = ESP_TIMER_TASK,
		.name = "fw_update_reset",
	};
esp_timer_handle_t fw_update_reset;


/**
 * Handles the OTA update request. Receives the .bin file via the web page and handles the firmware update.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK if successful, otherwise ESP_FAIL if timeout occurs and the update cannot be started.
 */
esp_err_t http_server_OTA_update_handler(httpd_req_t *req)
{
	esp_ota_handle_t ota_handle;

	char ota_buff[1024];
	int content_length = req->content_len;
	int content_received = 0;
	int recv_len;
	bool is_req_body_started = false;
	bool flash_successful = false;

	const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

	do 
	{
		// Read the data for the request
		if ((recv_len = httpd_req_recv(req, ota_buff, MIN(content_length, sizeof(ota_buff)))) <= 0)
		{
			if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
			{
				ESP_LOGE(TAG, "http_server_OTA_update_handler: Timeout while receiving data");
				continue; // Retry receiving if timeout occurred
			}
			ESP_LOGE(TAG, "http_server_OTA_update_handler: OTA other Error %d", recv_len);
			return ESP_FAIL;
		}
		printf("http_server_OTA_update_handler: OTA RX %d of %d bytes\n",content_received, recv_len);

		// Check if this is the first data we are receiving, If so, it will have the information in the handler that we need to start the OTA update
		if (!is_req_body_started)
		{
			is_req_body_started = true;

			// Get the location of the .bin file content (remove the web form data and headers)
			char *body_start_p = strstr(ota_buff, "\r\n\r\n") + 4;
			int body_part_len = recv_len - (body_start_p - ota_buff);

			printf("http_server_OTA_update_handler: OTA file size %d\r\n", content_length);

			esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
			if (err != ESP_OK)
			{
				ESP_LOGE(TAG, "http_server_OTA_update_handler: esp_ota_begin failed %d", err);
				printf("http_server_OTA_update_handler: Error with OTA begin, cancelling OTA\r\n");
				return ESP_FAIL;
			} 
			else 
			{
				printf("http_server_OTA_update_handler: Writing to partition subtype %d at offset 0x%" PRIx32 "\r\n",
       update_partition->subtype, update_partition->address);
			}

			// Write the first part of the data to the OTA partition
			esp_ota_write(ota_handle, body_start_p, body_part_len);
			content_received += body_part_len;
		}
		else 
		{
			// Write OTA data to the OTA partition
			esp_ota_write(ota_handle, ota_buff, recv_len);
			content_received += recv_len;
		}
	} while (recv_len > 0 && content_received < content_length);

	if (esp_ota_end(ota_handle) == ESP_OK)
	{
		// Lets update the partition with the new firmware
		if (esp_ota_set_boot_partition(update_partition) == ESP_OK)
		{
			const esp_partition_t *boot_partition = esp_ota_get_boot_partition();
			ESP_LOGI(TAG, "http_server_OTA_update_handler: Next booting from partition subtype %d at offset 0x%" PRIx32,
					boot_partition->subtype, boot_partition->address);
			flash_successful = true;
		}
		else 
		{
			ESP_LOGE(TAG, "http_server_OTA_update_handler: FLASH ERROR");
		}
	}
	else 
	{
		ESP_LOGE(TAG, "http_server_OTA_update_handler: esp_ota_end failed");
	}

	// We won't update the global variables throughout the file, so send the message about the status of the OTA update
	if (flash_successful)
	{
		http_server_monitor_send_message(HTTP_MSG_OTA_UPDATE_SUCCESSFUL);
	}
	else 
	{
		http_server_monitor_send_message(HTTP_MSG_OTA_UPDATE_FAILED);
	}
	return ESP_OK;
}

/**
 * OTA status handler, which responds with the firmware update status after the OTA update has started, and responds with the compile time/date when the page is first requested.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK if successful, otherwise ESP_FAIL if timeout occurs and the update cannot be started.
 */
esp_err_t http_server_OTA_status_handler(httpd_req_t *req)
{
	char otaJSON[100];
	ESP_LOGI(TAG, "http_server_OTA_status_handler: requested OTA status\n");
	snprintf(otaJSON, sizeof(otaJSON), "{\"ota_update_status\": %d, \"compile_time\": \"%s\", \"compile_date\": \"%s\"}", g_fw_update_status, __TIME__, __DATE__);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, otaJSON, strlen(otaJSON));

	return ESP_OK;
}


// Checks the g_fw_update_status and creates the fw_update_reset timer if g_fw_update_status is OTA_UPDATE_SUCCESSFUL
static void http_server_fw_update_reset_timer(void)
{
	if (g_fw_update_status == OTA_UPDATE_SUCCESSFUL)
	{
		ESP_LOGI(TAG, "http_server_fw_update_reset_timer: Creating fw_update_reset timer");
		ESP_ERROR_CHECK(esp_timer_create(&fw_update_reset_args, &fw_update_reset));
		ESP_ERROR_CHECK(esp_timer_start_once(fw_update_reset, 8000000));
	}
	else
	{
		ESP_LOGI(TAG, "http_server_fw_update_reset_timer: g_fw_update_status is unsuccessful");
	}
}


void http_server_fw_update_reset_callback(void *arg)
{
	ESP_LOGI(TAG, "http_server_fw_update_reset_callback: Timer timed out, Resetting ESP32 after successful OTA update");
	esp_restart();
}