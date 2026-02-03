#ifndef SERVER_MAP_H
#define SERVER_MAP_H

#include "inc/operations.h"
#include "types.h"

typedef struct {
    uint32_t pids[SERVER_COUNT];
} ServerMap;

void server_map_init(ServerMap *map);
void server_map_set(ServerMap *map, Server type, uint32_t pid);
int32_t server_map_get(ServerMap *map, Server type);

#endif