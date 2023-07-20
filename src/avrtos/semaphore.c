/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "debug.h"
#include "kernel.h"
#include "kernel_internals.h"
#include "semaphore.h"

#include <util/atomic.h>

int8_t k_sem_init(struct k_sem *sem, uint8_t initial_count, uint8_t limit)
{
#if CONFIG_KERNEL_ARGS_CHECKS
	if (!sem || !limit) {
		return -EINVAL;
	}
#endif

	sem->limit = limit;
	sem->count = MIN(limit, initial_count);
	Z_STATS_SEM_CLEAR(sem);
	dlist_init(&sem->waitqueue);

	return 0;
}

int8_t k_sem_take(struct k_sem *sem, k_timeout_t timeout)
{
	int8_t get = 0x00;

	const uint8_t key = irq_lock();

	if (sem->count != 0) {
		sem->count--;
	} else {
		Z_STATS_SEM_PEND(sem);
		get = z_pend_current(&sem->waitqueue, timeout);
	}

	irq_unlock(key);

	Z_STATS_SEM_TAKE(sem);

	if (get == 0) {
		__K_DBG_SEM_TAKE(z_current);
	}

	return get;
}

struct k_thread *k_sem_give(struct k_sem *sem)
{
	struct k_thread *thread;

	const uint8_t key = irq_lock();

	__K_DBG_SEM_GIVE(z_current);

	/* if a thread is thread is pending on a semaphore
	 * it means that its count is necessary 0. So we don't
	 * need to check if we reached the limit.
	 */

	thread = z_unpend_first_thread(&sem->waitqueue);

	/* If there is a thread pending on a semaphore,
	 * we to give the semaphore directly to the thread
	 */
	if (thread == NULL) {
		if (sem->count != sem->limit) {
			sem->count++;
		}
	}

	irq_unlock(key);

	Z_STATS_SEM_GIVE(sem);

	return thread;
}