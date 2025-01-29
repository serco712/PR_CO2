#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_ota_ops.h"

#include "esp_log.h"
#include "mqtt_client.h"


#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

static const char *TAG = "OTA";

static esp_ota_handle_t ota_handle = 0;
static const esp_partition_t *update_partition = NULL;
static int total_received = 0;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
   /* ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    ESP_LOGD(TAG, "free heap size is %" PRIu32 ", minimum %" PRIu32, esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_DATA:
        


        case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "Error en MQTT");
        break;


        default:
        break;

    }*/
}

void update_firmware(esp_ota_handle_t ota_handler, esp_partition_t particion) {
    ESP_LOGI(TAG,"Actualizando el firmware...");
    if (esp_ota_end(ota_handler) == ESP_OK){
        if (esp_ota_set_boot_partition(&particion) == ESP_OK) { //cambiamos la particion de arranque
            ESP_LOGI(TAG,"Actualizacion OTA correcta. Reiniciando...");
            esp_restart(); //reboot el nodo.
            //Y al reiniciar, mandar telemetria como para decir que esta correcto el proceso de actualziacion de ota.
        }

    }
}

void main_ota(esp_mqtt_client_handle_t client){ //seguramente haya que eliminarlo y sustituirlo por update_firmware
    //esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL); //Registramos el handler


}