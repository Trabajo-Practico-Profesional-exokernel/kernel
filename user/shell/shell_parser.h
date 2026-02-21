#ifndef SHELL_PARSER_H
#define SHELL_PARSER_H

#include "types.h"
#define MAX_PIPELINE_LEN 10
#define MAX_ARGS_PER_CMD 32
#define MAX_TOKENS 128

// Redirection types
typedef enum {
    REDIR_NONE = 0,
    REDIR_INPUT,      // <
    REDIR_OUTPUT,     // >
    REDIR_APPEND,     // >>
    REDIR_ERROR,      // 2>
    REDIR_ERROR_APPEND // 2>>
} redir_type_t;

// Single redirection entry
typedef struct {
    redir_type_t type;
    char path[256];    // File path for redirection
} redirection_t;

// Single command with its arguments and redirections
typedef struct {
    char *cmd;         // Command name (first word)
    char **argv;       // Arguments array (including command at argv[0])
    int argc;          // Argument count
    redirection_t stdin_redir;   // Input redirection
    redirection_t stdout_redir;  // Output redirection
    redirection_t stderr_redir;  // Error redirection
} command_t;

// Pipeline: a series of commands connected by pipes
typedef struct {
    command_t *commands;  // Array of commands
    int num_commands;     // Number of commands in pipeline
    char **tokens_to_free;  // Tokens allocated during parsing (for cleanup)
    int token_count;      // Number of tokens
} pipeline_t;

// Parser result
typedef struct {
    pipeline_t pipeline;
    int parse_error;      // 0 = success, non-zero = error code
    char error_msg[128];  // Error description
} parse_result_t;

// Parser interface
parse_result_t parse_command_line(char *input);
void free_parse_result(parse_result_t *result);
void print_parse_result(parse_result_t *result); // For debugging

#endif // SHELL_PARSER_H
