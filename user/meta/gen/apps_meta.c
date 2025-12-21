#include "meta/apps_info.h"

extern char _binary_apps_build_filesystem_app_bin_start[],_binary_apps_build_filesystem_app_bin_size[];
extern char _binary_apps_build_shell_app_bin_start[],_binary_apps_build_shell_app_bin_size[];
extern char _binary_apps_build_tests_shell_app_bin_start[],_binary_apps_build_tests_shell_app_bin_size[];
extern char _binary_apps_build_hello_world_app_bin_start[],_binary_apps_build_hello_world_app_bin_size[];
extern char _binary_apps_build_periodic_yield_app_bin_start[],_binary_apps_build_periodic_yield_app_bin_size[];
extern char _binary_apps_build_proc_a_app_bin_start[],_binary_apps_build_proc_a_app_bin_size[];
extern char _binary_apps_build_proc_b_app_bin_start[],_binary_apps_build_proc_b_app_bin_size[];
extern char _binary_apps_build_rm_app_bin_start[],_binary_apps_build_rm_app_bin_size[];
extern char _binary_apps_build_stat_app_bin_start[],_binary_apps_build_stat_app_bin_size[];
extern char _binary_apps_build_touch_app_bin_start[],_binary_apps_build_touch_app_bin_size[];


struct AppBinaryInfo _binary_apps[] = {
    {_binary_apps_build_filesystem_app_bin_start, (size_t) _binary_apps_build_filesystem_app_bin_size},
    {_binary_apps_build_shell_app_bin_start, (size_t) _binary_apps_build_shell_app_bin_size},
    {_binary_apps_build_tests_shell_app_bin_start, (size_t) _binary_apps_build_tests_shell_app_bin_size},
    {_binary_apps_build_hello_world_app_bin_start, (size_t) _binary_apps_build_hello_world_app_bin_size},
    {_binary_apps_build_periodic_yield_app_bin_start, (size_t) _binary_apps_build_periodic_yield_app_bin_size},
    {_binary_apps_build_proc_a_app_bin_start, (size_t) _binary_apps_build_proc_a_app_bin_size},
    {_binary_apps_build_proc_b_app_bin_start, (size_t) _binary_apps_build_proc_b_app_bin_size},
    {_binary_apps_build_rm_app_bin_start, (size_t) _binary_apps_build_rm_app_bin_size},
    {_binary_apps_build_stat_app_bin_start, (size_t) _binary_apps_build_stat_app_bin_size},
    {_binary_apps_build_touch_app_bin_start, (size_t) _binary_apps_build_touch_app_bin_size},

};
