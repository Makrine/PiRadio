#ifndef RADIO_HELPER__H
#define RADIO_HELPER__H

typedef struct {
    char name[100];
    char url[200];
} Station;

char* get_curl_output(const char* cmd);
int get_stations_by_country(const char *country, const int limit, const int offset, Station *stations);
void play_station(const char *url);
void stop_station(int force);

#endif // RADIO_HELPER__H