/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: TIoT list. \n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-08-03, Create file. \n
 */
#ifndef TIOT_LIST_H
#define TIOT_LIST_H

#include "tiot_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @defgroup tiot_common_util_tiot_list List
 * @ingroup  tiot_common_util
 * @{
 */

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct tiot_list_head {
    struct tiot_list_head *next, *prev;
};

#define tiot_list_head_init(name) \
    {                             \
        &(name), &(name)          \
    }

#define tiot_list_head_create(name) \
    struct tiot_list_head name = tiot_list_head_init(name)

static inline void tiot_init_list_head(struct tiot_list_head *list)
{
    if (list == NULL) {
        return;
    }
    list->next = list;
    list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void tiot___list_add(struct tiot_list_head *_new,
    struct tiot_list_head *prev,
    struct tiot_list_head *next)
{
    if (_new == NULL || prev == NULL || next == NULL) {
        return;
    }
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void tiot_list_add_tail(struct tiot_list_head *cur, struct tiot_list_head *head)
{
    tiot___list_add(cur, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void tiot___list_del(struct tiot_list_head *prev, struct tiot_list_head *next)
{
    if (prev == NULL || next == NULL) {
        return;
    }

    next->prev = prev;
    prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void tiot___list_del_entry(struct tiot_list_head *entry)
{
    if (entry == NULL) {
        return;
    }
    tiot___list_del(entry->prev, entry->next);
}

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void tiot_list_replace(struct tiot_list_head *old,
    struct tiot_list_head *_new)
{
    _new->next = old->next;
    _new->next->prev = _new;
    _new->prev = old->prev;
    _new->prev->next = _new;
}

/**
 * list_entry - get the struct for this entry
 * @ptr:    the &struct list_head pointer.
 * @type:    the type of the struct this is embedded in.
 * @member:    the name of the list_struct within the struct.
 */
#define tiot_list_entry(ptr, type, member) \
    tiot_container_of(ptr, type, member)

/**
 * list_for_each_entry    -    iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:  the name of the list_struct within the struct.
 * @pos_type:  type of the struct which the list_node in.
 */
#define tiot_list_for_each_entry(pos, head, member, pos_type)            \
    for ((pos) = tiot_list_entry((head)->next, pos_type, member);        \
         &(pos)->member != (head);                                         \
         (pos) = tiot_list_entry((pos)->member.next, pos_type, member))

/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:    the type * to use as a loop cursor.
 * @n:        another type * to use as temporary storage
 * @head:    the head for your list.
 * @member:    the name of the list_struct within the struct.
 * @pos_type:  type of pos.
 * @n_type:    type of n.
 */
#define tiot_list_for_each_entry_safe(pos, n, head, member, pos_type, n_type)         \
    for ((pos) = tiot_list_entry((head)->next, pos_type, member),                     \
        (n) = tiot_list_entry((pos)->member.next, pos_type, member);                  \
         &(pos)->member != (head);                                                      \
         (pos) = (n), (n) = tiot_list_entry((n)->member.next, n_type, member))

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif