#ifndef IPC_MSGS_H
#define IPC_MSGS_H

#include <stdint.h>

#define MAX_CONTENT_SIZE 128

typedef struct {
    int32_t server_id;
    int32_t type_msg;
    int32_t arg_1;
    int32_t arg_2;
    uint8_t content[MAX_CONTENT_SIZE];
} AppMsg;

typedef struct {
    int32_t app_id;
    int32_t type_msg;
    int32_t arg_1;
} CoordinatorMsg;

typedef struct {
    int32_t app_id;
    int32_t type_msg;
    int32_t arg_1;
    int32_t arg_2;
    int32_t arg_3;
    uint8_t content[MAX_CONTENT_SIZE];
} FilesystemMsg;


typedef struct {
    int32_t app_id;
    int32_t type_msg;
    int32_t pipe_id;
    int32_t size;
    uint8_t content[MAX_CONTENT_SIZE];
} PipeMsg;


typedef struct {
    int32_t app_id;
    int32_t type_msg;
    int32_t color;
    uint8_t content[MAX_CONTENT_SIZE];
} ConsoleMsg;


typedef union {
    CoordinatorMsg coord;
    FilesystemMsg  fs;
    PipeMsg        pipe;
    ConsoleMsg     console;
    uint8_t        raw[sizeof(FilesystemMsg)]; // el msj mas grande define tamaño
} ServiceMsgUnion;

#endif