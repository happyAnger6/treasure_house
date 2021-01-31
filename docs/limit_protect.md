# 用户态每cpu变量

## 1.实现  
用户态的每cpu内存和内核态实现方式一样:每个cpu有自己的%gs基址.在x86_64上,%fs用于线程私有存储,%gs通常未使用.

用户态的程序不能禁止抢占,但是在x86上读-修改-写操作对于中断和上下文切换是原子的.
这样每cpu计数,环形缓冲指针,每cpu锁和其它一些类似的东西可以用一种很高效的方式实现.

After this patch kernel recalculates %gs at each context switch.
This's implemented only via MSR_KERNEL_GS_BASE. Loading base via gdt
selector might be faster but it's much more complicated.

By the way, newer Intel cpus have even faster instructions for
changing %fs/%gs, but they are still not supported by the kernel.

Additional overhead is near to zero: this patch adds one extra multiplication
into __switch_to (only if gs is set by user-space and its base is above 4Gb):

        if (next->gs)
-               wrmsrl(MSR_KERNEL_GS_BASE, next->gs);
+               wrmsrl(MSR_KERNEL_GS_BASE, next->gs +
+                               cpu * next->gs_cpu_stride);

Child inherits setup from parent at clone because it gets a copy of task_struct.
Changing %gs via any other interface (selector, ARCH_SET_GS) disables striping.

Interface:

int arch_prctl(ARCH_GET_GS_PERCPU, unsigned long arg[2]);
int arch_prctl(ARCH_SET_GS_PERCPU, unsigned long arg[2]);

arg[0] - base address for cpu0
arg[1] - stride to each next cpu