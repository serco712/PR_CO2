/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "mqtt5_client.h"
#include "mqtt_sensor.h"
 
static int pub_interval = CONFIG_SEND_INTERVAL;
bool pub_enabled = true;

static const char *TAG = "mqtt_example";
const char *topic_data = "Informatica/3/Lab/CO2/data";
const char *topic_enable = "Informatica/3/Lab/CO2/enable";
const char *topic_disable = "Informatica/3/Lab/CO2/disable";
int QoS = 1;
const char prov_msg[] = "{\"provisionDeviceKey\": \"huxcz6wakl41mlkv2zzq\", \"provisionDeviceSecret\": \"e8i77gmt00x8313coc9e\", \"deviceName\": \"ESP_EF\"}";
const char *provision_topic_request = "/provision/request";
const char *provision_topic_response = "/provision/response";
bool provisionado = false;

typedef struct {
    esp_mqtt_client_handle_t client;
    int pub_interval;
} pub_task_params_t;


//Función que llama la tarea para publicar los valores aleatoriamente

void pub_task(void *param){

    pub_task_params_t *params = (pub_task_params_t *) param;
    esp_mqtt_client_handle_t client = params->client;

    while (pub_enabled) {
        char *data = "{\"data\": \"hola\"}";
        esp_mqtt_client_publish(client, topic_data, data, strlen(data), QoS, 0);
        ESP_LOGI(TAG, "Publicado: %s", data);
        vTaskDelay(params -> pub_interval / portTICK_PERIOD_MS);
    }

}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, provision_topic_response, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_publish(client, provision_topic_request, prov_msg, strlen(prov_msg), QoS, 0);
        ESP_LOGI(TAG, "sent provision publish successful, msg_id=%d", msg_id);

        

        /* Lógica de control de Edificio*/

        //Intervalo de tiempo
        msg_id = esp_mqtt_client_subscribe(client, topic_data, 0);
        ESP_LOGI(TAG, "sent subscribe successful intervalo, msg_id=%d", msg_id);

          //Enable
        msg_id = esp_mqtt_client_subscribe(client, topic_enable, 0);
        ESP_LOGI(TAG, "sent subscribe successful enable, msg_id=%d", msg_id);

          //Disable
        msg_id = esp_mqtt_client_subscribe(client, topic_disable, 0);
        ESP_LOGI(TAG, "sent subscribe successful disable, msg_id=%d", msg_id);

        /* Lógica de control de Edificio*/

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;

    //case MQTT_EVENT_PUB
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, QoS, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_DATA:

        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        // Controlar intervalo de publicación
        if (strncmp(event->topic, topic_data, event->topic_len) == 0) {
            pub_interval = atoi(event->data);
            ESP_LOGI(TAG, "Intervalo actualizado a %d ms", pub_interval);
        }
        // Habilitar sensor
        else if (strncmp(event->topic, topic_enable, event->topic_len) == 0) {
            pub_enabled = true;
            ESP_LOGI(TAG, "Sensor habilitado");
        }
        // Deshabilitar sensor
        else if (strncmp(event->topic, topic_disable, event->topic_len) == 0) {
            pub_enabled = false;
            ESP_LOGI(TAG, "Sensor deshabilitado");
            }
          
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}




static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
        .credentials.client_id = "co2",
        .credentials.username = "senslab",
        .credentials.authentication.password = "senslabMIOT",       //cambiar a CONFIG_MQTT_PSSWD
    };
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);


    // Estructura con los parámetros de la función pusb_Task
    pub_task_params_t *params = malloc(sizeof(pub_task_params_t));
    params->client = client;
    params->pub_interval = pub_interval; // Asignar el valor inicial de intervalo

    // Tarea para publicar datos
    xTaskCreate(pub_task, "task_sample", 2048, params, 5, NULL);
}

void mqtt_sensor_enable() {
    pub_enabled = true;
    ESP_LOGI(TAG, "Publicación habilitada");
}

void mqtt_sensor_disable() {
    pub_enabled = false;
    ESP_LOGI(TAG, "Publicación deshabilitada");
}


