#include <stdio.h>
#include <string.h>
#include "../include/mqtt_client_helper.h"

static message_handler_t user_handler = NULL;

static int internal_message_callback(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    if (user_handler) {
        user_handler(topicName, (char*)message->payload);
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

MQTTClient initialize_mqtt_client(message_handler_t handler, const char *topic_request, const char *clientid) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    user_handler = handler;

    MQTTClient_create(&client, ADDRESS, clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, NULL, internal_message_callback, NULL);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return NULL;
    }

    if((rc = MQTTClient_subscribe(client, topic_request, QOS)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to subscribe\n");
        return NULL;
    }

    return client;
}

bool send_msg(MQTTClient client, char* msg, const char *topic_status) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = msg;
    pubmsg.payloadlen = (int)strlen(msg);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic_status, &pubmsg, &token);
    printf("[MQTT sender] Publishing message: %s\n", msg);
    MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("[MQTT sender] Message delivered\n");

    return true;

}

bool disconnect_client(MQTTClient client) {
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return true;
}
