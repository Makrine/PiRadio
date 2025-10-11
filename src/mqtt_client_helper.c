#include <stdio.h>
#include <string.h>
#include "../include/mqtt_client_helper.h"


MQTTClient initialize_mqtt_client() {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return NULL;
    }

    return client;
}

bool send_msg(MQTTClient client, char* msg) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = msg;
    pubmsg.payloadlen = (int)strlen(msg);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    printf("Publishing message: %s\n", msg);
    MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message delivered\n");

    return true;

}

bool disconnect_client(MQTTClient client) {
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return true;
}
