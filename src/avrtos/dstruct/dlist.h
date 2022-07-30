#ifndef _CLIST_H
#define _CLIST_H

#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/*___________________________________________________________________________*/

/**
 * @brief Doubly linked list data structure 
 * - O(1) queue/dequeue/insertion/deletion complexity.
 * 
 * - n : number of items in the list
 * dlist_count is O(n)
 */

/**
 * - a ditem is already in the list if next != null and prev != null
 * - a ditem is not in the list if next == null && prev == null
 * - poping the last element of the runqueue doesn't have no any effect
 */

/*___________________________________________________________________________*/

struct ditem
{
        union
        {
                struct ditem *next;
                struct ditem *head;
        };
        union
        {
                struct ditem *prev;
                struct ditem *tail;
        };
};

typedef struct ditem ditem_t;
typedef struct ditem dlist_t;

#define DITEM_INIT(self)  \
    {                     \
        {                 \
            .next = self, \
        },                \
        {                 \
            .prev = self, \
        }                 \
    }

#define DLIST_INIT(_list)   \
    {                       \
        {                   \
            .head = &_list, \
        },                  \
        {                   \
            .tail = &_list, \
        }                   \
    }

#define DITEM_INIT_NULL() DITEM_INIT(NULL)

#define DEFINE_DLIST(list_name) struct ditem list_name = DLIST_INIT(list_name)

#define DLIST_EMPTY(_list) (_list->head == _list)

#define DITEM_VALID(_list, _node) (_list != _node)
#define DITEM_NEXT(_list, _node) (_node->next)
#define DITEM_PREV(_list, _node) (_node->prev)

/*___________________________________________________________________________*/

#define DLIST_FOREACH(_list, _node) \
    for (_node = _list->head; _node != _list; _node = _node->next)

#define DLIST_FOREACH_SAFE(_list, _tmp, _node) \
    for (_node = _list->head, _tmp = _list->head; \
    	 _node != _list; _node = _node->next, _tmp = _node)

/*___________________________________________________________________________*/

/**
 * @brief Initialize a doubly linked list
 * 
 * @param list 
 */
void dlist_init(struct ditem *list);

/**
 * @brief Add (queue) a node to the end of a doubly linked list
 * 
 * @param list 
 * @param node 
 */
void dlist_append(struct ditem *list, struct ditem *node);

/**
 * @brief Add (queue) a node to the beginning of a doubly linked list
 * 
 * @param list 
 * @param node 
 */
void dlist_prepend(struct ditem *list, struct ditem *node);

/**
 * @brief Insert a node before a given node
 * 
 * @param successor 
 * @param node 
 */
void dlist_insert(struct ditem *successor, struct ditem *node);

/**
 * @brief Remove a node from a doubly linked list
 * 
 * @param node 
 */
void dlist_remove(struct ditem *node);

/**
 * @brief Get the first node of a doubly linked list (dequeue)
 * 
 * @param list 
 * @return struct ditem* 
 */
struct ditem *dlist_get(struct ditem *list);

/**
 * @brief Check if a doubly linked list is empty
 * 
 * @param list 
 * @return true 
 * @return false 
 */
bool dlist_is_empty(struct ditem *list);

/**
 * @brief Calculate the number of items in a doubly linked list
 * 
 * @param list 
 * @return uint8_t 
 */
uint8_t dlist_count(struct ditem *list);

/*___________________________________________________________________________*/

#ifdef __cplusplus
}
#endif

#endif