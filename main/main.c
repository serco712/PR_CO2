#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <esp_wifi.h>

#include "nvs_flash.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "mqtt_sensor.h"
#include "wifi.h"
#include "include/sgp30.h"
#include "mqtt_client.h"
#include "lwip/err.h"
#include "lwip/sys.h"


#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4
#define EXAMPLE_ESP_MAXIMUM_RETRY  5


static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

static const char *TAG = "mqtt_component";
static const char *topic_attributes = "v1/devices/me/attributes";
static const char *topic_data = "v1/devices/me/telemetry";
/*const char *topic_data = "Informatica/3/Lab/CO2/data";
const char *topic_enable = "Informatica/3/Lab/CO2/enable";
const char *topic_disable = "Informatica/3/Lab/CO2/disable";*/
int QoS = 1;
const char *PROVISION_REQUEST_TOPIC = "/provision/request";
const char *PROVISION_RESPONSE_TOPIC = "/provision/response";

data = "hola";
len = sizeof(data);

// }

static void periodic_timer_callback(void* arg);

void init_i2c(void) {
    i2c_master_bus_config_t i2c_bus_config = {
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .i2c_port = I2C_NUM_0,
            .scl_io_num = 22,
            .sda_io_num = 21,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = true,
        };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));
    sgp30_init(&aqSensor, bus_handle, SGP30_I2C_ADDR);
}


void app_main(void)
{
//Primero comprobamos si venimos de una OTA
    init_i2c();
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        /* name is optional, but may help identify the timer when debugging */
        .name = "periodic"
    };

    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 5000000));
    //Comprobamos si estamos provisionados
    int msg_id;
    main_wifi();
    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */

    ESP_LOGI(TAG, "Conexión WiFi establecida.");

     // Esperar a que wifi se conecte
    vTaskDelay(5000 / portTICK_PERIOD_MS);



    //1-  Inicializar MQTT
    ESP_LOGI(TAG, "Iniciando MQTT...");
    mqtt_app_start(get_thingsboard_json());
    ESP_LOGI(TAG, "MQTT inicializado.");

    // Esperar a que MQTT se conecte
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    client = mqtt_client();
    msg_id = esp_mqtt_client_subscribe(client, topic_attributes, QoS);
    esp_mqtt_client_publish(client, topic_data, data, len, QoS);

}

static void periodic_timer_callback(void* arg)
{
    sgp30_get_co2_and_tvoc(&aqSensor, &co2, &tvoc);
    cJSON *root = cJSON_CreateObject();
    //cJSON_AddStringToObject(root, "deviceName", "");
    cJSON_AddNumberToObject(root, "co2", co2);
    cJSON_AddNumberToObject(root, "tvoc", tvoc);

    char *json_data = cJSON_Print(root);
    if (json_data)
    {
        esp_mqtt_client_publish(client, "v1/devices/me/telemetry", json_data, 0, 1, 0);
        ESP_LOGI(TAG, "Sent provisioning request: %s", json_data);
        free(json_data);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create JSON data");
    }

    cJSON_Delete(root);
}
