# C语言协程库实现说明
[代码实现](https://github.com/happyAnger6/treasure_house/tree/master/src/coroutine)
[一些细节](./coroutine_impl.md)

## 1. 当前支持的功能概览

### 1.1 创建任意数量协程并在协程中yield

```cpp
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "gtest/gtest.h"

#include "coroutine.h"
#include "asyncio.h"

static int g_run_sums = 0;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

static void co(void *args)
{
    int num = *((int *)args);

    pthread_mutex_lock(&g_lock);
    g_run_sums += num;  // 每个协程分别递增全局变量
    pthread_mutex_unlock(&g_lock);

    printf("coroutine:%d begin...\r\n", num);
    coroutine_yield();
    printf("coroutine:%d ended...\r\n", num);
}

TEST(coroutine_create, three_cos_run_success) 
{
    int a = 1, b = 2, c = 3; //a, b, c三个协程依次对全局变量g_run_sums增加1,2,3
    coroutine_init();
    coroutine_create(co, (void*)&a);
    coroutine_create(co, (void*)&b);
    coroutine_create(co, (void*)&c);

    coroutine_loop();

    EXPECT_EQ(g_run_sums, 6); // 最终全局变量为6
}
```

### 1.2 创建2个协程,其中一个睡眠100ms

```cpp
  
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "gtest/gtest.h"

#include "coroutine.h"
#include "asyncio.h"

static int seq = 0;
static int co_seq[2] = {0};

static void co_sleep(void *args)
{
    printf("co sleep begin.\r\n");
    asyncio_sleep(100); // 调用asyncio api睡眠100ms.
    co_seq[seq++] = 100;
    printf("co sleep end.\r\n");
}

static void co_nosleep(void *args)
{
    printf("co no sleep begin.\r\n");
    co_seq[seq++] = 200; 
    printf("co no sleep end.\r\n");
}

TEST(coroutine_run, co_sleep) 
{
    coroutine_init();
    coroutine_create(co_sleep, NULL);
    coroutine_create(co_nosleep, NULL);

    coroutine_loop();

    EXPECT_EQ(co_seq[0], 200); //验证未睡眠协程先完成了执行
    EXPECT_EQ(co_seq[1], 100);
}
```

## 2. COROUTINE状态

corouting有3种状态, RUNNALBE, RUNNING, WAITING.

+ WAITING: corouting暂停并等待一些条件以继续运行.比如:sleep, 系统调用,同步操作(原子或锁操作),这种延迟是性能差的根源.
+ RUNNABLE: corouting具备运行条件正在等待分配processor以执行指令
+ RUNNING: corouting已经分配到processor,并正在上面执行指令


## 3. 调度器实现

+ processor_t: 管理一个实际的os线程,内部使用队列维护分配给它的coroutine,使用epoll进行事件循环.
+ processors_t: 全局processor_t管理器,创建时会按照实际的cpu个数创建对应的processor_t, 它负责将新协程按照一定算法分配给某个processor_t.  
同时负责检测没有任何协程时退出进程.

### 3.1 主进程何时退出

当没有任何协程存在时,则退出主进程.

#### 3.1.1 实现原理

模拟实现了Golang中的waitGroup, 用于等待所有协程退出.新协程创建会调用waitGroup_add,协程结束会调用waitGroup_del,当waitGroup空闲时  
则说明所有协程都已经退出.

### 3.2 processor_t调度主循环处理

+ 1. 循环遍历coroutine就绪队列,依次运行coroutine.
+ 2. 如果没有就绪的coroutine且本地队列上coroutine个数为0,则进行步骤3,否则进行步骤4
+ 3. 通过条件变量等待分配新的coroutine,如果收到了条件变量且是退出指令,则进行步骤5,否则进行步骤1.
+ 4. 本地队列还有coroutine,但是coroutine都在等待事件,则进行事件循环以等待指定事件的到来,这样就会有coroutine就绪,进行步骤1.
+ 5. 退出主循环

### 3.3 上下文切换实现

#### 3.3.1 原理

corouting在用户态进行上下文切换,上下文主要包括:堆栈,寄存器(IP, SP等).  
上下文切换主要通过<ucontext.h>中定义的getcontext, setcontext, makecontext, swapcontext实现.

### 3.3.2 上下文切换时机

+ corouting主动调用coroutine_yield(),如果有其它待运行的coroutine则主动让出processor_t
+ 协程中调用了协程库asyncio API,则由API选择合适的时机进行上下文切换,如调用阻塞API,如corouting_sleep. 
+ 如果你在协程中执行cpu密集型操作或直接调用阻塞的C api,那么会影响当前processor的调度和运行.

#### 3.3.3 堆栈使用

+ 每个processor_t维护1M的堆栈空间M
+ 协程刚创建时为RUNNABLE状态,此时直接使用M作为堆栈,当协程需要放权时保存当前堆栈到协程自己的空间M0
+ 协程恢复运行时,将保存的堆栈M0还原到M中继续运行

这样每个协程最大都可以有1M的堆栈空间,且堆栈空间能够按需分配,每个processor_t上堆栈的消耗为所有协程  
实际使用的堆栈内存+1M.

如果不这样实现,每个协程都需要初始分配1M空间,消耗为协程个数*1M.

## 4. 异步操作协程库asyncio实现

+ asyncio提供一系列api用于在协程环境中编写并发代码.
+ asyncio是coroutine框架提供的api可以用于实现高性能网络服务器,数据库连接库,分布式任务队列等.
+ asyncio适合IO密集型和高级别的结构化网络程序

### 4.1 当前支持的API

+ coroutine_sleep(long delay_ms): 当前协程休眠指定ms.
+ coroutine_yield(): 当前协程主动放权给其它就绪协程,由调度器选择合适时机再重新调度.


## 5. 代码说明

### 5.1 编译代码

```shell

cmake -H. -Bbuild
cmake --build ./build
```

### 5.2 运行测试

```shell
cd build
ctest --verbose
```

## 6. todo list

+ 支持通过线程池执行阻塞C api,如getaddrinfo
+ 优化当前sched, coroutine获取方式,使用线程私有存储
+ 支持在非processor_t上下文创建协程
+ 支持future, 在异步回调代码和同步协程代码之间提供桥接, 一般不将future暴露给api使用者.
+ 支持socket协程同步操作, connect, accept, recv, send

## 7. 回调式编程的一些缺点

+ 代码流程分散,不易阅读
+ 增加了内存管理和异常管理的复杂性,容易遗漏内存释放等问题