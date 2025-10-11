#ifndef MQTT_CLIENT_HELPER__H
#define MQTT_CLIENT_HELPER__H

#include "MQTTClient.h"
#include <stdbool.h>

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "RadioReporter"
#define TOPIC       "radio/status"
#define QOS         1
#define TIMEOUT     10000L


MQTTClient initialize_mqtt_client();
bool send_msg(MQTTClient client, char* msg);
bool disconnect_client(MQTTClient client);

#endif // MQTT_CLIENT_HELPER__H