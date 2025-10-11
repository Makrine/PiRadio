#include "../include/radio_helper.h"
#include "../include/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 65536

pid_t vlc_pid = -1;

char* get_curl_output(const char* cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) return NULL;

    char *buffer = malloc(BUF_SIZE);
    if (!buffer) {
        pclose(fp);
        return NULL;
    }

    size_t len = 0;
    int c;
    while ((c = fgetc(fp)) != EOF && len < BUF_SIZE - 1) {
        buffer[len++] = (char)c;
    }
    buffer[len] = '\0';
    pclose(fp);
    return buffer;
}

int get_stations_by_country(const char *country, const int limit, const int offset, Station *stations) {
    char cmd[200];
    snprintf(cmd, sizeof(cmd),
        "curl -s \"http://all.api.radio-browser.info/json/stations/bycountry/%s?limit=%d&offset=%d\"", country, limit, offset);
    char *json_data = get_curl_output(cmd);
    if (!json_data) {
        printf("Failed to get data from curl\n");
        return -1;
    }

    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        printf("Error parsing JSON\n");
        free(json_data);
        return -1;
    }

    
    int size = cJSON_GetArraySize(root);
    for (int i = 0; i < size; i++) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        cJSON *name = cJSON_GetObjectItem(item, "name");
        cJSON *url = cJSON_GetObjectItem(item, "url_resolved");
        if (name && cJSON_IsString(name) && url && cJSON_IsString(url)) {
            strncpy(stations[i].name, name->valuestring, sizeof(stations[i].name) - 1);
            stations[i].name[sizeof(stations[i].name) - 1] = '\0';
            strncpy(stations[i].url, url->valuestring, sizeof(stations[i].url) - 1);
            stations[i].url[sizeof(stations[i].url) - 1] = '\0';
        } else {
            stations[i].name[0] = '\0';
            stations[i].url[0] = '\0';
        }
    }

    cJSON_Delete(root);
    free(json_data);
    return size;
}

void play_station(const char *url) {
    if(vlc_pid > 0) {
        kill(vlc_pid, SIGKILL);  // stop previous VLC
    }

    vlc_pid = fork();
    if(vlc_pid == 0) {
        // child process
        execlp("cvlc", "cvlc", "--intf", "dummy", "--aout=alsa", "--play-and-exit", url, NULL);
        _exit(1); // if exec fails
    }
    // parent continues
}

void stop_station(int force) {
    if(force) {
        execlp("pkill", "-9", "vlc", NULL);
        return;
    }
    if(vlc_pid > 0) {
        kill(vlc_pid, SIGKILL);
        vlc_pid = -1;
    }
    else {
        execlp("pkill", "-9", "vlc", NULL);
    }
    
}