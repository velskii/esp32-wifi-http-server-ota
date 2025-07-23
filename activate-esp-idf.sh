#!/bin/bash

ESP_IDF_PATH="$HOME/esp/esp-idf"

if [ ! -d "$ESP_IDF_PATH" ]; then
  echo "ESP-IDF path does not exist: $ESP_IDF_PATH"
fi

echo "Activating ESP-IDF: $ESP_IDF_PATH"
source "$ESP_IDF_PATH/export.sh"

idf.py --version && echo "Activated ESP-IDF"