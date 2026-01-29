#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

#include "ipc.h"
#include "inc/operations.h"
#include "types.h"

int32_t get_command(FilesystemOperation *command);
int32_t give_response(int32_t type_command, int32_t arg_1, int32_t arg_2, int32_t arg_3, int32_t fd);
int32_t update_coord_state(int32_t type_server, int32_t type_command, int32_t current_client_pid, int32_t fd);
#endif