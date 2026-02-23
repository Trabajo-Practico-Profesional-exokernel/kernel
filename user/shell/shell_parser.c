#include "shell_parser.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "types.h"
#include "lib.h"


// Helper: Duplicate a string
static char *str_dup(const char *str) {
    if (!str) return NULL;
    int len = strlen((const uint8_t*)str) + 1;
    char *copy = (char *)malloc(len);
    if (copy) {
        strcpy((uint8_t*)copy, (const uint8_t*)str);
    }
    return copy;
}

// Tokenize input into tokens (split by pipes and spaces)
// NOTE: All returned tokens are allocated with malloc() and must be freed
// Empty tokens are filtered out
static int tokenize(char *input, char **tokens, int max_tokens) {
    int count = 0;
    char *copy = (char *)malloc(strlen((const uint8_t*)input) + 1);
    
    if (!copy) return -1;
    
    strcpy((uint8_t*)copy, (const uint8_t*)input);
    
    // Simple tokenization: split on spaces and handle pipes
    int i = 0;
    int token_start = 0;
    int in_token = 0;
    
    while (copy[i] != '\0' && count < max_tokens) {
        if (copy[i] == ' ' || copy[i] == '|' || copy[i] == '>' || copy[i] == '<') {
            if (in_token) {
                // End current token and duplicate it
                copy[i] = '\0';
                char *token = str_dup(&copy[token_start]);
                if (!token) {
                    free(copy);
                    return -1;
                }
                // Only add non-empty tokens
                if (strlen((const uint8_t*)token) > 0) {
                    tokens[count++] = token;
                } else {
                    free(token);  // Free empty token
                }
                in_token = 0;
            }
            
            if (copy[i] != ' ') {
                // Handle special characters - allocate new strings for operators
                if (copy[i] == '>' && copy[i+1] == '>') {
                    // >> token
                    tokens[count] = str_dup(">>");
                    if (!tokens[count]) {
                        free(copy);
                        return -1;
                    }
                    count++;
                    i++;
                } else if (copy[i] == '2' && copy[i+1] == '>') {
                    if (copy[i+2] == '>') {
                        // 2>> token
                        tokens[count] = str_dup("2>>");
                        if (!tokens[count]) {
                            free(copy);
                            return -1;
                        }
                        count++;
                        i += 2;
                    } else {
                        // 2> token
                        tokens[count] = str_dup("2>");
                        if (!tokens[count]) {
                            free(copy);
                            return -1;
                        }
                        count++;
                        i++;
                    }
                } else {
                    // Single character: |, >, <
                    char single[2] = {copy[i], '\0'};
                    tokens[count] = str_dup(single);
                    if (!tokens[count]) {
                        free(copy);
                        return -1;
                    }
                    count++;
                }
            }
        } else {
            if (!in_token) {
                token_start = i;
                in_token = 1;
            }
        }
        i++;
    }
    
    // Handle last token
    if (in_token) {
        char *token = str_dup(&copy[token_start]);
        if (!token) {
            free(copy);
            return -1;
        }
        // Only add non-empty tokens
        if (strlen((const uint8_t*)token) > 0) {
            tokens[count++] = token;
        } else {
            free(token);  // Free empty token
        }
    }
    
    free(copy);
    return count;
}

// Parse arguments for a single command (from tokens until pipe/redir)
static int parse_command_args(char **tokens, int start_idx, int end_idx, 
                              command_t *cmd) {
    int arg_idx = 0;
    
    cmd->argv = (char **)malloc(sizeof(char *) * MAX_ARGS_PER_CMD);
    if (!cmd->argv) return -1;
    
    // First token is the command
    if (start_idx >= end_idx) return -1;
    
    // Skip any empty tokens at the start
    while (start_idx < end_idx && strlen((const uint8_t*)tokens[start_idx]) == 0) {
        start_idx++;
    }
    
    if (start_idx >= end_idx) return -1;
    
    cmd->cmd = tokens[start_idx];
    cmd->argv[arg_idx++] = tokens[start_idx];
    
    int i = start_idx + 1;
    while (i < end_idx) {
        // Skip empty tokens
        if (strlen((const uint8_t*)tokens[i]) == 0) {
            i++;
            continue;
        }
        
        // Check for redirections
        if (strcmp((const uint8_t*)tokens[i], (const uint8_t*)"<") == 0) {
            i++;
            // Find the next non-empty token for the filename
            while (i < end_idx && strlen((const uint8_t*)tokens[i]) == 0) {
                i++;
            }
            if (i >= end_idx) return -1; // No file specified
            cmd->stdin_redir.type = REDIR_INPUT;
            strcpy((uint8_t*)cmd->stdin_redir.path, (const uint8_t*)tokens[i]);
            i++;
        } 
        else if (strcmp((const uint8_t*)tokens[i], (const uint8_t*)">") == 0) {
            i++;
            // Find the next non-empty token for the filename
            while (i < end_idx && strlen((const uint8_t*)tokens[i]) == 0) {
                i++;
            }
            if (i >= end_idx) return -1;
            cmd->stdout_redir.type = REDIR_OUTPUT;
            strcpy((uint8_t*)cmd->stdout_redir.path, (const uint8_t*)tokens[i]);
            i++;
        }
        else if (strcmp((const uint8_t*)tokens[i], (const uint8_t*)">>") == 0) {
            i++;
            // Find the next non-empty token for the filename
            while (i < end_idx && strlen((const uint8_t*)tokens[i]) == 0) {
                i++;
            }
            if (i >= end_idx) return -1;
            cmd->stdout_redir.type = REDIR_APPEND;
            strcpy((uint8_t*)cmd->stdout_redir.path, (const uint8_t*)tokens[i]);
            i++;
        }
        else if (strcmp((const uint8_t*)tokens[i], (const uint8_t*)"2>") == 0) {
            i++;
            // Find the next non-empty token for the filename
            while (i < end_idx && strlen((const uint8_t*)tokens[i]) == 0) {
                i++;
            }
            if (i >= end_idx) return -1;
            cmd->stderr_redir.type = REDIR_ERROR;
            strcpy((uint8_t*)cmd->stderr_redir.path, (const uint8_t*)tokens[i]);
            i++;
        }
        else if (strcmp((const uint8_t*)tokens[i], (const uint8_t*)"2>>") == 0) {
            i++;
            // Find the next non-empty token for the filename
            while (i < end_idx && strlen((const uint8_t*)tokens[i]) == 0) {
                i++;
            }
            if (i >= end_idx) return -1;
            cmd->stderr_redir.type = REDIR_ERROR_APPEND;
            strcpy((uint8_t*)cmd->stderr_redir.path, (const uint8_t*)tokens[i]);
            i++;
        }
        else {
            // Regular argument - only add if not empty
            if (strlen((const uint8_t*)tokens[i]) > 0) {
                if (arg_idx >= MAX_ARGS_PER_CMD - 1) return -1;
                cmd->argv[arg_idx++] = tokens[i];
            }
            i++;
        }
    }
    
    cmd->argv[arg_idx] = NULL; // Null-terminate argv
    cmd->argc = arg_idx;
    return 0;
}

// Main parsing function
parse_result_t parse_command_line(char *input) {
    parse_result_t result;
    memset(&result, 0, sizeof(parse_result_t));

    // Tokenize input
    char *tokens[MAX_TOKENS];
    int token_count = tokenize(input, tokens, MAX_TOKENS);

    if (token_count <= 0) {
        result.parse_error = 1;
        strcpy((uint8_t*)result.error_msg, (const uint8_t*)"Tokenization failed");
        return result;
    }

    // Check for background execution (& at end)
    int background = 0;
    if (token_count > 0 && strcmp((const uint8_t*)tokens[token_count-1], (const uint8_t*)"&") == 0) {
        background = 1;
        free(tokens[token_count-1]);
        token_count--;
    }

    // Count commands (separated by pipes)
    int cmd_count = 1;
    for (int i = 0; i < token_count; i++) {
        if (strcmp((const uint8_t*)tokens[i], (const uint8_t*)"|") == 0) {
            cmd_count++;
        }
    }

    if (cmd_count > MAX_PIPELINE_LEN) {
        result.parse_error = 1;
        strcpy((uint8_t*)result.error_msg, (const uint8_t*)"Too many commands in pipeline");
        // Free tokens before returning
        for (int i = 0; i < token_count; i++) {
            free(tokens[i]);
        }
        return result;
    }

    // Allocate space for commands
    result.pipeline.commands = (command_t *)malloc(sizeof(command_t) * cmd_count);
    if (!result.pipeline.commands) {
        result.parse_error = 1;
        strcpy((uint8_t*)result.error_msg, (const uint8_t*)"Memory allocation failed");
        // Free tokens before returning
        for (int i = 0; i < token_count; i++) {
            free(tokens[i]);
        }
        return result;
    }

    // Initialize all commands
    for (int i = 0; i < cmd_count; i++) {
        memset(&result.pipeline.commands[i], 0, sizeof(command_t));
    }

    // Parse each command in the pipeline
    int cmd_idx = 0;
    int cmd_start = 0;

    for (int i = 0; i <= token_count; i++) {
        int is_pipe = (i < token_count && strcmp((const uint8_t*)tokens[i], (const uint8_t*)"|") == 0);
        int is_end = (i == token_count);

        if (is_pipe || is_end) {
            // Parse command from cmd_start to i
            if (parse_command_args(tokens, cmd_start, i, &result.pipeline.commands[cmd_idx]) != 0) {
                result.parse_error = 1;
                strcpy((uint8_t*)result.error_msg, (const uint8_t*)"Failed to parse command");
                // Free tokens before returning
                for (int j = 0; j < token_count; j++) {
                    free(tokens[j]);
                }
                return result;
            }
            cmd_idx++;
            cmd_start = i + 1;
        }
    }

    // Store tokens array reference in pipeline for cleanup
    // We need to keep track of tokens to free them later
    result.pipeline.tokens_to_free = (char **)malloc(sizeof(char*) * token_count);
    if (result.pipeline.tokens_to_free) {
        for (int i = 0; i < token_count; i++) {
            result.pipeline.tokens_to_free[i] = tokens[i];
        }
        result.pipeline.token_count = token_count;
    }

    result.pipeline.num_commands = cmd_count;
    result.pipeline.background = background;
    return result;
}

// Free allocated resources
void free_parse_result(parse_result_t *result) {
    if (result->pipeline.commands) {
        for (int i = 0; i < result->pipeline.num_commands; i++) {
            if (result->pipeline.commands[i].argv) {
                free(result->pipeline.commands[i].argv);
            }
        }
        free(result->pipeline.commands);
    }
    
    // Free all tokens that were allocated during tokenization
    if (result->pipeline.tokens_to_free) {
        for (int i = 0; i < result->pipeline.token_count; i++) {
            free(result->pipeline.tokens_to_free[i]);
        }
        free(result->pipeline.tokens_to_free);
    }
}

// Debug printing
void print_parse_result(parse_result_t *result) {
    if (result->parse_error) {
        printf("Parse error: %s\n", result->error_msg);
        return;
    }
    
    printf("Pipeline with %d command(s):\n", result->pipeline.num_commands);
    
    for (int i = 0; i < result->pipeline.num_commands; i++) {
        command_t *cmd = &result->pipeline.commands[i];
        printf("  Command %d: %s\n", i, cmd->cmd);
        printf("    Args: ");
        for (int j = 0; j < cmd->argc; j++) {
            printf("%s ", cmd->argv[j]);
        }
        printf("\n");
        
        if (cmd->stdin_redir.type != REDIR_NONE) {
            printf("    < %s\n", cmd->stdin_redir.path);
        }
        if (cmd->stdout_redir.type != REDIR_NONE) {
            char *op = (cmd->stdout_redir.type == REDIR_APPEND) ? ">>" : ">";
            printf("    %s %s\n", op, cmd->stdout_redir.path);
        }
        if (cmd->stderr_redir.type != REDIR_NONE) {
            char *op = (cmd->stderr_redir.type == REDIR_ERROR_APPEND) ? "2>>" : "2>";
            printf("    %s %s\n", op, cmd->stderr_redir.path);
        }
    }
}
