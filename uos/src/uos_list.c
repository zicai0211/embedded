#include "uos_list.h"

/**
 * @brief Initialize a list 初始化整个链表结构，包括哨兵节点和计数器
 * @param list Pointer to the list to be initialized
 *
 */
void list_init(List_t *list)
{
    list->list_end.next = &list->list_end;
    list->list_end.prev = &list->list_end;

    list->list_end.owner = NULL;
    list->list_end.item_value = 0xFFFFFFFFU;
    list->list_end.container = list;
    list->item_count = 0;
}
/*
    初始化单个节点
*/
void list_item_init(ListItem_t *list)
{
    list->container = NULL;
    list->owner = NULL;
    list->item_value = 0;
    list->next = NULL;
    list->prev = NULL;

}
static void list_insert_between(ListItem_t *prev, ListItem_t *next,
                ListItem_t *item)
{
    item->next = next;
    item->prev = prev;

    prev->next = item;
    next->prev = item;
}

void list_insert_tail(List_t *list, ListItem_t *item)
{
    ListItem_t *tail;
    if(list == NULL || item == NULL || item->container != NULL)
    {
        return;
    }

    tail = list->list_end.prev;
    list_insert_between(tail, &list->list_end, item);
    list->item_count++;
    item->container = list;
}

void list_insert_head(List_t *list, ListItem_t *item)
{
    if (list == NULL || item == NULL || item->container != NULL)
    {
        return;
    }

    list_insert_between(&list->list_end, list->list_end.next, item);

    item->container = list;
    list->item_count++;
}
// 按照item_value值从大到小插入链表
// 从头开始找
// 找到第一个 item_value 大于新节点的位置
// 把新节点插到它前面
void list_insert_ordered(List_t *list, ListItem_t *item)
{
    ListItem_t *iter;
    if (list == NULL || item == NULL || item->container != NULL)
    {
        return;
    }
    // 从链表头部开始遍历，找到第一个item_value大于新项的节点
    iter = list->list_end.next;
    while (iter != &list->list_end && iter->item_value <= item->item_value)
    {
        iter = iter->next;
    }

    list_insert_between(iter->prev, iter, item);
    list->item_count++;
    item->container = list;
}

void list_remove(ListItem_t *item)
{
    List_t *list;
    if(item == NULL || item->container == NULL)
    {
        return;
    }
    list = item->container;
    item->prev->next = item->next;
    item->next->prev = item->prev;

    item->container = NULL;
    item->next = NULL;
    item->prev = NULL;

    if(list->item_count > 0)
    {
        list->item_count--;
    }

}
bool list_is_empty(List_t *list)
{
    if(list == NULL)
    {
        return true;
    }
    return list->item_count == 0;
    //or return list->list_end->next == &list->list_end;
}

ListItem_t *list_get_head(const List_t *list)
{
    if(list == NULL || list->item_count == 0)
    {
        return NULL;
    }
    return list->list_end.next;
}

ListItem_t *list_pop_head(List_t *list)
{
    if(list == NULL || list->item_count == 0)
    {
        return NULL;
    }

    ListItem_t *head =  list_get_head(list);
    if(head != NULL)
    {
        list_remove(head);
    }
    return head;
}
uint32_t list_count(const List_t *list)
{
    if (list == NULL)
    {
        return 0;
    }
    return list->item_count;
}
