#include <sys/sysinfo.h>

int get_cpu_nums()
{
    static int cpu_nums = -1;
    if (cpu_nums == -1) 
        cpu_nums = get_nprocs();
    
    return cpu_nums;
}