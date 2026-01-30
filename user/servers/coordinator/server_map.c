#include "server_map.h"

void server_map_init(ServerMap *map) {
    for (int i = 0; i < SERVER_COUNT; i++) {
        map->pids[i] = 0;
    }
}

void server_map_set(ServerMap *map, Server type, uint32_t pid) {
    if (type >= 0 && type < SERVER_COUNT) {
        map->pids[type] = pid;
    }
}

uint32_t server_map_get(ServerMap *map, Server type) {
    if (type >= 0 && type < SERVER_COUNT) {
        return map->pids[type];
    }
    return -1;
}
