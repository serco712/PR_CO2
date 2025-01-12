#ifndef MQTT_SENSOR_H
#define MQTT_SENSOR_H

#include "mqtt_client.h"

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

static void mqtt_app_start();

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

#endif // MQTT_SENSOR_H