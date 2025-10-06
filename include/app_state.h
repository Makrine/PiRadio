#ifndef APP_STATE_H
#define APP_STATE_H

enum State {
    ZERO,
    INIT,
    COUNTRY_SELECTION,
    STATION_SELECTION,
    STREAM_PLAYING,
    ERROR
};

typedef struct {
    int selected_station;
    int selected_country;
    enum State state;
} AppState;

#endif
