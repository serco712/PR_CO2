idf_component_register(SRCS "wifi.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi mqtt_comp wifi_provisioning)

