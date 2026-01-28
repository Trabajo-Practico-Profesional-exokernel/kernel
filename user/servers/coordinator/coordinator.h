#ifndef COORDINATOR
#define COORDINATOR

#include "inc/operations.h"
#include "types.h"
#include "arch/proc.h"
#include "server_map.h"

typedef struct {
    ServerMap server_map; 
} Coordinator;

void init_coordinator();
void init_servers();
extern Coordinator coordinator;

#endif
