#ifndef MQTT_SENSOR_H
#define MQTT_SENSOR_H

#include "mqtt_client.h"
#include "cJSON.h"


ESP_EVENT_DECLARE_BASE(MQTT_COMP_EVENTS);
enum
{
    MQTT_COMP_CONNECTED,
    MQTT_COMP_OTA
};




/**
 * @brief Inicializa el cliente MQTT.
 *
 * @param broker_uri URI del broker MQTT.
 * @param username Nombre de usuario para el broker.
 * @param password Contraseña para el broker.
 * @param client_id ID del cliente MQTT.
 */
void mqtt_sensor_init(const char *broker_uri, const char *username, const char *password, const char *client_id);

/**
 * @brief Publica un mensaje en un tópico.
 *
 * @param topic Tópico donde se publicará el mensaje.
 * @param data Mensaje a enviar.
 */
void mqtt_sensor_publish(const char *topic, const char *data);

/**
 * @brief Habilita la publicación del sensor.
 */
void mqtt_sensor_enable();

/**
 * @brief Deshabilita la publicación del sensor.
 */
void mqtt_sensor_disable();

void pub_task();

void mqtt_app_start(char *json_data, esp_event_loop_handle_t loop);

void mqtt_app_start_not_prov(cJSON *data);

void mqtt_app_start_prov();

//static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

void reconnect_mqtt_not_prov();

void reconnect_mqtt_prov();

void send_data(char* data);

void subscribe(char* topic);

esp_mqtt_client_handle_t unregister_mqtt_handler();
static void mqtt_event_handler_not_prov(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

#endif // MQTT_SENSOR_H