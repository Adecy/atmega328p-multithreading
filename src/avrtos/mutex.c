#include "mutex.h"

#include <avrtos/kernel.h>
#include <avrtos/debug.h>

#include <util/atomic.h>

void k_mutex_init(struct k_mutex *mutex)
{
    mutex->lock = 0xFFu;
    mutex->waitqueue = NULL;
}

uint8_t k_mutex_lock(struct k_mutex *mutex, k_timeout_t timeout)
{
    uint8_t lock;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        lock = _k_mutex_lock(mutex);
        if ((lock != 0) && (timeout.value != K_NO_WAIT.value))
        {
            __K_DBG_MUTEX_WAIT(k_current);

            dlist_queue(&mutex->waitqueue, &k_current->wmutex);

            _k_reschedule(timeout);

            k_yield();

            // TODO use a dlist in order to remove it without condition
            if (TEST_BIT(k_current->flags, K_FLAG_TIMER_EXPIRED))
            {
                dlist_remove(&mutex->waitqueue, &k_current->wmutex);
            }

            lock = _k_mutex_lock(mutex);
        }
    }

    if (lock == 0)
    {
        __K_DBG_MUTEX_LOCKED(k_current);
    }

    return lock;
}

void k_mutex_unlock(struct k_mutex *mutex)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        _k_mutex_unlock(mutex);

        __K_DBG_MUTEX_UNLOCKED(k_current);

        struct ditem* first_waiting_thread = dlist_dequeue(&mutex->waitqueue);
        if (first_waiting_thread != NULL)
        {
            struct k_thread *th = THREAD_OF_QITEM(first_waiting_thread);

            /* immediate: the first thread in the queue 
             * must be the first to get the semaphore */
            _k_immediate_wake_up(th);

            k_yield();
        }
    }
}