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

static const char *TAG = "mqtt_example";
const char *topic_data_a = "Informatica/3/Lab/CO2/data";
const char *topic_enable_a = "Informatica/3/Lab/CO2/enable";
const char *topic_disable_a = "Informatica/3/Lab/CO2/disable";
int QoS_a = 1;

static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

// static esp_mqtt_client_handle_t client;
// esp_mqtt_client_handle_t mqtt_get_client() {
//     return client;
// }

// void test_mqtt_functionality(void) {

//       // 2. Obtener el cliente
//     esp_mqtt_client_handle_t client = mqtt_get_client();
//     if (client == NULL) {
//         ESP_LOGE("MQTT_APP", "Failed to get MQTT client. Aborting!");
//         return;
//     }
    
//      // 3. Publicar un mensaje
   
//     const char *message = "Hola desde ESP32c3";
 
//     int msg_id = esp_mqtt_client_publish(topic_data,message, strlen(message), QoS, 0);
//     if (msg_id != -1) {
//         ESP_LOGI(TAG, "Mensaje publicado con éxito, msg_id=%d", msg_id);
//     } else {
//         ESP_LOGE(TAG, "Error al publicar el mensaje.");
//     }

//     // 4. Suscribirse a un topic para recibir mensajes
//     esp_mqtt_client_subscribe(client,topic_enable, 1);
//     ESP_LOGI(TAG, "Suscripción al topic: %s",topic_enable);


// }
void app_main(void)
{
//Primero comprobamos si venimos de una OTA


//Comprobamos si estamos provisionados


    main_wifi();
    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */

    ESP_LOGI(TAG, "Conexión WiFi establecida.");

     // Esperar a que wifi se conecte
    vTaskDelay(5000 / portTICK_PERIOD_MS);



    //1-  Inicializar MQTT
    ESP_LOGI(TAG, "Iniciando MQTT...");
    mqtt_app_start();
    ESP_LOGI(TAG, "MQTT inicializado.");

    // Esperar a que MQTT se conecte
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // Probar funcionalidad MQTT
    //test_mqtt_functionality();
}
