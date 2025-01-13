#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <esp_wifi.h>


#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "mqtt_sensor.h"
#include "wifi.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define EXAMPLE_ESP_WIFI_SSID      "Redmi9A"
#define EXAMPLE_ESP_WIFI_PASS      "uddhjdk623"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

static const char *TAG = "mqtt_example";
const char *topic_data_a = "Informatica/3/Lab/CO2/data";
const char *topic_enable_a = "Informatica/3/Lab/CO2/enable";
const char *topic_disable_a = "Informatica/3/Lab/CO2/disable";
int QoS_a = 1;

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

// Inicializar NVS
    //flash_init();

    // Inicializar stack de red
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Conectando al WiFi...");
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                    .required = true,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    //main_wifi();

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
