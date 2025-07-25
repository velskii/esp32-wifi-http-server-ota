#ifndef DHT11_H_  
#define DHT11_H_


#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2

#define DHT_GPIO_PIN			33

/**
 * Starts DHT11 sensor task
 */
void DHT11_task_start(void);

#endif