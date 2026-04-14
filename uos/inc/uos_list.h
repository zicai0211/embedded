#ifndef UOS_LIST_H_
#define UOS_LIST_H_
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define OS_MAX_PRI 3

typedef struct ListItem
{
    struct ListItem *next;
    struct ListItem *prev;
    void *owner;//记录该项所属的对象，通常是任务TCB指针
    uint32_t item_value;//排序键值，通常用于任务唤醒时间或优先级
    struct list *container;//记录该项所在的链表，方便删除时快速定位
} ListItem_t;

typedef struct list
{
    ListItem_t list_end;//链表尾哨兵节点，永远位于链表末端，item_value为0
    uint32_t item_count;
}List_t;

void list_init(List_t *list);
void list_item_init(ListItem_t *list);
void list_insert_tail(List_t *list, ListItem_t *new_item);
void list_insert_head(List_t *list, ListItem_t *item);
void list_insert_ordered(List_t *list, ListItem_t *item);


void list_remove(ListItem_t *item);
bool list_is_empty(List_t *list);
ListItem_t *list_get_head(const List_t *list);
ListItem_t *list_pop_head(List_t *list);
uint32_t list_count(const List_t *list);

#endif // !LIST_H_
