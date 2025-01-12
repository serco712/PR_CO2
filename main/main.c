<<<<<<< Updated upstream
=======
#include <stdio.h>
>>>>>>> Stashed changes
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <esp_wifi.h>

static const char *TAG = "mqtt_example";
const char *topic_data = "Informatica/3/Lab/CO2/data";
const char *topic_enable = "Informatica/3/Lab/CO2/enable";
const char *topic_disable = "Informatica/3/Lab/CO2/disable";
int QoS = 1;

static esp_mqtt_client_handle_t client;
esp_mqtt_client_handle_t mqtt_get_client() {
    return client;
}

void test_mqtt_functionality(void) {

      // 2. Obtener el cliente
    esp_mqtt_client_handle_t client = mqtt_get_client();
    if (client == NULL) {
        ESP_LOGE("MQTT_APP", "Failed to get MQTT client. Aborting!");
        return;
    }
    
     // 3. Publicar un mensaje
   
    const char *message = "Hola desde ESP32c3";
 
    int msg_id = esp_mqtt_client_publish(topic_data,message, strlen(message), QoS, 0);
    if (msg_id != -1) {
        ESP_LOGI(TAG, "Mensaje publicado con éxito, msg_id=%d", msg_id);
    } else {
        ESP_LOGE(TAG, "Error al publicar el mensaje.");
    }

    // 4. Suscribirse a un topic para recibir mensajes
    esp_mqtt_client_subscribe(client,topic_enable, 1);
    ESP_LOGI(TAG, "Suscripción al topic: %s",topic_enable);


}

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
    wifi_prov_init();

    ESP_LOGI(TAG, "Conexión WiFi establecida.");

     // Esperar a que wifi se conecte
    vTaskDelay(5000 / portTICK_PERIOD_MS);



    //1-  Inicializar MQTT
    ESP_LOGI(TAG, "Iniciando MQTT...");
    mqtt_start();
    ESP_LOGI(TAG, "MQTT inicializado.");

    // Esperar a que MQTT se conecte
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    // Probar funcionalidad MQTT
    test_mqtt_functionality();
<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes
}
