#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/apps/sntp.h"

#include "tasks_common.h"
#include "http_server_monitor.h"
#include "sntp_time_sync.h"
#include "wifi_app.h"

static const char TAG[] = "sntp_time_sync";

// SNTP operating mode set sstatus
static bool sntp_op_mode_set = false;

// Initialize SNTP service using SNTP_OPMODE_POLL
static void sntp_time_sync_init_sntp(void)
{
  ESP_LOGI(TAG, "SNTP operating mode set to SNTP_OPMODE_POLL");
  if (!sntp_op_mode_set) {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_op_mode_set = true;
  }
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();

  // Let the http server know that SNTP is initialized
  http_server_monitor_send_message(HTTP_MSG_TIME_SERVICE_INITIALIZED);
}

/**
 * Gets the current time if the current time is not up to date.
 */
static void sntp_time_sync_obtain_time(void)
{
  time_t now;
  struct tm time_info = {0};
  time(&now);
  localtime_r(&now, &time_info);

  // Check the time, in case we need to initialize/reinitialize SNTP
  if (time_info.tm_year < (2016 - 1900)) {
    ESP_LOGI(TAG, "Time not set yet. Initializing SNTP...");
    
    sntp_time_sync_init_sntp();
    setenv("TZ", "GMT4", 1); // Set timezone to GMT+4
    tzset(); // Apply the timezone setting

    sntp_setservername(0, "pool.ntp.org");
  }
}

/**
 * The SNTP time synchronization task
 * @param pvParam Pointer to task parameters (not used).
 */
static void sntp_time_sync(void *pvParam)
{
  while (1) {
    sntp_time_sync_obtain_time();
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}

char *sntp_time_sync_get_time(void)
{
  static char time_buffer[100] = {0};
  time_t now = 0;
  struct tm time_info = {0};

  time(&now);
  localtime_r(&now, &time_info);

  if(time_info.tm_year < (2016 - 1900) ) {
    ESP_LOGI(TAG, "Time not set yet. Initializing SNTP...");
  } else {
    // Format the time into a string
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &time_info);
    ESP_LOGI(TAG, "Current time: %s", time_buffer);
  }
  return time_buffer;
}

void sntp_time_sync_task_start(void)
{
  xTaskCreatePinnedToCore(&sntp_time_sync, "sntp_time_sync", SNTP_TIME_SYNC_TASK_STACK_SIZE, NULL, SNTP_TIME_SYNC_TASK_PRIORITY, NULL, SNTP_TIME_SYNC_TASK_CORE_ID);
}