#include <stdio.h>
#include "../include/radio_helper.h"
#include "../include/lcd_helper.h"
#include "../include/encoder_helper.h"
#include "../include/app_state.h"

#define MAX_COUNTRIES 9
#define PADDING_X   5
#define WRAP_INDEX(idx, delta, max) \
    ((((idx) + (delta)) % (max) + (max)) % (max))

#define BACKGROUND_COLOR    BLACK
#define MAX_STATIONS   20
#define STATION_OFFSET  0

int wiringPiInitialized = -1;

AppState app_state = {0, 0, ZERO};

const char *countries[MAX_COUNTRIES] = {
        "Georgia", "Netherlands", "Germany", "France", "Italy",
        "Spain", "Australia", "Japan", "Canada"
    };
Station stations[MAX_STATIONS];

void draw_countries() {
    draw_text(PADDING_X, 5, "Select a country:", WHITE, 0);
    for(int i = 0; i < MAX_COUNTRIES; i++) {
        int is_selected = i == app_state.selected_country;
        int y_location = 20 + i * LINE_HEIGHT;
        if(is_selected) {
            draw_rectangle(PADDING_X, y_location, LCD_WIDTH, LINE_HEIGHT, BLACK);
        }
        char buffer[200];
        sprintf(buffer, "%d: %s", i, countries[i]);
        draw_text(PADDING_X, y_location,
            buffer, is_selected ? RED : CYAN, 0);
    }
}

void draw_stations() {
    draw_text(PADDING_X, 5, countries[app_state.selected_country], WHITE, 0);
    for(int i = 0; i < MAX_STATIONS; i++) {
        int is_selected = i == app_state.selected_station;
        int y_location = 20 + i * LINE_HEIGHT;
        if(is_selected) {
            draw_rectangle(PADDING_X, y_location, LCD_WIDTH, LINE_HEIGHT, BLACK);
        }
        if(i == 0) {
            draw_text(PADDING_X, y_location,
            "Back", is_selected ? RED : CYAN, 0);
        }
        else {
            char buffer[200];
            sprintf(buffer, "%d: %s", i, stations[i].name);
            draw_text(PADDING_X, y_location,
                buffer, is_selected ? RED : CYAN, 0);
        }
        
    }
}


int country_selection() {
    app_state.state = COUNTRY_SELECTION;

    set_background(BACKGROUND_COLOR);
    draw_countries();

    while(1) {
        encoder_value_t val = read_encoder();
    
        if(val.rotation != 0) {
            app_state.selected_country = WRAP_INDEX(app_state.selected_country, val.rotation, MAX_COUNTRIES);
            draw_countries();
        }

        if(val.btn == 1) {
            return 1;
        }
        encoder_delay(5);
    }

    return 0;
}

int station_selection() {
    app_state.state = STATION_SELECTION;

    set_background(BACKGROUND_COLOR);
    int size = get_stations_by_country(countries[app_state.selected_country], MAX_STATIONS, STATION_OFFSET, stations+1);
    if(size == MAX_STATIONS) {
        draw_stations();
    }

    while(1) {
        encoder_value_t val = read_encoder();
    
        if(val.rotation != 0) {
            app_state.selected_station = WRAP_INDEX(app_state.selected_station, val.rotation, MAX_STATIONS);
            draw_stations();
        }

        if(val.btn == 1) {
            return 1;
        }
        encoder_delay(5);
    }

    return 0;
}


int main() {
    
    lcd_ptr_t lcd_settings = lcd_init();
    int encoder_init_status = encoder_init();

    if(lcd_settings == NULL || encoder_init_status != 0) {
        printf("Initialization failed\n");
        return -1;
    }

    app_state.state = INIT;

    stations[0] = (Station){"Back", ""};
  
    while(1) {
        int country_selection_status = country_selection();

        if(country_selection_status) {
            printf("%s selected!\n", countries[app_state.selected_country]);
            int station_selection_status = station_selection();

            if(station_selection_status) {
                if(app_state.selected_station == 0) continue;
                else {
                    printf("Playing %s\n", stations[app_state.selected_station].name);
                    break;
                }
            }
            
        }
    }
    
    
    lcd_cleanup(lcd_settings);
    
    return 0;
}