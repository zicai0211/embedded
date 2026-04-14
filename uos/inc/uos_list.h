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
    ListItem_t list_end;//链表尾哨兵节点，永远位于链表末端，item_value为oxffffff
    uint32_t item_count;//链表项数量，维护链表状态
}List_t;

void list_init(List_t *list);//初始化链表
void list_item_init(ListItem_t *list);//初始化链表项
void list_insert_tail(List_t *list, ListItem_t *new_item);//将新项插入链表末尾
void list_insert_head(List_t *list, ListItem_t *item);//将新项插入链表头部
void list_insert_ordered(List_t *list, ListItem_t *item);//按照item_value值从大到小插入链表
void list_remove(ListItem_t *item);//从链表中移除指定项
bool list_is_empty(List_t *list);//检查链表是否为空
ListItem_t *list_get_head(const List_t *list);//获取链表头部项指针
ListItem_t *list_pop_head(List_t *list);//从链表头部弹出项并返回指针
uint32_t list_count(const List_t *list);//获取链表项数量

#endif // !LIST_H_
