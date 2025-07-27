#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "tasks_common.h"
#include "wifi_app.h"
#include "wifi_reset_button.h"

static const char TAG[] = "wifi_reset_button";

// Semaphore handle for the button press
SemaphoreHandle_t wifi_reset_semaphore = NULL;

/**
 * ISR handler for the WiFi reset button (BOOT button).
 * @param arg parameter passed to the ISR handler (not used).
 */
void IRAM_ATTR wifi_reset_button_isr_handler(void *arg) {
    // Give the semaphore to signal the button press
    if (wifi_reset_semaphore != NULL) {
      // Notify the button task
        xSemaphoreGiveFromISR(wifi_reset_semaphore, NULL);
    } else {
        ESP_LOGE(TAG, "Semaphore not initialized");
    }
}

/**
 * WiFi reset button task reacts to a BOOT button press event by sending a message to the WiFi application, to disconnect the current WiFi connection.
 * @param pvParam Pointer to task parameters (not used).
 */
void wifi_reset_button_task(void *pvParam)
{
    while (1){
      // Wait for the button press event
      if (xSemaphoreTake(wifi_reset_semaphore, portMAX_DELAY) == pdTRUE) {
          ESP_LOGI(TAG, "WiFi reset button pressed, disconnecting WiFi...");
          // Notify the WiFi application to disconnect and clear credentials
          wifi_app_send_message(WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT);

          vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay to prevent multiple triggers
      }
    }
}

void wifi_reset_button_config(void) {
    // Create the binary semaphore
    wifi_reset_semaphore = xSemaphoreCreateBinary();

    // Configure the button and set the direction
    esp_rom_gpio_pad_select_gpio(WIFI_RESET_BUTTON);
    // gpio_pad_select_gpio(WIFI_RESET_BUTTON);
    gpio_set_direction(WIFI_RESET_BUTTON, GPIO_MODE_INPUT);

    // Enable interrupt on the negative edge (button press)
    gpio_set_intr_type(WIFI_RESET_BUTTON, GPIO_INTR_NEGEDGE);

    // Create the WiFi reset button task 
    xTaskCreatePinnedToCore(&wifi_reset_button_task, "wifi_reset_button", WIFI_RESET_BUTTON_TASK_STACK_SIZE, NULL, WIFI_RESET_BUTTON_TASK_PRIORITY, NULL, WIFI_RESET_BUTTON_TASK_CORE_ID);

    // Install gpio isr service
    if (gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install GPIO ISR service");
        return;
    }

    // Attach the interrupt handler for the WiFi reset button
    if (gpio_isr_handler_add(WIFI_RESET_BUTTON, wifi_reset_button_isr_handler, NULL) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add ISR handler for WiFi reset button");
        return;
    }

    // Configure the GPIO for the WiFi reset button
    // gpio_config_t io_conf = {
    //     .intr_type = GPIO_INTR_NEGEDGE, // Trigger on falling edge
    //     .mode = GPIO_MODE_INPUT,         // Set as input mode
    //     .pin_bit_mask = (1ULL << WIFI_RESET_BUTTON), // Pin mask for the button
    //     .pull_down_en = 0,               // Disable pull-down resistor
    //     .pull_up_en = 1                  // Enable pull-up resistor
    // };
    
    // // Apply the configuration
    // if (gpio_config(&io_conf) != ESP_OK) {
    //     ESP_LOGE(TAG, "Failed to configure WiFi reset button GPIO");
    // } else {
    //     ESP_LOGI(TAG, "WiFi reset button configured successfully");
    // }
}