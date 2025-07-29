#include <string.h>

#include "aws_iot.h"
#include "core_mqtt.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"

static const char *TAG = "AWS_IOT";

static uint32_t get_time_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

esp_err_t aws_iot_mqtt_connect(MQTTContext_t *mqttContext,
                               TransportInterface_t *transport,
                               MQTTFixedBuffer_t *networkBuffer)
{
    MQTTStatus_t mqttStatus;

    MQTTStatus_t status = MQTT_Init(mqttContext,
                                    transport,
                                    get_time_ms,
                                    NULL,
                                    networkBuffer);

    if (status != MQTTSuccess)
    {
        ESP_LOGE(TAG, "MQTT_Init failed: %d", status);
        return ESP_FAIL;
    }

    MQTTConnectInfo_t connectParams = {
        .cleanSession = true,
        .keepAliveSeconds = 60,
        .pClientIdentifier = AWS_IOT_CLIENT_IDENTIFIER,
        .clientIdentifierLength = strlen(AWS_IOT_CLIENT_IDENTIFIER),
    };

    bool sessionPresent;
    mqttStatus = MQTT_Connect(mqttContext,
                              &connectParams,
                              NULL,
                              1000,
                              &sessionPresent);

    if (mqttStatus != MQTTSuccess)
    {
        ESP_LOGE(TAG, "MQTT_Connect failed: %d", mqttStatus);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "MQTT connected to broker!");
    return ESP_OK;
}

esp_err_t aws_iot_mqtt_publish(MQTTContext_t *mqttContext,
                               const char *message)
{
    MQTTPublishInfo_t publishInfo = {
        .qos = MQTTQoS0,
        .retain = false,
        .dup = false,
        .pTopicName = AWS_IOT_TOPIC,
        .topicNameLength = AWS_IOT_TOPIC_LENGTH,
        .pPayload = message,
        .payloadLength = strlen(message),
    };

    MQTTStatus_t status = MQTT_Publish(mqttContext,
                                       &publishInfo,
                                       0);
    if (status != MQTTSuccess)
    {
        ESP_LOGE(TAG, "MQTT_Publish failed: %d", status);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Published: %s", message);
    return ESP_OK;
}

esp_err_t aws_iot_mqtt_subscribe(MQTTContext_t *mqttContext)
{
    MQTTSubscribeInfo_t subscribeInfo = {
        .qos = MQTTQoS0,
        .pTopicFilter = AWS_IOT_TOPIC,
        .topicFilterLength = AWS_IOT_TOPIC_LENGTH,
    };

    MQTTStatus_t status = MQTT_Subscribe(mqttContext,
                                         &subscribeInfo,
                                         1,
                                         0);
    if (status != MQTTSuccess)
    {
        ESP_LOGE(TAG, "MQTT_Subscribe failed: %d", status);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Subscribed to topic: %s", AWS_IOT_TOPIC);
    return ESP_OK;
}
