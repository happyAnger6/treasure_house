#ifndef _TH_LIST_H
#define _TH_LIST_H

struct list_head {
	struct list_head *next, *prev;
};

static inline void __list_add(struct list_head *new,
				struct list_head *prev,
				struct list_head *next)
{
	next->prev = new;
	new->prev = prev;
	new->next = next;
	prev->next = new;
}

static inline void list_add(struct list_head *new,
				struct list_head *head) 
{
	__list_add(new, head, head->next);
}

#endif
