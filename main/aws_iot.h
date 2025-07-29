#ifndef MAIN_AWS_IOT_H
#define MAIN_AWS_IOT_H

#include "core_mqtt.h"
#include "transport_interface.h"
#include "esp_err.h"

#define AWS_IOT_CLIENT_IDENTIFIER "esp32-client"
#define AWS_IOT_TOPIC            "esp32/topic"
#define AWS_IOT_TOPIC_LENGTH     (sizeof(AWS_IOT_TOPIC) - 1)

esp_err_t aws_iot_mqtt_connect(MQTTContext_t *mqttContext,
                               TransportInterface_t *transport,
                               MQTTFixedBuffer_t *networkBuffer);

esp_err_t aws_iot_mqtt_publish(MQTTContext_t *mqttContext,
                               const char *message);

esp_err_t aws_iot_mqtt_subscribe(MQTTContext_t *mqttContext);

#endif // MAIN_AWS_IOT_H
