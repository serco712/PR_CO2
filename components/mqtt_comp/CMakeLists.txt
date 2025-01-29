idf_component_register(SRCS "mqtt_sensor.c"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt json esp_wifi nvs_flash app_update)
