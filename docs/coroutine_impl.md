# coroutine库实现文档

## 1. 协程活动

### 1.1 等待队列

等待队列: 用于使协程等待某一特定事件发生,而无须频繁轮循.协程在等待期间睡眠,在事件发生后由协程调度器唤醒.

#### 结构说明

##### 1.1.1 wait_queue_t

```C
typedef struct wait_queue {
    unsigned int flag;
    void *private; 
    wait_queue_func_t func;
    struct list_head task_list;
} wait_queue_t;

```

+ flags: WQ_FLAG_EXCLUSIVE或者为0,WQ_FLAG_EXCLUSIVE表示等待协程想要被独占唤醒.
+ private: 通常指向等待唤醒的coroutine实例
+ func: 唤醒协程函数
+ task_list: 用于将协程放到等待队列中

##### 为了将协程投入睡眠,需要调用wait_event或某个等价函数,当前协程睡眠并将控制权释放给调度器
##### 在另外一处地方,必须调用wake_up或某个等价函数来唤醒等待队列中的协程.