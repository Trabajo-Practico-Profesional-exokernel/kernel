#include "meta/apps_info.h"

extern char _binary_touch_app_bin_start[],_binary_touch_app_bin_size[];
extern char _binary_rm_app_bin_start[],_binary_rm_app_bin_size[];
extern char _binary_stat_app_bin_start[],_binary_stat_app_bin_size[];
extern char _binary_cat_app_bin_start[],_binary_cat_app_bin_size[];
extern char _binary_coordinator_app_bin_start[],_binary_coordinator_app_bin_size[];
extern char _binary_echo_app_bin_start[],_binary_echo_app_bin_size[];
extern char _binary_example_dapp_app_bin_start[],_binary_example_dapp_app_bin_size[];
extern char _binary_filesystem_app_bin_start[],_binary_filesystem_app_bin_size[];
extern char _binary_fs/cat_app_bin_start[],_binary_fs/cat_app_bin_size[];
extern char _binary_fs/chmod_app_bin_start[],_binary_fs/chmod_app_bin_size[];
extern char _binary_fs/chown_app_bin_start[],_binary_fs/chown_app_bin_size[];
extern char _binary_fs/link_app_bin_start[],_binary_fs/link_app_bin_size[];
extern char _binary_fs/ls_app_bin_start[],_binary_fs/ls_app_bin_size[];
extern char _binary_fs/mkdir_app_bin_start[],_binary_fs/mkdir_app_bin_size[];
extern char _binary_fs/rmdir_app_bin_start[],_binary_fs/rmdir_app_bin_size[];
extern char _binary_fs/stat_app_bin_start[],_binary_fs/stat_app_bin_size[];
extern char _binary_fs/touch_app_bin_start[],_binary_fs/touch_app_bin_size[];
extern char _binary_fs/unlink_app_bin_start[],_binary_fs/unlink_app_bin_size[];
extern char _binary_hello_world_app_bin_start[],_binary_hello_world_app_bin_size[];
extern char _binary_infinite_loop_app_bin_start[],_binary_infinite_loop_app_bin_size[];
extern char _binary_kill_app_bin_start[],_binary_kill_app_bin_size[];
extern char _binary_malloc_program_app_bin_start[],_binary_malloc_program_app_bin_size[];
extern char _binary_page_fault_app_bin_start[],_binary_page_fault_app_bin_size[];
extern char _binary_periodic_yield_app_bin_start[],_binary_periodic_yield_app_bin_size[];
extern char _binary_pipe_app_bin_start[],_binary_pipe_app_bin_size[];
extern char _binary_proc_a_app_bin_start[],_binary_proc_a_app_bin_size[];
extern char _binary_proc_b_app_bin_start[],_binary_proc_b_app_bin_size[];
extern char _binary_read_write_shell_app_bin_start[],_binary_read_write_shell_app_bin_size[];
extern char _binary_shell_app_bin_start[],_binary_shell_app_bin_size[];
extern char _binary_show_env_app_bin_start[],_binary_show_env_app_bin_size[];
extern char _binary_simple_frk_app_bin_start[],_binary_simple_frk_app_bin_size[];


struct AppBinaryInfo _binary_apps[] = {
    {_binary_touch_app_bin_start, (size_t) _binary_touch_app_bin_size},
    {_binary_rm_app_bin_start, (size_t) _binary_rm_app_bin_size},
    {_binary_stat_app_bin_start, (size_t) _binary_stat_app_bin_size},
    {_binary_cat_app_bin_start, (size_t) _binary_cat_app_bin_size},
    {_binary_coordinator_app_bin_start, (size_t) _binary_coordinator_app_bin_size},
    {_binary_echo_app_bin_start, (size_t) _binary_echo_app_bin_size},
    {_binary_example_dapp_app_bin_start, (size_t) _binary_example_dapp_app_bin_size},
    {_binary_filesystem_app_bin_start, (size_t) _binary_filesystem_app_bin_size},
    {_binary_fs/cat_app_bin_start, (size_t) _binary_fs/cat_app_bin_size},
    {_binary_fs/chmod_app_bin_start, (size_t) _binary_fs/chmod_app_bin_size},
    {_binary_fs/chown_app_bin_start, (size_t) _binary_fs/chown_app_bin_size},
    {_binary_fs/link_app_bin_start, (size_t) _binary_fs/link_app_bin_size},
    {_binary_fs/ls_app_bin_start, (size_t) _binary_fs/ls_app_bin_size},
    {_binary_fs/mkdir_app_bin_start, (size_t) _binary_fs/mkdir_app_bin_size},
    {_binary_fs/rmdir_app_bin_start, (size_t) _binary_fs/rmdir_app_bin_size},
    {_binary_fs/stat_app_bin_start, (size_t) _binary_fs/stat_app_bin_size},
    {_binary_fs/touch_app_bin_start, (size_t) _binary_fs/touch_app_bin_size},
    {_binary_fs/unlink_app_bin_start, (size_t) _binary_fs/unlink_app_bin_size},
    {_binary_hello_world_app_bin_start, (size_t) _binary_hello_world_app_bin_size},
    {_binary_infinite_loop_app_bin_start, (size_t) _binary_infinite_loop_app_bin_size},
    {_binary_kill_app_bin_start, (size_t) _binary_kill_app_bin_size},
    {_binary_malloc_program_app_bin_start, (size_t) _binary_malloc_program_app_bin_size},
    {_binary_page_fault_app_bin_start, (size_t) _binary_page_fault_app_bin_size},
    {_binary_periodic_yield_app_bin_start, (size_t) _binary_periodic_yield_app_bin_size},
    {_binary_pipe_app_bin_start, (size_t) _binary_pipe_app_bin_size},
    {_binary_proc_a_app_bin_start, (size_t) _binary_proc_a_app_bin_size},
    {_binary_proc_b_app_bin_start, (size_t) _binary_proc_b_app_bin_size},
    {_binary_read_write_shell_app_bin_start, (size_t) _binary_read_write_shell_app_bin_size},
    {_binary_shell_app_bin_start, (size_t) _binary_shell_app_bin_size},
    {_binary_show_env_app_bin_start, (size_t) _binary_show_env_app_bin_size},
    {_binary_simple_frk_app_bin_start, (size_t) _binary_simple_frk_app_bin_size},

};
