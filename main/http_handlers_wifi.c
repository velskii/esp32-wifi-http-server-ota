#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_netif.h"

#include "wifi_app.h"
#include "http_handlers_wifi.h"
#include "http_server_monitor.h"

static const char TAG[] = "http_handlers_wifi";

int g_wifi_connect_status = NONE;


/**
 * wifiConnect.json handler which handles the request for the Wi-Fi connection credentials. It is invoked after the connect button is pressed and handles the SSID and password from the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK if successful, otherwise ESP_FAIL if timeout occurs and the update cannot be started.
 */
esp_err_t http_server_wifi_connect_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/wifiConnect.json requested");
	size_t len_ssid = 0, len_pass = 0;
	char *ssid_str = NULL, *pass_str = NULL;

	// Get SSID header
	len_ssid = httpd_req_get_hdr_value_len(req, "my-connect-ssid") + 1;
	if (len_ssid > 1)
	{
		ssid_str = malloc(len_ssid);
		if (httpd_req_get_hdr_value_str(req, "my-connect-ssid", ssid_str, len_ssid) == ESP_OK)
		{
			ESP_LOGI(TAG, "http_server_wifi_connect_json_handler: Found header => my-connect-ssid: %s", ssid_str);
		}
	}

	// Get password header
	len_pass = httpd_req_get_hdr_value_len(req, "my-connect-pwd") + 1;
	if (len_pass > 1)
	{
		pass_str = malloc(len_pass);
		if (httpd_req_get_hdr_value_str(req, "my-connect-pwd", pass_str, len_pass) == ESP_OK)
		{
			ESP_LOGI(TAG, "http_server_wifi_connect_json_handler: Found header => my-connect-pwd: %s", pass_str);
		}
	}
	
	// Update the Wi-Fi configuration with the SSID and password and let the wifi application know about the new credentials
	wifi_config_t *wifi_config = wifi_app_get_wifi_config();
	memset(wifi_config, 0x00, sizeof(wifi_config_t));
	memcpy(wifi_config->sta.ssid, ssid_str, len_ssid);
	memcpy(wifi_config->sta.password, pass_str, len_pass);
	wifi_app_send_message(WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER);

	free(ssid_str);
	free(pass_str);

	return ESP_OK;
}
/**
 * WifiConnectStatus handler which updates the connection status for the web page.
 */
esp_err_t http_server_wifi_connect_status_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/wifiConnectStatus requested");
	char statusJSON[100];
	sprintf(statusJSON, "{\"wifi_connect_status\": %d}", g_wifi_connect_status);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, statusJSON, strlen(statusJSON));
	return ESP_OK;
}

/**
 * wifiConnectInfo.json handler which updates the web page with connection information.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK if successful, otherwise ESP_FAIL if timeout occurs and the update cannot be started.
 */
esp_err_t http_server_get_wifi_connect_info_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/wifiConnectInfo.json requested");

	char ipInfoJSON[200];
	memset(ipInfoJSON, 0, sizeof(ipInfoJSON));

	char ip[IP4ADDR_STRLEN_MAX];
	char netmask[IP4ADDR_STRLEN_MAX];
	char gw[IP4ADDR_STRLEN_MAX];

	if (g_wifi_connect_status == HTTP_WIFI_STATUS_CONNECT_SUCCESS)
	{
		wifi_ap_record_t wifi_data;
		ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&wifi_data));
		char *ssid = (char *)wifi_data.ssid;

		esp_netif_ip_info_t ip_info;
		ESP_ERROR_CHECK(esp_netif_get_ip_info(esp_netif_sta, &ip_info));
		esp_ip4addr_ntoa(&ip_info.ip, ip, IP4ADDR_STRLEN_MAX);
		esp_ip4addr_ntoa(&ip_info.netmask, netmask, IP4ADDR_STRLEN_MAX);
		esp_ip4addr_ntoa(&ip_info.gw, gw, IP4ADDR_STRLEN_MAX);

		sprintf(ipInfoJSON, "{\"wifi_connect_status\": %d, \"wifi_ssid\": \"%s\", \"wifi_ip\": \"%s\", \"wifi_netmask\": \"%s\", \"wifi_gateway\": \"%s\"}", g_wifi_connect_status, ssid, ip, netmask, gw);
	}
	
	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, ipInfoJSON, strlen(ipInfoJSON));

	return ESP_OK;
}

/**
 * wifiDisconnect.json handler which responds by sending to the wifi application to disconnect.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK if successful, otherwise ESP_FAIL if timeout occurs and the update cannot be started.
 */
esp_err_t http_server_wifi_disconnect_json_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "/wifiDisconnect.json requested");
	wifi_app_send_message(WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT);
	return ESP_OK;
}