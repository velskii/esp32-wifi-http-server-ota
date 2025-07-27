#include "esp_log.h"

#include "wifi_app.h"
#include "http_handlers_ap_ssid.h"

static const char TAG[] = "http_handlers_ap_ssid";

esp_err_t http_server_get_ap_ssid_json_handler(httpd_req_t *req)
{
  ESP_LOGI(TAG, "/apSSID.json requested");

  char ssidJSON[50];
  wifi_config_t *wifi_config = wifi_app_get_wifi_config();
  char *ssid = (char *)wifi_config->ap.ssid;

  sprintf(ssidJSON, "{\"ssid\": \"%s\"}", ssid);

  httpd_resp_set_type(req, "application/json");
  httpd_resp_send(req, ssidJSON, strlen(ssidJSON));

  return ESP_OK;
}