#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include "../include/radio_helper.h"
#include "../include/lcd_helper.h"
#include "../include/encoder_helper.h"
#include "../include/app_state.h"
#include "../include/mqtt_client_helper.h"

#define CLIENTID        "RadioReporter"
#define TOPIC_STATUS    "radio/status"
#define TOPIC_REQUEST   "radio/request"
#define BACKGROUND_COLOR    BLACK
#define TEXT_COLOR          CYAN
#define SELECTED_TEXT_COLOR RED

#define MAX_STATIONS   21
#define STATION_OFFSET  0
#define MAX_VISIBLE_STATION_COUNT 13

#define WRAP_INDEX(idx, delta, max) \
    ((((idx) + (delta)) % (max) + (max)) % (max))
typedef struct {
    Station station;
    char country[15];
} Message;
AppState app_state = {0, 0, ZERO};
Station stations[20];
int wiringPiInitialized = -1;
MQTTClient client = NULL;
char mqtt_json_msg[512];
Message mqtt_msg = { {"name", "url"}, "ML" };

typedef void (*draw_func_t)(void);
const char *countries[10] = {
        "Turn Off", "Georgia", "Netherlands", "Germany", "Russia", "Italy",
        "Spain", "Australia", "Japan", "Canada"
    };



void on_radio_request(const char *topic, const char *payload) {
    printf("Got message on %s: %s\n", topic, payload);
}


void get_json_str(Message msg, char *json) {
    sprintf(json,
        "{ \"station\": { \"name\": \"%s\", \"url\": \"%s\" }, \"country\": \"%s\" }",
        msg.station.name, msg.station.url, msg.country);
}
void draw_countries() {
    draw_text(5, 5, "Select a country:", WHITE, 0);
    for(int i = 0; i < 10; i++) {
        int is_selected = i == app_state.selected_country;
        int y_location = 20 + i * LINE_HEIGHT;
        if(is_selected) {
            draw_rectangle(5, y_location, LCD_WIDTH, LINE_HEIGHT, BACKGROUND_COLOR);
        }
        char buffer[200];
        sprintf(buffer, "%d: %s", i, countries[i]);
        draw_text(5, y_location,
            buffer, is_selected ? SELECTED_TEXT_COLOR : TEXT_COLOR, 0);
    }
}
void send_mqtt_msg_wrapper() {
    get_json_str(mqtt_msg, mqtt_json_msg);
    send_msg(client, mqtt_json_msg, TOPIC_STATUS);
}
int encoder_selection_loop(int *selected_index, int max_items, draw_func_t draw_func, bool draw_bg) {
    if(draw_bg) set_background(BACKGROUND_COLOR);
    if (draw_func) { draw_func(); }

    while(1) {
        encoder_value_t val = read_encoder();
    
        if(val.rotation != 0 && draw_func) {
            *selected_index = WRAP_INDEX(*selected_index, val.rotation, max_items);
            draw_func();
        }

        if(val.btn == 1) {
            return 1;
        }
        encoder_delay(5);
    }

    return 0;
}
int last_selected_station = 0;
void draw_stations() {
    int start_station = app_state.selected_station - MAX_VISIBLE_STATION_COUNT;
    start_station = start_station < 0 ? 0 : start_station;
    if(app_state.selected_station > MAX_VISIBLE_STATION_COUNT || last_selected_station > MAX_VISIBLE_STATION_COUNT) {
        set_background(BACKGROUND_COLOR);
    }
    draw_text(5, 5, countries[app_state.selected_country], WHITE, 0);
    for(int i = 0; i < MAX_STATIONS; i++) {
        int is_selected = i+start_station == app_state.selected_station;
        int y_location = 20 + i * LINE_HEIGHT;
        if(is_selected) {
            draw_rectangle(5, y_location, LCD_WIDTH, LINE_HEIGHT, BACKGROUND_COLOR);
        }

        if(i+start_station >= MAX_STATIONS) break;
        char buffer[200];
        sprintf(buffer, "%d: %s", i+start_station, stations[i+start_station].name);
        draw_text(5, y_location,
            buffer, is_selected ? SELECTED_TEXT_COLOR : TEXT_COLOR, 0);
        
    }
    last_selected_station = app_state.selected_station;
}
int station_selection() {
    set_background(BACKGROUND_COLOR);
    int size = get_stations_by_country(countries[app_state.selected_country], MAX_STATIONS, STATION_OFFSET, stations+1);
    printf("size: %d\n", size);
    if(size > 0) {
        draw_stations();
    }
    int res = encoder_selection_loop(&app_state.selected_station, MAX_STATIONS, draw_stations, false);
    Station station = stations[app_state.selected_station];
    mqtt_msg.station = station;
    
    send_mqtt_msg_wrapper();
    
    return res;
}
int country_selection() {
    int res = encoder_selection_loop(&app_state.selected_country, 10, draw_countries, true);
    sprintf(mqtt_msg.country, countries[app_state.selected_country]);

    send_mqtt_msg_wrapper();
    return res;
}
int main() {
    client = initialize_mqtt_client(on_radio_request, TOPIC_REQUEST, CLIENTID);
    if (!client) return 1;

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
        switch (app_state.state)
        {
            case INIT:
                app_state.state = COUNTRY_SELECTION;
                break;
            case COUNTRY_SELECTION:
                country_selection_status = country_selection();
                if(country_selection_status) {
                    if(app_state.selected_country == 0) { // turn off
                        set_background(BLACK);
                        stop_station(0);
                        disconnect_client(client);
                        running = 0;
                    }
                    else {
                        app_state.state = STATION_SELECTION;
                    }
                    
                }
                break;
            case STATION_SELECTION:
                station_selection_status = station_selection();
                if(station_selection_status) {
                    if(app_state.selected_station == 0) { // if we select "back" we go back to country selection
                        app_state.state = COUNTRY_SELECTION;
                    }
                    else {
                        printf("Playing %s\n", stations[app_state.selected_station].name);
                        play_station(stations[app_state.selected_station].url);
                        app_state.state = STREAM_PLAYING;
                    }
                }
                break;

            default:
                app_state.state = INIT;
                break;
        }
    }
    lcd_cleanup(lcd_settings);
    

    disconnect_client(client);
}