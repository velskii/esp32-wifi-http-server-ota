#set(EXTRA_COMPONENT_DIRS components)

idf_component_register(
    SRCS 
        "main.c"
        "rgb_led.c"
        "wifi_app.c"
        "dht11.c"
        "http_server.c"
        "http_handlers_static.c"
        "http_handlers_wifi.c"
        "http_server_monitor.c"
        "http_handlers_sensor.c"
        "http_handlers_ota.c"
        "http_handlers_sntp.c"
        "http_handlers_ap_ssid.c"
        "app_nvs.c"
        "wifi_reset_button.c"
        "sntp_time_sync.c"
        "aws_iot.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        aws_iot
    REQUIRES 
        driver
        app_update
        esp_wifi
        esp_http_server
        nvs_flash
        esp_timer
        dht
        vfs
        fatfs
        aws_iot
    EMBED_FILES 
    "webpage/index.html" 
    "webpage/app.css" 
    "webpage/app.js" 
    "webpage/favicon.ico" 
    "webpage/jquery-3.3.1.min.js"
)

target_add_binary_data(${COMPONENT_TARGET} "certs/aws_root_ca_pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/certificate_pem_crt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/private_pem_key" TEXT)
                       
                       