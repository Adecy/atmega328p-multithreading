/*___________________________________________________________________________*/

#include <avr/io.h>

#include "avrtos/misc/uart.h"

#include "avrtos/dstruct/dlist.h"
#include "avrtos/dstruct/queue.h"
#include "avrtos/dstruct/tqueue.h"
#include "avrtos/dstruct/debug.h"

#include "avrtos/defines.h"

/*___________________________________________________________________________*/

void test_queue(void);
void test_ref_dlist(void);
void test_unreft_dlist(void);
void test_tqueue(void);
void test_scheduler(void);
void test_mutex(void);

/*___________________________________________________________________________*/

int main(void)
{
  usart_init();
  
  usart_printl("queue");
  test_queue();

  usart_printl("ref dlist");
  test_ref_dlist();

  usart_printl("unret dlist");
  test_unreft_dlist();

  usart_printl("tqueue");
  test_tqueue();

  usart_printl("scheduler");
  test_scheduler();

  usart_printl("mutex");
  test_mutex();
}

/*___________________________________________________________________________*/

//
// QUEUE
//

struct item
{
  const char name;
  struct qitem i;
};

#define ITEM(name) {name, {0}}
#define ITEM_OF(qitem) ((struct item*) CONTAINER_OF(qitem, struct item, i))
#define NAME_OF(qitem) ITEM_OF(qitem)->name


struct item items[] = {
  ITEM('A'), ITEM('B'), ITEM('C'), ITEM('D'), ITEM('E')
};

DEFINE_QUEUE(myqueue);

inline void queue(struct item *item) { return queue(&myqueue, &item->i); }
inline struct item *dequeue(void)
{
  struct qitem *item = dequeue(&myqueue);
  if (item == NULL)
  {
    return NULL;
  }
  else
  {
    return ITEM_OF(item);
  }
}
void print_queue_item(struct qitem *item) { usart_transmit(NAME_OF(item)); }
inline void print_queue(void) { print_queue(myqueue, print_queue_item); }

void test_queue(void)
{
  queue(&items[0]);
  queue(&items[1]);
  queue(&items[2]);

  struct item * dequeued = NULL;

  print_queue();
  dequeued = dequeue();
  print_queue();
  usart_print("queue ");
  print_queue_item(&dequeued->i);
  usart_transmit('\n');
  queue(dequeued);
  print_queue();
  dequeued = dequeue();
  print_queue();
  dequeued = dequeue();
  print_queue();
  usart_print("queue ");
  print_queue_item(&dequeued->i);
  usart_transmit('\n');
  queue(dequeued);
  print_queue();
  dequeued = dequeue();
  print_queue();
}

/*___________________________________________________________________________*/

//
// DLIST
//

struct item2
{
  const char name;
  struct ditem i;
};

#define DITEM(name) {name, {0, 0}}
#define DITEM_OF(item) ((struct item2*) CONTAINER_OF(item, struct item2, i))
#define DNAME_OF(item) DITEM_OF(item)->name
#define DEFINE_DLIST_ITEM(dlist_item) struct item2 dlist_item = {'*', {&dlist_item.i, &dlist_item.i} }

struct item2 ditems[] = {
  DITEM('A'), DITEM('B'), DITEM('C'), DITEM('D'), DITEM('E')
};

DEFINE_DLIST_ITEM(dlist);

void print_dlist_item(struct ditem *item) { usart_transmit(DNAME_OF(item)); }
inline void print_dlist(void) { print_dlist(&dlist.i, print_dlist_item); }
inline void dlist_queue(struct item2 *item) { return dlist_queue(&dlist.i, &item->i); }
inline struct item2 *dlist_dequeue(void)
{
  struct ditem *item = dlist_dequeue(&dlist.i);
  if (item == &dlist.i)
  {
    return NULL;
  }
  else
  {
    return DITEM_OF(item);
  }
}

void test_ref_dlist(void)
{
  print_dlist();

  dlist_queue(&ditems[0]);
  dlist_queue(&ditems[1]);
  dlist_queue(&ditems[2]);
  
  print_dlist();

  push_back(&dlist.i, &ditems[3].i);
  
  print_dlist();

  struct item2 * item = dlist_dequeue();

  print_dlist();

  item = dlist_dequeue();

  print_dlist();

  push_front(&dlist.i, &item->i);

  print_dlist();
}

void test_unreft_dlist(void)
{
  struct ditem *ref = &ditems[0].i;

  dlist_ref(ref);

  print_dlist(ref, print_dlist_item);

  dlist_queue(ref, &ditems[1].i);
  dlist_queue(ref, &ditems[2].i);
  dlist_queue(ref, &ditems[3].i);
  
  print_dlist(ref, print_dlist_item);
  pop_ref(&ref);
  print_dlist(ref, print_dlist_item);
  pop_ref(&ref);
  print_dlist(ref, print_dlist_item);
  pop_ref(&ref);
  print_dlist(ref, print_dlist_item);
  pop_ref(&ref); // cannot have 0 elements in the list
  print_dlist(ref, print_dlist_item);
}

/*___________________________________________________________________________*/

//
// TQUEUE
//

struct item3 {
    const char chr;
    struct titem tie;
};

struct item3 titems[] = {
    {'A', {100}},
    {'B', {25}},
    {'C', {35}},
    {'D', {35}},
    {'E', {25}}
};

void print_titem(struct titem *item)
{
    struct item3 *i = CONTAINER_OF(item, struct item3, tie);
    usart_transmit(i->chr);
    usart_transmit('[');
    usart_u16(i->tie.delay_shift);
    usart_transmit(']');
}

void print_tqueue(struct titem *root)
{
  print_tqueue(root, print_titem);
}

void test_tqueue(void)
{
    struct titem *root = NULL;

    for (uint8_t i = 0; i < 5; i++)
    {
        _tqueue_schedule(&root, &titems[4 - i].tie);
    }   

    print_tqueue(root);

    tqueue_shift(&root, 20);

    print_tqueue(root);

    tqueue_shift(&root, 20);

    print_tqueue(root);

    struct titem *pop = tqueue_pop(&root);
    print_titem(pop);
    usart_transmit('\n');
    pop = tqueue_pop(&root);
    print_titem(pop);
    usart_transmit('\n');

    tqueue_shift(&root, 20);

    print_tqueue(root);

    tqueue_remove(&root, &titems[2].tie);

    print_tqueue(root);

    tqueue_remove(&root, &titems[2].tie);

    print_tqueue(root);

    tqueue_remove(&root, &titems[0].tie);

    print_tqueue(root);

    tqueue_remove(&root, &titems[3].tie);

    print_tqueue(root);
}

/*___________________________________________________________________________*/

struct item2 threads[] = {
  DITEM('M'), DITEM('A'), DITEM('B'), 
};

struct ditem *runqueue2;

void test_scheduler(void)
{
  runqueue2 = &threads[0].i;
  dlist_ref(runqueue2);

  dlist_queue(runqueue2, &threads[1].i);
  dlist_queue(runqueue2, &threads[2].i);

  print_dlist(runqueue2, print_dlist_item);

  usart_printl("==== POPREF ====");

  struct ditem * ref = pop_ref(&runqueue2);

  usart_print("pop_ref(&runqueue2) = ");
  print_dlist_item(ref);
  usart_transmit('\n');

  print_dlist(runqueue2, print_dlist_item);

/*
  usart_printl("==== PUSHBACK ====");

  push_back(runqueue2, ref);

  struct ditem * next = runqueue2;

  usart_print("next = runqueue2 ");
  print_dlist_item(next);
  usart_transmit('\n');

  print_dlist(runqueue2, print_dlist_item);
*/
  usart_printl("==== PUSHREF ====");

  struct ditem * next = push_ref(&runqueue2, ref);

  usart_print("push_ref(&runqueue2, ref) = ");
  print_dlist_item(next);
  usart_transmit('\n');

  print_dlist(runqueue2, print_dlist_item);

  usart_printl("==== REFREQUEUETOP ====");

  next = ref_requeue(&runqueue2);

  usart_print("ref_requeue(&runqueue2) = ");
  print_dlist_item(next);
  usart_transmit('\n');

  print_dlist(runqueue2, print_dlist_item);
}

/*___________________________________________________________________________*/

struct qitem *mutex_ref = NULL;

struct item wthreads[] = {
  ITEM('M'), ITEM('A'), ITEM('B'), 
};

void test_mutex(void)
{
  queue(&mutex_ref, &wthreads[0].i);

  print_queue(mutex_ref, print_queue_item);  
}

/*___________________________________________________________________________*/