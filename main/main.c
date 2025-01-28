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
#include "esp_event_base.h"
#include "wifi.h"
#include "sgp30.h"
#include "mqtt_client.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_timer.h"

#include "ota.h"


#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4
#define EXAMPLE_ESP_MAXIMUM_RETRY  5


static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;
static const char *TAG = "main";
bool posicionado = false;

sgp30_t aqSensor;
i2c_master_bus_handle_t bus_handle;
esp_timer_handle_t periodic_timer;
esp_event_loop_handle_t loop_h;


int tvoc = 5, co2 = 5;

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

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    //esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_COMP_CONNECTED:
            ESP_LOGI(TAG, "Conectado a MQTT");
            // const esp_timer_create_args_t periodic_timer_args = {
            //     .callback = &periodic_timer_callback,
            //     /* name is optional, but may help identify the timer when debugging */
            //     .name = "periodic"
            // };
            
            // ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
            // ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 5000000));


            break;

        case MQTT_COMP_OTA:
            // ESP_LOGI(TAG, "Nueva informacion de OTA disponible");
            // esp_mqtt_client_handle_t client =  unregister_mqtt_handler();
            // subscribe("v1/devices/me/attributes/response/+");
            // subscribe("v2/fw/response/+/chunk/+");
            //main_ota(client);
            break;

        default:
            //ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            //ESP_LOGI(TAG, "MQTT data received on topic %.*s: %.*s", event->topic_len, event->topic, event->data_len, event->data);
            //ESP_LOGI(TAG, "Other event=%s", cJSON_Print(response));
            break;
    }
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
    
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 20000000));
    
    //Comprobamos si estamos provisionados
    esp_event_loop_args_t loop_args = {
        .queue_size = 5,
        .task_name = "loop_task",
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 1024,
        .task_core_id = tskNO_AFFINITY
    };

    ESP_ERROR_CHECK(esp_event_loop_create(&loop_args, &loop_h));
    ESP_ERROR_CHECK(esp_event_handler_instance_register_with(loop_h, MQTT_COMP_EVENTS, ESP_EVENT_ANY_ID, mqtt_event_handler, loop_h, NULL));
    main_wifi();
    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */

    ESP_LOGI(TAG, "Conexión WiFi establecida.");

     // Esperar a que wifi se conecte
    vTaskDelay(5000 / portTICK_PERIOD_MS);



    //1-  Inicializar MQTT
    ESP_LOGI(TAG, "Iniciando MQTT...");
    mqtt_app_start(get_thingsboard_json(), &loop_h);
    ESP_LOGI(TAG, "MQTT inicializado.");

    // Esperar a que MQTT se conecte
    vTaskDelay(2000 / portTICK_PERIOD_MS);
}

static void periodic_timer_callback(void* arg)
{
    if (isConnected()) {
        if(!posicionado){
            send_atribute();
            ESP_LOGI(TAG, "Posicionamiento enviado");
            posicionado = true;
        }
        sgp30_get_co2_and_tvoc(&aqSensor, &co2, &tvoc);
        cJSON *root = cJSON_CreateObject();
        //cJSON_AddStringToObject(root, "deviceName", "");
        cJSON_AddNumberToObject(root, "co2", co2);
        cJSON_AddNumberToObject(root, "tvoc", tvoc);

        char *json_data = cJSON_Print(root);
        if (json_data)
        {
            send_data(json_data);
            ESP_LOGI(TAG, "Sent provisioning request: %s", json_data);
            free(json_data);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to create JSON data");
        }

        cJSON_Delete(root);
    }
}
