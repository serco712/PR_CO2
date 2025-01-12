idf_component_register(SRCS "mqtt_sensor.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi esp_event nvs_flash tcp_transport)
