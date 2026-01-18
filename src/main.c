#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include "../include/cJSON.h"
#include "../include/radio_helper.h"
#include "../include/lcd_helper.h"
#include "../include/encoder_helper.h"
#include "../include/app_state.h"
#include "../include/mqtt_client_helper.h"


#define MAX_COUNTRIES 11
#define PADDING_X   5
#define PADDING_Y   5
#define WRAP_INDEX(idx, delta, max) \
    ((((idx) + (delta)) % (max) + (max)) % (max))

#define BACKGROUND_COLOR    BLACK
#define TEXT_COLOR          CYAN
#define SELECTED_TEXT_COLOR RED
#define MAX_STATIONS   21
#define STATION_OFFSET  0
#define MAX_VISIBLE_STATION_COUNT 13

#define CLIENTID        "RadioReporter"
#define TOPIC_STATUS    "radio/status"
#define TOPIC_REQUEST   "radio/request"

typedef void (*draw_func_t)(AppState *);

int wiringPiInitialized = -1;
MQTTClient client = NULL;
AppState app_state = {0, 0, ZERO, "", "", ""};

typedef struct {
    Station station;
    char country[15];
} Message;


char mqtt_json_msg[512];
Message mqtt_msg = { {"", ""}, "" };
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
const char *countries[MAX_COUNTRIES] = {
        "Turn Off", "Georgia", "Netherlands", "Germany", "Russia", "Italy",
        "Spain", "Australia", "Japan", "Canada", "China"
    };
Station stations[MAX_STATIONS];
bool app_state_changed = false;

void parse_json_payload(const char *payload, Message *message) {
    char buffer[512];
    size_t len = strlen(payload);
    if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
    memcpy(buffer, payload, len);
    buffer[len] = '\0';
    cJSON *json = cJSON_Parse(buffer);
    if (!json) {
        printf("Failed to parse JSON\n");
        return;
    }

    cJSON *country_item = cJSON_GetObjectItem(json, "country");
    cJSON *station_item = cJSON_GetObjectItem(json, "station");
    cJSON *url_item = cJSON_GetObjectItem(json, "url");

    
    if (cJSON_IsString(country_item) && cJSON_IsString(station_item) && cJSON_IsString(url_item)) {
        Message msg = { {"name", "url"}, "ML" };
        strncpy(msg.country, country_item->valuestring, sizeof(msg.country)-1);
        strncpy(msg.station.name, station_item->valuestring, sizeof(msg.station.name)-1);
        strncpy(msg.station.url, url_item->valuestring, sizeof(msg.station.url)-1);
        *message = msg;
    }
}

void on_radio_request(const char *topic, const char *payload) {
    Message msg;
    parse_json_payload(payload, &msg);

    pthread_mutex_lock(&state_mutex);
    sprintf(app_state.country, msg.country);
    sprintf(app_state.station_name, msg.station.name);
    sprintf(app_state.station_url, msg.station.url);
    app_state.state = STREAM_PLAYING;
    app_state_changed = true;
    pthread_mutex_unlock(&state_mutex);
    
    printf("[MQTT receiver] Got message on %s: station: %s, url: %s, country: %s\n", topic, msg.station.name, msg.station.url, msg.country);
}

void get_json_str(Message msg, char *json) {
    snprintf(json, 512,
        "{ \"station\": { \"name\": \"%s\", \"url\": \"%s\" }, \"country\": \"%s\" }",
        msg.station.name, msg.station.url, msg.country);

}

void send_mqtt_msg_wrapper() {
    get_json_str(mqtt_msg, mqtt_json_msg);
    send_msg(client, mqtt_json_msg, TOPIC_STATUS);
}

void handle_signal(int sig) {
    printf("Received signal %d, cleaning up...\n", sig);
    set_background(BLACK);
    stop_station(1);
    disconnect_client(client);
    exit(0);
}

void draw_countries(AppState *state) {
    draw_text(PADDING_X, 5, "Select a country:", WHITE, 0);
    for(int i = 0; i < MAX_COUNTRIES; i++) {
        int is_selected = i == state->selected_country;
        int y_location = 20 + i * LINE_HEIGHT;
        if(is_selected) {
            draw_rectangle(PADDING_X, y_location, LCD_WIDTH, LINE_HEIGHT, BACKGROUND_COLOR);
        }
        char buffer[200];
        sprintf(buffer, "%d: %s", i, countries[i]);
        draw_text(PADDING_X, y_location,
            buffer, is_selected ? SELECTED_TEXT_COLOR : TEXT_COLOR, 0);
    }
}
int last_selected_station = 0;

void draw_stations(AppState *state) {
    int start_station = state->selected_station - MAX_VISIBLE_STATION_COUNT;
    start_station = start_station < 0 ? 0 : start_station;
    if(state->selected_station > MAX_VISIBLE_STATION_COUNT || last_selected_station > MAX_VISIBLE_STATION_COUNT) {
        set_background(BACKGROUND_COLOR);
    }
    draw_text(PADDING_X, 5, countries[state->selected_country], WHITE, 0);
    for(int i = 0; i < MAX_STATIONS; i++) {
        int is_selected = i+start_station == state->selected_station;
        int y_location = 20 + i * LINE_HEIGHT;
        if(is_selected) {
            draw_rectangle(PADDING_X, y_location, LCD_WIDTH, LINE_HEIGHT, BACKGROUND_COLOR);
        }

        if(i+start_station >= MAX_STATIONS) break;
        char buffer[200];
        sprintf(buffer, "%d: %s", i+start_station, stations[i+start_station].name);
        draw_text(PADDING_X, y_location,
            buffer, is_selected ? SELECTED_TEXT_COLOR : TEXT_COLOR, 0);
        
    }
    last_selected_station = state->selected_station;
}


int encoder_selection_loop(int *selected_index, int max_items, draw_func_t draw_func, AppState *state, bool draw_bg) {
    if(draw_bg) set_background(BACKGROUND_COLOR);
    if (draw_func) { draw_func(state); }

    while(1) {
        if(app_state_changed) {
            pthread_mutex_lock(&state_mutex);
            app_state_changed = false;
            pthread_mutex_unlock(&state_mutex);
            break;
        }
        
        encoder_value_t val = read_encoder();
    
        if(val.rotation != 0 && draw_func) {
            *selected_index = WRAP_INDEX(*selected_index, val.rotation, max_items);
            draw_func(state);
        }

        if(val.btn == 1) {
            return 1;
        }
        encoder_delay(5);
    }

    return 0;
}

int country_selection(AppState *state) {
    int res = encoder_selection_loop(&state->selected_country, MAX_COUNTRIES, draw_countries, state, true);
    if(res) {
        sprintf(mqtt_msg.country, countries[state->selected_country]);
        sprintf(state->country, countries[state->selected_country]);
        send_mqtt_msg_wrapper();
    }
    
    return res;
}

int station_selection(AppState *state) {
    set_background(BACKGROUND_COLOR);
    int size = get_stations_by_country(countries[state->selected_country], MAX_STATIONS-1, STATION_OFFSET, stations+1);
    if(size > 0) {
        draw_stations(state);
    }
    int res = encoder_selection_loop(&state->selected_station, MAX_STATIONS, draw_stations, state, false);

    if(res) {
        Station station = stations[state->selected_station];
        mqtt_msg.station = station;
        sprintf(state->station_name, stations[state->selected_station].name);
        sprintf(state->station_url, stations[state->selected_station].url);
        send_mqtt_msg_wrapper();
    }
    
    
    return res;
}

bool draw_station_info = true;

int stream_playing_control(AppState *state) {
    if(draw_station_info) {
        set_background(BACKGROUND_COLOR);
        char buffer[200];
        sprintf(buffer, "Now playing: %s... ", state->station_name);
        draw_text(PADDING_X, PADDING_Y, buffer, TEXT_COLOR, 1);
        draw_text(PADDING_X, PADDING_Y+60, "Press the button to go back.", TEXT_COLOR, 1);
        draw_station_info = false;
    }
    
    return encoder_selection_loop(NULL, 0, NULL, NULL, false);
}

int main() {
    
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);

    client = initialize_mqtt_client(on_radio_request, TOPIC_REQUEST, CLIENTID);
    if (!client) {
        printf("failed to init mqtt client\n");
        return -1;
    }

    lcd_ptr_t lcd_settings = lcd_init();
    int encoder_init_status = encoder_init();

    if(lcd_settings == NULL || encoder_init_status != 0) {
        printf("Initialization failed\n");
        return -1;
    }
    
    app_state.state = INIT;

    stations[0] = (Station){"Back", ""};

    int country_selection_status = -1;
    int station_selection_status = -1;

    int running = 1;

    while(running) {

        pthread_mutex_lock(&state_mutex);

        AppState current_state = app_state;

        pthread_mutex_unlock(&state_mutex);
        
        switch (current_state.state)
        {
            case INIT:
                current_state.state = COUNTRY_SELECTION;
                break;
            case COUNTRY_SELECTION:
                country_selection_status = country_selection(&current_state);
                if(country_selection_status) {
                    if(current_state.selected_country == 0) { // turn off
                        set_background(BLACK);
                        stop_station(0);
                        disconnect_client(client);
                        running = 0;
                    }
                    else {
                        current_state.state = STATION_SELECTION;
                    }
                    
                }
                else {
                    continue;
                }
                break;
            case STATION_SELECTION:
                station_selection_status = station_selection(&current_state);
                if(station_selection_status) {
                    if(current_state.selected_station == 0) { // if we select "back" we go back to country selection
                        current_state.state = COUNTRY_SELECTION;
                    }
                    else {
                        printf("Playing %s\n", current_state.station_name);
                        current_state.state = STREAM_PLAYING;
                    }
                }
                else {
                    continue;
                }
                break;
            case STREAM_PLAYING:
                play_station(current_state.station_url);
                draw_station_info = true;
                if(stream_playing_control(&current_state)) {
                    current_state.state = STATION_SELECTION;
                }
                else {
                    continue;
                }
                break;
            default:
                current_state.state = INIT;
                break;
        }
        app_state = current_state;
    }
    lcd_cleanup(lcd_settings);
    
    return 0;
}