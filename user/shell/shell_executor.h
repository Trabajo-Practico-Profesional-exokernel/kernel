#ifndef SHELL_EXECUTOR_H
#define SHELL_EXECUTOR_H

#include "types.h"
#include "shell_parser.h"

// Execution result
typedef struct {
    int exit_code;
    int execution_error;
    char error_msg[128];
} exec_result_t;

// Execute a parsed pipeline
// Handles forking, piping, and redirections
exec_result_t execute_pipeline(pipeline_t *pipeline);

// Execute a single command without piping (simpler case)
// Used for built-in commands like cd, pwd, etc.
exec_result_t execute_single_command(command_t *cmd);

#endif // SHELL_EXECUTOR_H
