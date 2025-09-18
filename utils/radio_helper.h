#ifndef radio_helper__h
#define radio_helper__h

struct Station {
    char name[100];
    char url[200];
};

char* get_curl_output(const char* cmd);
int get_stations_by_country(const char *country, const int limit, const int offset, struct Station *stations);
void play_station(const char *url);
void stop_station();
#endif
