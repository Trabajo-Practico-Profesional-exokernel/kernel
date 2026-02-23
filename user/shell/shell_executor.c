#include "shell_executor.h"
#include "shell_parser.h"
#include "command_handler.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "lib.h"
#include "environ.h"

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

static int is_builtin(const char *cmd);

// Execute a single command (no piping)
exec_result_t execute_single_command(command_t *cmd) {
    exec_result_t result;
    memset(&result, 0, sizeof(exec_result_t));

    command_handler_t command = find_command_builtin(cmd->cmd);
    if(command){
        result.exit_code = command(cmd->argc - 1, &cmd->argv[1]);
        result.execution_error = result.exit_code != 0? 1:0;
        
        return result;
    }

    
    // For external programs, fork and exec
    int pid = sys_fork();
    
    setenv("pwd", get_curr_path(), 1);

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
        if (pipeline->background) {
            int pid = sys_fork();
            if (pid == 0) {
                set_gid(-1); // Update to gid == pid
                // Child process
                exec_result_t child_result = execute_single_command(&pipeline->commands[0]);
                exit(child_result.exit_code);
            } else if (pid > 0) {
                printf("[background pid %d]\n", pid);
                // Do not wait for child
                result.exit_code = 0;
                return result;
            } else {
                result.execution_error = 1;
                strcpy((uint8_t*)result.error_msg, (const uint8_t*)"Fork failed");
                return result;
            }
        } else {
            return execute_single_command(&pipeline->commands[0]);
        }
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

            // Load/set env
            setenv("pwd", get_curr_path(), 1);
            set_gid(-1); // Set gid == pid

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
            command_handler_t command = find_command_builtin(pipeline->commands[i].cmd);
            int ret = 0;
            if(command){
                ret = command(pipeline->commands[i].argc - 1, &pipeline->commands[i].argv[1]);
            } else {
                ret = command_default_exec(pipeline->commands[i].cmd, &pipeline->commands[i].argv[0]);
            }
            exit(ret);

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

    if (pipeline->background) {
        // Print all background PIDs
        printf("[background pids");
        for (int i = 0; i < pipeline->num_commands; i++) {
            printf(" %d", pids[i]);
        }
        printf("]\n");
        result.exit_code = 0;
        return result;
    }

    // Wait for all children
    int last_status = 0;
    for (int i = 0; i < pipeline->num_commands; i++) {
        last_status = wait(pids[i]);
    }

    result.exit_code = last_status;
    return result;
}
