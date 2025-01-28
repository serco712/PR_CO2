idf_component_register(SRCS "wifi.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES mqtt_comp sgp30_comp wifi_provisioning ota)

