#ifndef MAIN_WIFI_RESET_BUTTON_H_
#define MAIN_WIFI_RESET_BUTTON_H_

// Default interrupt flag
#define ESP_INTR_FLAG_DEFAULT 0

// Wifi reset button it the boot button on the DevKit
#define WIFI_RESET_BUTTON 0

// Congifures Wifi reset button and interrupt configuration
void wifi_reset_button_config(void);


#endif // MAIN_WIFI_RESET_BUTTON_H_