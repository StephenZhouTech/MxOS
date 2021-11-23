#ifndef __MXOS_LIST_H__
#define __MXOS_LIST_H__

#include "os_types.h"

typedef struct ListHead {
    struct ListHead *next, *prev;
} ListHead_t;

#define LIST_INVALID_POS            0xdeadbeef

#define OffsetOf(TYPE, MEMBER) ((OS_Uint32_t) &((TYPE *)0)->MEMBER)
/**
 * ContainerOf - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define ContainerOf(ptr, type, member) ( (type *)((OS_Uint8_t *)(ptr) - OffsetOf(type, member)) )

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    ListHead_t name = LIST_HEAD_INIT(name)

/**
 * ListHeadInit - Initialize a ListHead structure
 * @list: ListHead structure to be initialized.
 *
 * Initializes the ListHead to point to itself.  If it is a list header,
 * the result is an empty list.
 */
static inline void ListHeadInit(ListHead_t *list)
{
    list->next = list;
    list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __ListAdd(ListHead_t *new,
                  ListHead_t *prev,
                  ListHead_t *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/**
 * ListAdd - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void ListAdd(ListHead_t *new, ListHead_t *head)
{
    __ListAdd(new, head, head->next);
}

/**
 * ListAddTail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void ListAddTail(ListHead_t *new, ListHead_t *head)
{
    __ListAdd(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __ListDel(ListHead_t * prev, ListHead_t * next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * ListDel - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: ListEmpty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void ListDel(ListHead_t *entry)
{
    __ListDel(entry->prev, entry->next);
    entry->next = (ListHead_t *)LIST_INVALID_POS;
    entry->prev = (ListHead_t *)LIST_INVALID_POS;
}

/**
 * ListReplace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void ListReplace(ListHead_t *old,
                ListHead_t *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

/**
 * ListMove - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void ListMove(ListHead_t *list, ListHead_t *head)
{
    __ListDel(list->prev, list->next);
    ListAdd(list, head);
}

/**
 * ListMoveTail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void ListMoveTail(ListHead_t *list,
                  ListHead_t *head)
{
    __ListDel(list->prev, list->next);
    ListAddTail(list, head);
}

/**
 * ListIsFirst -- tests whether @list is the first entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int ListIsFirst(const ListHead_t *list,
                    const ListHead_t *head)
{
    return list->prev == head;
}

/**
 * ListIsLast - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int ListIsLast(const ListHead_t *list,
                const ListHead_t *head)
{
    return list->next == head;
}

/**
 * ListEmpty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int ListEmpty(const ListHead_t *head)
{
    return head->next == head;
}


/**
 * ListSwap - replace entry1 with entry2 and re-add entry1 at entry2's position
 * @entry1: the location to place entry2
 * @entry2: the location to place entry1
 */
static inline void ListSwap(ListHead_t *entry1,
                 ListHead_t *entry2)
{
    ListHead_t *pos = entry2->prev;

    ListDel(entry2);
    ListReplace(entry1, entry2);
    if (pos == entry1)
        pos = entry2;
    ListAdd(entry1, pos);
}

static inline void __ListSplice(const ListHead_t *list,
                 ListHead_t *prev,
                 ListHead_t *next)
{
    ListHead_t *first = list->next;
    ListHead_t *last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}

/**
 * ListSplice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void ListSplice(const ListHead_t *list,
                ListHead_t *head)
{
    if (!ListEmpty(list))
        __ListSplice(list, head, head->next);
}

/**
 * ListSpliceTail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void ListSpliceTail(ListHead_t *list,
                ListHead_t *head)
{
    if (!ListEmpty(list))
        __ListSplice(list, head->prev, head);
}

/**
 * ListForEach  -   iterate over a list
 * @pos:    the &ListHead_t to use as a loop cursor.
 * @head:   the head for your list.
 */
#define ListForEach(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)


/**
 * ListEntry - get the struct for this entry
 * @ptr:    the &ListHead_t pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the ListHead within the struct.
 */
#define ListEntry(ptr, type, member) \
    ContainerOf(ptr, type, member)

/**
 * ListFirstEntry - get the first element from a list
 * @ptr:    the list head to take the element from.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the ListHead within the struct.
 */
#define ListFirstEntry(ptr, type, member) \
    ListEntry((ptr)->next, type, member)

#endif // __MXOS_LIST_H__
