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

#define EXAMPLE_ESP_WIFI_SSID      "Redmi9A"
#define EXAMPLE_ESP_WIFI_PASS      "uddhjdk623"
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
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    //Fase 4: conexion wifi
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP){
        ESP_LOGI(TAG,"WiFi driver stopped");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED){ //Comprueba que esta conectado al dispositivo e imprime un mesaje
        ESP_LOGI(TAG,"Connected");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_BEACON_TIMEOUT){ //La estación no ha recibido el beacon del AP (punto de acceso) en el tiempo limite  
        ESP_LOGI(TAG,"Beacon timeout!");
    }
    //Fase 6: desconexion
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } 
    
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        
        //Fase 7: Cambio de IP
        if(event->ip_changed){ //Informa de que la IP ha cambiado, este mensaje saldra siempre que se conecta el dispositivo por primera vez
            ESP_LOGI(TAG,"ip changed!");
        }
        // Fase 5: conexion ip
        
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        
        
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_GOT_IP6) { //SIgue el procedimiento de la IPv4 pero con la IPv6
    
        ip_event_got_ip6_t* event = (ip_event_got_ip6_t*) event_data;
                
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip6_info.ip));
        
        
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP) {
    
       ESP_LOGI(TAG,"ip lost!");
           
    }
}


void app_main(void)
{
//Primero comprobamos si venimos de una OTA


//Comprobamos si estamos provisionados

// Inicializar NVS
    //flash_init();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // Inicializar stack de red
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "Conectando al WiFi...");
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK,
            .sae_h2e_identifier = "",
        },

    };


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    //main_wifi();
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
