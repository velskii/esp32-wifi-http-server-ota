#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "app_nvs.h"
#include "wifi_app.h"

static const char TAG[] = "app_nvs";

// NVS name space used for storing WiFi credentials
const char app_nvs_sta_creds_namespace[] = "stacreds";

esp_err_t app_nvs_save_sta_creds(void)
{
  nvs_handle handle;
  esp_err_t esp_err;
  ESP_LOGI(TAG, "app_nvs_save_sta_creds: Saving station mode WiFi credentials to NVS");

  wifi_config_t *wifi_sta_config = wifi_app_get_wifi_config();

  if(wifi_sta_config)
  {
    esp_err = nvs_open(app_nvs_sta_creds_namespace, NVS_READWRITE, &handle);
    if (esp_err != ESP_OK) {
      ESP_LOGE(TAG, "app_nvs_save_sta_creds: Failed to open NVS namespace %s, error: %s", app_nvs_sta_creds_namespace, esp_err_to_name(esp_err));
      return esp_err;
    }

    esp_err = nvs_set_blob(handle, "ssid", wifi_sta_config->sta.ssid, MAX_SSID_LENGTH);
    if (esp_err != ESP_OK) {
      ESP_LOGE(TAG, "app_nvs_save_sta_creds: Failed to save ssid to NVS, error: %s", esp_err_to_name(esp_err));
      nvs_close(handle);
      return esp_err;
    }

    esp_err = nvs_set_blob(handle, "password", wifi_sta_config->sta.password, MAX_PASSWORD_LENGTH);
    if (esp_err != ESP_OK) {
      ESP_LOGE(TAG, "app_nvs_save_sta_creds: Failed to save password to NVS, error: %s", esp_err_to_name(esp_err));
      nvs_close(handle);
      return esp_err;
    }

    esp_err = nvs_commit(handle);
    if (esp_err != ESP_OK) {
      ESP_LOGE(TAG, "app_nvs_save_sta_creds: Failed to commit changes to NVS, error: %s", esp_err_to_name(esp_err));
    }

    nvs_close(handle);
    ESP_LOGI(TAG, "app_nvs_save_sta_creds: Successfully saved station mode WiFi credentials: SSID: %s, Password: %s to NVS", 
             wifi_sta_config->sta.ssid, wifi_sta_config->sta.password);
    
    return ESP_OK;
  }
  else
  {
    ESP_LOGW(TAG, "app_nvs_save_sta_creds: No WiFi station configuration found to save");
    return ESP_FAIL;
  }
}

bool app_nvs_load_sta_creds(void)
{
  nvs_handle handle;
  esp_err_t esp_err;
  ESP_LOGI(TAG, "app_nvs_load_sta_creds: Loading station mode WiFi credentials from NVS");

  if(nvs_open(app_nvs_sta_creds_namespace, NVS_READONLY, &handle) == ESP_OK)
  {
    wifi_config_t *wifi_sta_config = wifi_app_get_wifi_config();
    if (wifi_sta_config == NULL) {
       wifi_sta_config = (wifi_config_t *)malloc(sizeof(wifi_config_t));
    }
    memset(wifi_sta_config, 0x00, sizeof(wifi_config_t));

    // Allocate buffers for SSID and password
    size_t wifi_config_size = sizeof(wifi_config_t);
    uint8_t *wifi_config_buff = (uint8_t *)malloc(sizeof(uint8_t) * wifi_config_size);
    memset(wifi_config_buff, 0x00, wifi_config_size);

    // Load SSID
    wifi_config_size = sizeof(wifi_sta_config->sta.ssid);
    esp_err = nvs_get_blob(handle, "ssid", wifi_config_buff, &wifi_config_size);
    if (esp_err != ESP_OK) {
      free(wifi_config_buff);
      ESP_LOGI(TAG, "app_nvs_load_sta_creds: No SSID found in NVS, error: %s", esp_err_to_name(esp_err));
      nvs_close(handle);
      return false;
    }
    memcpy(wifi_sta_config->sta.ssid, wifi_config_buff, wifi_config_size);

    // Load password
    wifi_config_size = sizeof(wifi_sta_config->sta.password);
    esp_err = nvs_get_blob(handle, "password", wifi_config_buff, &wifi_config_size);
    if (esp_err != ESP_OK) {
      free(wifi_config_buff);
      ESP_LOGI(TAG, "app_nvs_load_sta_creds: No password found in NVS, error: %s", esp_err_to_name(esp_err));
      nvs_close(handle);
      return false;
    }
    memcpy(wifi_sta_config->sta.password, wifi_config_buff, wifi_config_size);

    free(wifi_config_buff);
    nvs_close(handle);
    ESP_LOGI(TAG, "app_nvs_load_sta_creds: Successfully loaded station mode WiFi credentials: SSID: %s, Password: %s from NVS", 
             wifi_sta_config->sta.ssid, wifi_sta_config->sta.password);

    return wifi_sta_config->sta.ssid[0] != '\0' && wifi_sta_config->sta.password[0] != '\0';
  }
  else
  {
    ESP_LOGW(TAG, "app_nvs_load_sta_creds: No WiFi station configuration found to load");
    return false;
  }
}

esp_err_t app_nvs_clear_sta_creds(void)
{
  nvs_handle handle;
  esp_err_t esp_err;
  ESP_LOGI(TAG, "app_nvs_clear_sta_creds: Clearing station mode WiFi credentials from NVS");

  esp_err = nvs_open(app_nvs_sta_creds_namespace, NVS_READWRITE, &handle);
  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "app_nvs_clear_sta_creds: Failed to open NVS namespace %s, error: %s", app_nvs_sta_creds_namespace, esp_err_to_name(esp_err));
    return esp_err;
  }

  esp_err = nvs_erase_all(handle);
  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "app_nvs_clear_sta_creds: Failed to erase NVS namespace %s, error: %s", app_nvs_sta_creds_namespace, esp_err_to_name(esp_err));
    nvs_close(handle);
    return esp_err;
  }

  esp_err = nvs_commit(handle);
  if (esp_err != ESP_OK) {
    ESP_LOGE(TAG, "app_nvs_clear_sta_creds: Failed to commit changes to NVS, error: %s", esp_err_to_name(esp_err));
  }

  nvs_close(handle);
  ESP_LOGI(TAG, "app_nvs_clear_sta_creds: Successfully cleared station mode WiFi credentials from NVS");
  
  return ESP_OK;
}