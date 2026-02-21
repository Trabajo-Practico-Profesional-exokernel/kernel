#include "shell_executor.h"
#include "shell_parser.h"
#include "command_handler.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "lib.h"

// Helper: apply redirections to current process
static int apply_redirections(command_t *cmd) {
    // Input redirection
    if (cmd->stdin_redir.type == REDIR_INPUT) {
        int fd = open(cmd->stdin_redir.path, 0); // 0 = O_RDONLY
        if (fd < 0) {
            printf("Error: Cannot open file for input: %s\n", cmd->stdin_redir.path);
            return -1;
        }
        // Redirect stdin to this file
        dup2(fd, 0);
        close(fd);
    }
    
    // Output redirection (> overwrites)
    if (cmd->stdout_redir.type == REDIR_OUTPUT) {
        int fd = open(cmd->stdout_redir.path, 1); // 1 = O_WRONLY
        if (fd < 0) {
            printf("Error: Cannot open file for output: %s\n", cmd->stdout_redir.path);
            return -1;
        }
        dup2(fd, 1); // Redirect stdout
        close(fd);
    }
    
    // Output append (>>)
    if (cmd->stdout_redir.type == REDIR_APPEND) {
        // Note: append mode would need syscall support
        // For now, treat same as REDIR_OUTPUT
        int fd = open(cmd->stdout_redir.path, 1);
        if (fd < 0) {
            printf("Error: Cannot open file for append: %s\n", cmd->stdout_redir.path);
            return -1;
        }
        dup2(fd, 1);
        close(fd);
    }
    
    // Error redirection (2> redirects stderr)
    if (cmd->stderr_redir.type == REDIR_ERROR || cmd->stderr_redir.type == REDIR_ERROR_APPEND) {
        int fd = open(cmd->stderr_redir.path, 1);
        if (fd < 0) {
            printf("Error: Cannot open file for stderr: %s\n", cmd->stderr_redir.path);
            return -1;
        }
        dup2(fd, 2); // Redirect stderr
        close(fd);
    }
    
    return 0;
}

// Helper: check if command is a built-in (must run in shell process)
// Only cd, pwd, exit, clear, smile, procls are truly built-in
static int is_builtin(const char *cmd) {
    const char *builtins[] = {
        "cd", "pwd", "exit", "clear", "smile", "procls", NULL
    };
    
    for (int i = 0; builtins[i] != NULL; i++) {
        if (strcmp((const uint8_t*)cmd, (const uint8_t*)builtins[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Execute a single command (no piping)
exec_result_t execute_single_command(command_t *cmd) {
    exec_result_t result;
    memset(&result, 0, sizeof(exec_result_t));
    
    // For built-in commands, execute directly
    if (is_builtin(cmd->cmd)) {
        // Reconstruct args string for built-in handlers
        // Built-ins expect args as a single string
        char args_str[256] = {0};
        if (cmd->argc > 1) {
            strcpy((uint8_t*)args_str, (const uint8_t*)cmd->argv[1]);
            for (int i = 2; i < cmd->argc; i++) {
                strcat((uint8_t*)args_str, (const uint8_t*)" ");
                strcat((uint8_t*)args_str, (const uint8_t*)cmd->argv[i]);
            }
        }
        result.exit_code = exec_command(cmd->cmd, args_str);
        return result;
    }
    
    // For external programs, fork and exec
    int pid = sys_fork();
    if (pid == 0) {
        // Child process
        if (apply_redirections(cmd) != 0) {
            exit(1);
        }
        sys_execv(cmd->cmd, (const char **)cmd->argv);
        printf("Error: Cannot execute %s\n", cmd->cmd);
        exit(127);
    } else if (pid > 0) {
        // Parent waits for child
        int status = wait(pid);
        result.exit_code = status;
    } else {
        result.execution_error = 1;
        strcpy((uint8_t*)result.error_msg, (const uint8_t*)"Fork failed");
    }
    
    return result;
}

// Execute a pipeline (potentially multiple commands connected by pipes)
exec_result_t execute_pipeline(pipeline_t *pipeline) {
    exec_result_t result;
    memset(&result, 0, sizeof(exec_result_t));
    
    if (pipeline->num_commands == 0) {
        result.execution_error = 1;
        strcpy((uint8_t*)result.error_msg, (const uint8_t*)"Empty pipeline");
        return result;
    }
     
    // Single command - no piping needed
    if (pipeline->num_commands == 1) {
        return execute_single_command(&pipeline->commands[0]);
    }
    
    // Multiple commands - need piping
    int pids[MAX_PIPELINE_LEN];
    int pipes[MAX_PIPELINE_LEN - 1][2]; // pipes between commands
    
    // Create all pipes
    for (int i = 0; i < pipeline->num_commands - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            result.execution_error = 1;
            strcpy((uint8_t*)result.error_msg, (const uint8_t*)"Pipe creation failed");
            return result;
        }
    }
    
    // Fork and execute each command
    for (int i = 0; i < pipeline->num_commands; i++) {
        int pid = sys_fork();
        
        if (pid == 0) {
            // Child process
            
            // Setup input redirection or pipe from previous command
            if (i == 0) {
                // First command: read from stdin or file
                if (pipeline->commands[i].stdin_redir.type != REDIR_NONE) {
                    if (apply_redirections(&pipeline->commands[i]) != 0) {
                        exit(1);
                    }
                }
            } else {
                // Not first: read from previous pipe
                dup2(pipes[i-1][0], 0);
                close(pipes[i-1][0]);
            }
            
            // Setup output redirection or pipe to next command
            if (i == pipeline->num_commands - 1) {
                // Last command: write to stdout or file
                if (pipeline->commands[i].stdout_redir.type != REDIR_NONE) {
                    if (apply_redirections(&pipeline->commands[i]) != 0) {
                        exit(1);
                    }
                }
            } else {
                // Not last: write to next pipe
                dup2(pipes[i][1], 1);
                close(pipes[i][1]);
            }
            
            // Apply any error redirections
            if (pipeline->commands[i].stderr_redir.type != REDIR_NONE) {
                int fd = open(pipeline->commands[i].stderr_redir.path, 1);
                if (fd >= 0) {
                    dup2(fd, 2);
                    close(fd);
                }
            }
            
            // Close all pipe ends in child
            for (int j = 0; j < pipeline->num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute the command
            if (is_builtin(pipeline->commands[i].cmd)) {
                // Built-ins in pipelines - not ideal but supported
                char args_str[256] = {0};
                if (pipeline->commands[i].argc > 1) {
                    strcpy((uint8_t*)args_str, (const uint8_t*)pipeline->commands[i].argv[1]);
                    for (int j = 2; j < pipeline->commands[i].argc; j++) {
                        strcat((uint8_t*)args_str, (const uint8_t*)" ");
                        strcat((uint8_t*)args_str, (const uint8_t*)pipeline->commands[i].argv[j]);
                    }
                }
                int ret = exec_command(pipeline->commands[i].cmd, args_str);
                exit(ret);
            } else {
                sys_execv(pipeline->commands[i].cmd, (const char **)pipeline->commands[i].argv);
                printf("Error: Cannot execute %s\n", pipeline->commands[i].cmd);
                exit(127);
            }
        } else if (pid > 0) {
            // Parent process - store PID
            pids[i] = pid;
        } else {
            // Fork failed
            result.execution_error = 1;
            strcpy((uint8_t*)result.error_msg, (const uint8_t*)"Fork failed");
            return result;
        }
    }
    
    // Parent: close all pipes
    for (int i = 0; i < pipeline->num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Wait for all children
    int last_status = 0;
    for (int i = 0; i < pipeline->num_commands; i++) {
        last_status = wait(pids[i]);
    }
    
    result.exit_code = last_status;
    return result;
}
