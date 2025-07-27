#include "esp_log.h"

#include "sntp_time_sync.h"
#include "http_handlers_sntp.h"

static const char TAG[] = "http_handlers_sntp";

// Local time set status
bool g_is_local_time_set = false;

/**
 * localTime.json handler which responds with the local time.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK if successful, otherwise ESP_FAIL if timeout occurs and the update cannot be started.
 */
esp_err_t http_server_get_local_time_json_handler(httpd_req_t *req) {
  ESP_LOGI(TAG, "/localTime.json requested");

  char localTimeJSON[100] = {0};

  if (g_is_local_time_set)
  {
    snprintf(localTimeJSON, sizeof(localTimeJSON), "{\"time\": \"%s\"}", sntp_time_sync_get_time());
  }

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, localTimeJSON, strlen(localTimeJSON));
  
  return ESP_OK;
}