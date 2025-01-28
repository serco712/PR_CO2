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
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    ESP_LOGD(TAG, "free heap size is %" PRIu32 ", minimum %" PRIu32, esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_DATA:
        // Procesar datos recibidos
        if (strstr(event->topic, "chunk") != NULL) {
            ESP_LOGI(TAG, "Recibiendo parte del firmware (%d bytes)...", event->data_len);

            if (ota_handle == 0) {
                // Inicializar OTA
                update_partition = esp_ota_get_next_update_partition(NULL);
                esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
                ESP_LOGI(TAG, "Iniciando partición OTA");
                total_received = 0;
            }

            esp_ota_write(ota_handle, event->data, event->data_len);
            total_received += event->data_len;

            ESP_LOGI(TAG, "Total recibido: %d bytes", total_received);
        }
        break;


        case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "Error en MQTT");
        break;


        default:
        break;

    }
}

void main_ota(esp_mqtt_client_handle_t client){
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL); //Registramos el handler


}