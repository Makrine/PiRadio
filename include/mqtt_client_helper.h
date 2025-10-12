#ifndef MQTT_CLIENT_HELPER__H
#define MQTT_CLIENT_HELPER__H

#include "MQTTClient.h"
#include <stdbool.h>

#define ADDRESS         "tcp://localhost:1883"
#define QOS             1
#define TIMEOUT         10000L

typedef void (*message_handler_t)(const char *topic, const char *payload);


MQTTClient initialize_mqtt_client(message_handler_t message_received, const char *topic_request, const char *clientid);
bool send_msg(MQTTClient client, char* msg, const char *topic_status);
bool disconnect_client(MQTTClient client);

#endif // MQTT_CLIENT_HELPER__H