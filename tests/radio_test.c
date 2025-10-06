#include "../include/radio_helper.h"
#include <stdio.h>
#include <unistd.h>

#define LIMIT   20
#define OFFSET  0
#define COUNTRY "Georgia"
#define SELECTION 5
#define DELAY   5

int main() {
    Station stations[20];
    
    int size = get_stations_by_country(COUNTRY, MAX_STATIONS, STATION_OFFSET, stations);
    printf("SIZE: %d | Name: %s | Url: %s\n", size, stations[SELECTION].name, stations[SELECTION].url);
    if(size == MAX_STATIONS)
        play_station(stations[SELECTION].url);
    
    sleep(DELAY);
    stop_station();
    return 0;
}
