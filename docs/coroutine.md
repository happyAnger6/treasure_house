# C语言协程库实现说明

## COROUTINE状态

corouting有3种状态, RUNNALBE, RUNNING, WAITING.

+ WAITING: corouting暂停并等待一些条件以继续运行.比如:系统调用,同步操作(原子或锁操作),这种延迟是性能差的根源.
+ RUNNABLE: corouting具备运行条件正在等待分配processor以执行指令
+ RUNNING: corouting已经分配到processor,并正在上面执行指令


## 上下文切换

### 原理

corouting在用户态进行上下文切换,上下文主要包括:堆栈,寄存器(IP, SP等).  
上下文切换主要通过<ucontext.h>中定义的getcontext, setcontext, makecontext, swapcontext实现.

### 时机

+ corouting主动调用coroutine_yield(),如果有其它待运行的coroutine则主动让出processor
+ 协程库asyncio API选择合适的时机进行上下文切换,如调用阻塞API,如corouting_sleep. 
+ 如果你在协程中执行cpu密集型操作或直接调用阻塞的C api,那么会影响当前processor的调度和运行.

#### asyncio

+ asyncio提供一系列api用于在协程环境中编写并发代码.
+ asyncio是coroutine框架提供的api可以用于实现高性能网络服务器,数据库连接库,分布式任务队列等.
+ asyncio适合IO密集型和高级别的结构化网络程序
