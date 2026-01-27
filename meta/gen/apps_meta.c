#include "meta/apps_info.h"

extern char _binary_touch_app_bin_start[],_binary_touch_app_bin_size[];
extern char _binary_rm_app_bin_start[],_binary_rm_app_bin_size[];
extern char _binary_stat_app_bin_start[],_binary_stat_app_bin_size[];
extern char _binary_cat_app_bin_start[],_binary_cat_app_bin_size[];
extern char _binary_coordinator_app_bin_start[],_binary_coordinator_app_bin_size[];
extern char _binary_filesystem_app_bin_start[],_binary_filesystem_app_bin_size[];
extern char _binary_hello_world_app_bin_start[],_binary_hello_world_app_bin_size[];
extern char _binary_kalloc_program_app_bin_start[],_binary_kalloc_program_app_bin_size[];
extern char _binary_page_fault_app_bin_start[],_binary_page_fault_app_bin_size[];
extern char _binary_periodic_yield_app_bin_start[],_binary_periodic_yield_app_bin_size[];
extern char _binary_pipe_app_bin_start[],_binary_pipe_app_bin_size[];
extern char _binary_proc_a_app_bin_start[],_binary_proc_a_app_bin_size[];
extern char _binary_proc_b_app_bin_start[],_binary_proc_b_app_bin_size[];
extern char _binary_read_write_shell_app_bin_start[],_binary_read_write_shell_app_bin_size[];
extern char _binary_shell_app_bin_start[],_binary_shell_app_bin_size[];
extern char _binary_tests_shell_app_bin_start[],_binary_tests_shell_app_bin_size[];


struct AppBinaryInfo _binary_apps[] = {
    {_binary_touch_app_bin_start, (size_t) _binary_touch_app_bin_size},
    {_binary_rm_app_bin_start, (size_t) _binary_rm_app_bin_size},
    {_binary_stat_app_bin_start, (size_t) _binary_stat_app_bin_size},
    {_binary_cat_app_bin_start, (size_t) _binary_cat_app_bin_size},
    {_binary_coordinator_app_bin_start, (size_t) _binary_coordinator_app_bin_size},
    {_binary_filesystem_app_bin_start, (size_t) _binary_filesystem_app_bin_size},
    {_binary_hello_world_app_bin_start, (size_t) _binary_hello_world_app_bin_size},
    {_binary_kalloc_program_app_bin_start, (size_t) _binary_kalloc_program_app_bin_size},
    {_binary_page_fault_app_bin_start, (size_t) _binary_page_fault_app_bin_size},
    {_binary_periodic_yield_app_bin_start, (size_t) _binary_periodic_yield_app_bin_size},
    {_binary_pipe_app_bin_start, (size_t) _binary_pipe_app_bin_size},
    {_binary_proc_a_app_bin_start, (size_t) _binary_proc_a_app_bin_size},
    {_binary_proc_b_app_bin_start, (size_t) _binary_proc_b_app_bin_size},
    {_binary_read_write_shell_app_bin_start, (size_t) _binary_read_write_shell_app_bin_size},
    {_binary_shell_app_bin_start, (size_t) _binary_shell_app_bin_size},
    {_binary_tests_shell_app_bin_start, (size_t) _binary_tests_shell_app_bin_size},

};
