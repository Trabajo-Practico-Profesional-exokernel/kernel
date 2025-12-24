// __attribute__((section(".text.proc_a")))
// int syscall(int num, int a, int b, int c)
// {
//     int ret;
//     asm volatile (
//         "int $0x80"
//         : "=a"(ret)
//         : "a"(num), "b"(a), "c"(b), "d"(c)
//         : "memory"
//     );
//     return ret;
// }