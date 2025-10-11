#include "../include/mqtt_client_helper.h"
#include "../include/radio_helper.h"

typedef struct {
    Station station;
    char country[3];
} Message;

void get_json_str(Message msg, char *json) {
    sprintf(json,
        "{ \"station\": { \"name\": \"%s\", \"url\": \"%s\" }, \"country\": \"%s\" }",
        msg.station.name, msg.station.url, msg.country);
}

int main() {
    MQTTClient client = initialize_mqtt_client();

    Message msg = { {"TRAKI", "http://radiox.example.com"}, "ML" };
    char json[512];
    
    get_json_str(msg, json);

    send_msg(client, json);

    disconnect_client(client);
}