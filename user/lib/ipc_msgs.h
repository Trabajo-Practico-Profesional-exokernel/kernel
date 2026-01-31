#ifndef IPC_MSGS_H
#define IPC_MSGS_H

// #include <stdint.h>
#include "types.h"

#define MAX_CONTENT_SIZE 128

typedef struct{
    int32_t sender_pid;
    int32_t arg_1;
    int32_t arg_2;
    int32_t arg_3;
    int32_t arg_4;
    int32_t arg_5;
    int32_t arg_6;
} Msg;

typedef struct {
    int32_t app_id;
    int32_t server_type;

    int32_t type_op;
    int32_t fd;
    int32_t type_command;
    int32_t state;
} CoordinatorOperation;

typedef struct {
    int32_t app_id;
    int32_t type_op;

    int32_t arg_1;
    int32_t arg_2;

    int32_t fd;

    int32_t content_1_vaddr;
    int32_t len_content_1;

    int32_t content_2_vaddr;
    int32_t len_content_2;
} FilesystemOperation;

typedef struct {
    int32_t app_id;
    int32_t type_op;

    int32_t fd;

    int32_t content_vaddr;
    int32_t len_content;
} PipeOperation;

typedef struct {
    int32_t app_id;
    int32_t type_op;

    int32_t arg_1;
    int32_t arg_2;

    int32_t content_vaddr;
    int32_t len_content;

} ConsoleOperation;

#endif
