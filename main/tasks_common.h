/**
 * @file tasks_common.h
 * @brief Common definitions and includes for task management in the system.
 * Created on: July 2, 2025
 * Author: Greg
 */
#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

// WIFI application task
#define WIFI_APP_TASK_STACK_SIZE              4096
#define WIFI_APP_TASK_PRIORITY                5
#define WIFI_APP_TASK_CORE_ID                 0

// HTTP server task
#define HTTP_SERVER_TASK_STACK_SIZE           8192
#define HTTP_SERVER_TASK_PRIORITY             4
#define HTTP_SERVER_TASK_CORE_ID              0

// HTTP server Monitor task
#define HTTP_SERVER_MONITOR_TASK_STACK_SIZE   4098
#define HTTP_SERVER_MONITOR_TASK_PRIORITY     3
#define HTTP_SERVER_MONITOR_TASK_CORE_ID      0

// WiFi Reset Button task
#define WIFI_RESET_BUTTON_TASK_STACK_SIZE     2048
#define WIFI_RESET_BUTTON_TASK_PRIORITY       6
#define WIFI_RESET_BUTTON_TASK_CORE_ID        0

// DHT11 sensor task
#define DHT11_TASK_STACK_SIZE                 4096
#define DHT11_TASK_PRIORITY                   5
#define DHT11_TASK_CORE_ID                    1

// SNTP Time Sync task
#define SNTP_TIME_SYNC_TASK_STACK_SIZE        4096
#define SNTP_TIME_SYNC_TASK_PRIORITY          4
#define SNTP_TIME_SYNC_TASK_CORE_ID           1

#endif /* MAIN_TASKS_COMMON_H_ */