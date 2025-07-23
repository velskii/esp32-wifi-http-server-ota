idf_component_register(SRCS "http_server_monitor.c" "http_server.c" "http_server.backup.c" "http_handlers_sensor.c" "http_handlers_ota.c" "http_handlers_wifi.c" "http_handlers_static.c" "coding_style.c" "main.c" "rgb_led.c" "wifi_app.c" "dht11.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver app_update esp_wifi esp_http_server nvs_flash esp_timer
                       EMBED_FILES "webpage/index.html" "webpage/app.css" "webpage/app.js" "webpage/favicon.ico" "webpage/jquery-3.3.1.min.js")
