# 链表

本项目直接采用 Linux kernel 链表实现

## 内核链表与普通链表对比

一般而言，链表的实现使用如下结构

```c
struct list_node {
    struct list_node *next;
    void *data;
};
```

也即将数据嵌入list 结点中，为了实现多态，一般将数据定义成 (void *), 并实现对应的比较与拷贝等函数；对于C++，可以用模板与运算符重载达到同样的目的

内核链表则不同，它将list 嵌入数据结构中，例如

```c
struct people {
    unsigned int age;
    char name[128];
    struct list_head list;
};
```

内核链表是一个双向环形链表，`struct list_head` 定义如下

```c
struct list_head {
    struct list_head *next, *prev;
};
```

显然，我们使用 `people.list.next` 来访问下一个结点，`people.list.prev` 来访问前一个结点

这里的关键在于如何从list 地址找到结构体地址，在C语言中，结构体中的变量的偏移地址在编译时已经确定了。使用内核提供的一个宏`container_of`
即可获得包含`struct list_head` 的结构的地址

当然，`list.h` 中已经定义好了我们所需操作的接口

## 使用链表

```c
void INIT_LIST_HEAD(struct list_head *list); // 初始化链表
void list_add(struct list_head *_new, struct list_head *head); // 添加一个结点（使用_new是为了兼容C++, new 在C++ 中是关键字）
void list_add_tail(struct list_head *_new, struct list_head *head); // 添加一个结点至链表尾
void list_del(struct list_head *entry);     // 删除一个结点，该操作并不释放结构体内存
void list_del_init(struct list_head *entry); // 删除一个结点并对其重新初始化
void list_move(struct list_head *list, struct list_head *head); // 把结点从一个链表移到另一个链表
void list_move_tail(struct list_head *list, struct list_head *head); // 把结点从一个链表移到另一个链表的末尾
int list_empty(const struct list_head *head); // 判断结点是否为空，若不为空返回0，否则非0
void list_splice(const struct list_head *list, struct list_head *head); // 合并两个未连接的链表，将list 指向的链表插入到head 后面
void list_splice_init(struct list_head *list, struct list_head *head); // 合并两个未连接的链表并初始化list
#define list_entry(ptr, type, member)   // 获取包含member(struct list_head) 的type 地址，ptr: struct list_head *
// 以上操作复杂度均为 O(1)

#define list_for_each(p, head)        // 遍历链表，p, head: struct list_head *, 其中head 是头结点
#define list_for_each_entry(pos, head, member) // 遍历链表，pos: 包含list_head 的结构体指针
#define list_for_each_entry_reverse(pos, head, member) // 反向遍历链表
#define list_for_each_entry_safe(pos, next, head, member) // 遍历的同时支持删除, 额外提供next(struct list_head*)
#define list_for_each_entry_safe_reverse(pos, next, head, member) // 反向遍历的同时支持删除
// 以上遍历操作复杂度均为 O(n)
```

## 示例

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "list.h"

struct people {
    unsigned int age;
    char name[128];
    struct list_head list;
};

static inline void err_quit(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    struct list_head lists;
    INIT_LIST_HEAD(&lists);

    struct people *Alice = (struct people *)malloc(sizeof(struct people));
    if (Alice == NULL) {
        err_quit("malloc fail");
    }
    Alice->age = 29;
    strcpy(Alice->name, "Alice");
    list_add(&Alice->list, &lists);

    struct people *River = (struct people *)malloc(sizeof(struct people));
    if (River == NULL) {
        err_quit("malloc fail");
    }
    River->age = 7;
    strcpy(River->name, "River");
    list_add_tail(&River->list, &lists);

    struct people *Edith = (struct people *)malloc(sizeof(struct people));
    if (Edith == NULL) {
        err_quit("malloc fail");
    }
    Edith->age = 17;
    strcpy(Edith->name, "Edith");
    list_add_tail(&Edith->list, &lists);

    struct people *pos, *next;
    list_for_each_entry_safe(pos, next, &lists, list) {
        if (pos->age < 10) {
            list_del(&pos->list);
        }
    }

    list_for_each_entry(pos, &lists, list) {
        printf("%s\n", pos->name);
    }
}
```
