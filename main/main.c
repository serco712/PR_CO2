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
void app_main(void)
{
//Primero comprobamos si venimos de una OTA


//Comprobamos si estamos provisionados

    int msg_id;
    //main_wifi();
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
