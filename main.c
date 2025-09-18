#include <stdio.h>
#include "utils/radio_helper.h"
#include "utils/lcd_helper.h"
#include "utils/encoder_helper.h"
#include "utils/st7735s.h"
#include <stdlib.h>

#define OFFSET 5
#define LIMIT 15
#define MAX_COUNTRIES 9

int selected_station = 0;
int selected_country = 0;
int btn_pressed = 0;
const char* country;
const char *countries[] = {
    "Georgia",
    "Netherlands",
    "Germany",
    "France",
    "Italy",
    "Spain",
    "Australia",
    "Japan",
    "Canada"
};

int print_stations(struct Station *stations, const char *country) {
    if(btn_pressed) {set_background_color(0, 0, 0);}
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Stations in %s:", country);
    draw_text(5, 5, buffer, CYAN);
    snprintf(buffer, sizeof(buffer), "-------------------------");
    draw_text(5, 10, buffer, CYAN);
    for(int i = 0; i < LIMIT; i++) {
        //printf("%s %s\n", stations[i].name, stations[i].url);
        snprintf(buffer, sizeof(buffer), "%d: %s", i, stations[i].name);
        if(i == selected_station) {
            draw_text(5, 20 + i * 10, buffer, RED);

        }
        else draw_text(5, 20 + i * 10, buffer, CYAN);
    }
    return 0;
}

int print_countries(const char *countries[], int count) {
    for(int i = 0; i < count; i++) {
        if(i == selected_country)
            draw_text(5, 20 + i * 10, countries[i], RED);
        else
            draw_text(5, 20 + i * 10, countries[i], CYAN);
    }
    return 0;
}

// int main(void) {
    
//     lcd_ptr_t my_lcd_settings = setup();

//     if(set_background_color(0, 255, 255) < LCD_OK) {
//         fail_handler();
//     }
//     // if(draw_text(10, 10, "hello niggaz 123 ! Glory TO Mackiland YOO", RED)    < LCD_OK) {
//     //     fail_handler();
//     // }
//     //animate_text("niggazzz");
//     lcd_cleanup(my_lcd_settings);

//     return 0;
// }

const char* country_selection() {
    draw_text(5, 5, "Select a country:", WHITE);
    print_countries(countries, MAX_COUNTRIES);

    while(1) {
        int btn = 0;
        int change = read_encoder(&btn);
        if(change != 0) {
            if(change == 1 && selected_country < MAX_COUNTRIES - 1) {
                selected_country++;
            } else if(change == -1 && selected_country > 0) {
                selected_country--;
            }
            print_countries(countries, 9);
            btn_pressed = 0;
        }
        if(btn && btn != btn_pressed) {
            btn_pressed = 1;
            set_background_color(0, 0, 0);
            return countries[selected_country];
        }
        encoder_delay(5);
    }

}

void station_selection(struct Station *stations, const char *country) {
    print_stations(stations, country);

    while(1) {
        int btn = 0;
        int change = read_encoder(&btn);
        if(change != 0) {
            if(change == 1 && selected_station < LIMIT - 1) {
                selected_station++;
            } else if(change == -1 && selected_station > 0) {
                selected_station--;
            }
            print_stations(stations, country);
            btn_pressed = 0;
        }
        if(btn && btn != btn_pressed) {
            btn_pressed = 1;
            if(selected_station == 0) {
                // Go back to country selection
                selected_country = 0;
                selected_station = 0;
                set_background_color(0, 0, 0);
                country = country_selection();
                get_stations_by_country(country, LIMIT, OFFSET, stations+1);
                print_stations(stations, country);
                continue;
            }
            set_background_color(0, 0, 0);
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "Selected station:");
            draw_text(5, 5, buffer, CYAN);
            snprintf(buffer, sizeof(buffer), "%s", stations[selected_station].name);
            draw_text(5, 20, buffer, CYAN);
            play_station(stations[selected_station].url);
            //printf("Selected station: %s\n", stations[selected_station].name);
            // Here you would add code to play the selected station
        }
        encoder_delay(5);
    }
}

int main() {
    lcd_ptr_t my_lcd_settings = setup();
    setup_encoder();
    if(set_background_color(0, 0, 0) < LCD_OK) {
        fail_handler();
    }

    country = country_selection();

    struct Station stations[100];
    stations[0] = (struct Station){"Back", ""};
    get_stations_by_country(country, LIMIT, OFFSET, stations+1);
    station_selection(stations, country);
    
    lcd_cleanup(my_lcd_settings);
    return 0;
}