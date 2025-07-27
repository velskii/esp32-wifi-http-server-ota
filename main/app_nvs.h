#ifndef MAIN_APP_NVS_H_
#define MAIN_APP_NVS_H_

#include <esp_err.h>

/**
 * Saves station mode WiFi credentials to NVS.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t app_nvs_save_sta_creds(void);

/**
 * Loads the previously saved station mode WiFi credentials from NVS.
 * @return true if previously saved credentials were found, false otherwise.
 */
bool app_nvs_load_sta_creds(void);

/**
 * Clears the saved station mode WiFi credentials from NVS.
 * @return ESP_OK on success, or an error code on failure.
 */
esp_err_t app_nvs_clear_sta_creds(void);


#endif /* MAIN_APP_NVS_H_ */