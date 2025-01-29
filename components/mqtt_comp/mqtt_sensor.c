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
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include "mqtt_sensor.h"

#include "esp_ota_ops.h"

#define PROVISION_REQUEST_TOPIC "/provision/request"
#define PROVISION_RESPONSE_TOPIC "/provision/response"

ESP_EVENT_DEFINE_BASE(MQTT_COMP_EVENTS);

static const char *TAG = "mqtt_component";

esp_event_loop_handle_t loop_connect;
char *client_attribute;
char *provision_device_key;
char *provision_device_secret;
char *device_name;
static int pub_interval;
bool pub_enabled;
bool mqtt_connected = false;
bool ota = false;
bool jerarquizado = false;

static esp_mqtt_client_handle_t client = NULL;

bool provisionado = false;

char provisioned_client_username[64] = {0}; 

//Funcion para suscribir clientes a diferentes topics de mensajes MQTT
void subscribe(char* topic) {
    esp_mqtt_client_subscribe(client, topic, 1);
}

bool isConnected() {
    return mqtt_connected;
}

bool activeOTA(){
    return ota;
}

static esp_ota_handle_t ota_handle = 0;
static const esp_partition_t *update_partition = NULL;
static int total_received = 0;


//Función que llama la tarea para publicar los valores aleatoriamente

void save_credentials(const char *credentials)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK)
    {
        nvs_set_str(nvs_handle, "credentials", credentials);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        ESP_LOGI(TAG, "Credentials saved to NVS");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to open NVS");
    }
}

void send_provision_request()
{
    cJSON *root = cJSON_CreateObject();
    //cJSON_AddStringToObject(root, "deviceName", device_name);
    cJSON_AddStringToObject(root, "provisionDeviceKey", provision_device_key);
    cJSON_AddStringToObject(root, "provisionDeviceSecret", provision_device_secret);

    char *json_data = cJSON_Print(root);
    if (json_data)
    {
        esp_mqtt_client_publish(client, PROVISION_REQUEST_TOPIC, json_data, 0, 1, 0);
        ESP_LOGI(TAG, "Sent provisioning request: %s", json_data);
        free(json_data);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create JSON data");
    }

    cJSON_Delete(root);
}
//

// Error logging function
static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

void pub_task(void *param){

    // pub_task_params_t *params = (pub_task_params_t *) param;
    // client = params->client;

    // while (pub_enabled) {
    //     char *data = "{\"data\": \"hola\"}";
    //     esp_mqtt_client_publish(client, topic_data, data, strlen(data), QoS, 0);
    //     ESP_LOGI(TAG, "Publicado: %s", data);
    //     vTaskDelay(params -> pub_interval / portTICK_PERIOD_MS);
    // }

}

static void mqtt_event_handler_not_prov(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Conectado al broker MQTT");
            esp_mqtt_client_subscribe(client, PROVISION_RESPONSE_TOPIC, 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Desconectado del broker MQTT");
            reconnect_mqtt_not_prov();
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT data received on topic %.*s: %.*s", event->topic_len, event->topic, event->data_len, event->data);
            if (strncmp(event->topic, PROVISION_RESPONSE_TOPIC, event->topic_len) == 0)
            {
                cJSON *response = cJSON_Parse(event->data);
                if (response)
                {
                    const cJSON *status = cJSON_GetObjectItem(response, "status");
                    if (status && strcmp(status->valuestring, "SUCCESS") == 0)
                    {
                        ESP_LOGI(TAG, "Provisioning successful, status saved: %s", status->valuestring);
                        const cJSON *credentials = cJSON_GetObjectItem(response, "credentialsValue");
                        if (credentials)
                        {
                            save_credentials(credentials->valuestring);
                            ESP_LOGI(TAG, "Provisioning successful, credentials saved: %s", credentials->valuestring);
                            strcpy(provisioned_client_username, credentials->valuestring);
                            esp_mqtt_client_disconnect(client);
                            //reconnect_mqtt();
                        }
                    }
                    else
                    {
                        ESP_LOGE(TAG, "Provisioning failed: %s", cJSON_GetObjectItem(response, "errorMsg")->valuestring);
                    }
                    cJSON_Delete(response);
                }
            }
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "Suscrito");
            send_provision_request();
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
            ESP_LOGI(TAG, "MQTT data received on topic %.*s: %.*s", event->topic_len, event->topic, event->data_len, event->data);
            //ESP_LOGI(TAG, "Other event=%s", cJSON_Print(response));
            break;
    }
}

static void mqtt_event_handler_prov(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    int msg_id, response_id;
    
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        //ESP_ERROR_CHECK(esp_event_post_to(loop_connect, MQTT_COMP_EVENTS, MQTT_COMP_CONNECTED, NULL, 0, portMAX_DELAY));
        mqtt_connected = true;
        break;

    //case MQTT_EVENT_PUB
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        mqtt_connected = false;
        reconnect_mqtt_prov();
        break;

    case MQTT_EVENT_SUBSCRIBED:
        // ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, QoS, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_DATA:

        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        // printf("DATA=%.*s\r\n", event->data_len, event->data);

        // // Controlar intervalo de publicación
        // if (strncmp(event->topic, topic_data, event->topic_len) == 0) {
        //     pub_interval = atoi(event->data);
        //     ESP_LOGI(TAG, "Intervalo actualizado a %d ms", pub_interval);
        // }
        // // Habilitar sensor
        // else if (strncmp(event->topic, topic_enable, event->topic_len) == 0) {
        //     pub_enabled = true;
        //     ESP_LOGI(TAG, "Sensor habilitado");
        // }
        // // Deshabilitar sensor
        // else if (strncmp(event->topic, topic_disable, event->topic_len) == 0) {
        //     pub_enabled = false;
        //     ESP_LOGI(TAG, "Sensor deshabilitado");
        //     }
        if (strstr(event->topic, "fw") != NULL) { //se ha lanzado una campaña de OTA
            //esp_event_post_to(loop_connect, MQTT_COMP_EVENTS, MQTT_COMP_OTA, NULL, 0, portMAX_DELAY);
            ota = true;
            // esp_mqtt_client_subscribe(client, "v1/devices/me/attributes/response/+");
            // esp_mqtt_client_subscribe(client, "v2/fw/response/+/chunk/+");
        }
        else if (activeOTA()){
            // Procesar datos recibidos del nuevo firmware de la OTA
            if (strstr(event->topic, "chunk") != NULL) {
                ESP_LOGI(TAG, "Recibiendo parte del firmware (%d bytes)...", event->data_len);

                if (ota_handle == 0) {
                    // Inicializar OTA
                    update_partition = esp_ota_get_next_update_partition(NULL);
                    esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
                    ESP_LOGI(TAG, "Iniciando partición OTA");
                    total_received = 0;
                }

                esp_ota_write(ota_handle, event->data, event->data_len); //escribimos chunk a chunk del fw recibido en la particion de OTA.
                total_received += event->data_len;

                ESP_LOGI(TAG, "Total recibido: %d bytes", total_received);
                //update_firmware(ota_handle, update_partition);

                //coger el dato del atributo fw_size haciendo una publicacion sobre el atributo compartido para esperar la respuesta (en un JSON)
                //if fw_size == total_received: ota=false; update_firmware()
            }
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


void reconnect_mqtt_not_prov() {
    esp_mqtt_client_unregister_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler_not_prov);
    reconnect_mqtt_prov();
}

void reconnect_mqtt_prov() {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    size_t required_size = 0;
    nvs_get_str(nvs_handle, "URI", NULL, &required_size);
    char *uri = malloc(required_size);
    nvs_get_str(nvs_handle, "URI", uri, &required_size);
    nvs_get_str(nvs_handle, "credentials", NULL, &required_size);
    char* prov = malloc(required_size);
    nvs_get_str(nvs_handle, "credentials", prov, &required_size);
    ESP_LOGI(TAG, "required size=%d", required_size);
    if (required_size != 0) {
        ESP_LOGI(TAG, "URI=%s", uri);
        ESP_LOGI(TAG, "access token=%s", prov);
        nvs_close(nvs_handle);
        esp_mqtt_client_config_t mqtt_cfg = {
            .broker.address.uri = uri,
            .credentials.username = prov,
            .credentials.client_id = ""
        };

        client = esp_mqtt_client_init(&mqtt_cfg);
        if (!client) {
            ESP_LOGE(TAG, "Failed to initialize MQTT client");
            return;
        }

        // Registrar el manejador de eventos
        esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler_prov, NULL);
    }
    else
        esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler_not_prov, NULL);
    
    // Iniciar el cliente MQTT
    esp_mqtt_client_start(client);
}


void mqtt_app_start_not_prov(cJSON *data)
{
    const cJSON *uri = cJSON_GetObjectItem(data, "URI");
    const cJSON *key = cJSON_GetObjectItem(data, "deviceKey");
    const cJSON *secret = cJSON_GetObjectItem(data, "deviceSecret");
    const cJSON *jerarquia = cJSON_GetObjectItem(data, "jerarquia");

    ESP_LOGI(TAG, "URI=%s", uri->valuestring);
    ESP_LOGI(TAG, "key=%s", key->valuestring);
    ESP_LOGI(TAG, "secret=%s", secret->valuestring);
    ESP_LOGI(TAG, "jerarquia=%s", jerarquia->valuestring);

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK)
    {
        nvs_set_str(nvs_handle, "URI", uri->valuestring);
        nvs_set_str(nvs_handle, "deviceKey", key->valuestring);
        nvs_set_str(nvs_handle, "deviceSecret", secret->valuestring);
        //nvs_set_str(nvs_handle, "jerarquia", jerarquia->valuestring);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        ESP_LOGI(TAG, "Credentials saved to NVS");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to open NVS");
    }

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = uri->valuestring,
        .credentials.client_id = "",
        .credentials.username = "provision"
    };

    provision_device_key = key->valuestring;
    provision_device_secret = secret->valuestring;
    client_attribute = jerarquia->valuestring;

    client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler_not_prov, NULL);

    esp_mqtt_client_start(client);
}

void mqtt_app_start(char *json_data, esp_event_loop_handle_t *loop) {
    loop_connect = *loop;
    ESP_LOGI(TAG, "json data=%s", json_data);
    cJSON *data = cJSON_Parse(json_data);
    if (data)
        mqtt_app_start_not_prov(data);
    else{
        jerarquizado = true;
        reconnect_mqtt_prov();
    }
}

void mqtt_sensor_enable() {
    pub_enabled = true;
    ESP_LOGI(TAG, "Publicación habilitada");
}

void mqtt_sensor_disable() {
    pub_enabled = false;
    ESP_LOGI(TAG, "Publicación deshabilitada");
}

void send_data(char* data) {
    esp_mqtt_client_publish(client, "v1/devices/me/telemetry", data, 0, 1, 0);
}

void send_atribute() {
    char atributo_json[100];
    char inicio[20] = "{\"Jerarquia\":\"";
    char final[5]= "\"}";
    snprintf(atributo_json, sizeof(atributo_json), "%s%s%s", inicio, client_attribute, final);
    ESP_LOGI(TAG, "JSON de la jerarquia: %s", atributo_json);
    esp_mqtt_client_publish(client, "v1/devices/me/attributes", atributo_json, 0, 1, 0);
}

bool get_jerarquizado(){
    return jerarquizado;
}

void cambio_jerarquizado(){
    jerarquizado = true;
}