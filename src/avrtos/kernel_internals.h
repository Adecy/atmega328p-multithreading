/*
 * Copyright (c) 2022 Lucas Dietrich <ld.adecy@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _AVRTOS_KERNEL_INTERNALS_H
#define _AVRTOS_KERNEL_INTERNALS_H

#include "kernel.h"

/**
 * @brief Make the current thread waiting/pending for an object being available.
 *
 * Suspend the thread and add it to the waitqueue.
 * The function will return if the thread is awakaned or on timeout.
 *
 * If timeout is K_FOREVER, the thread should we awakaned.
 * If timeout is K_NO_WAIT, the thread returns immediately
 *
 * Assumptions :
 *  - interrupt flag is cleared when called.
 *
 * @param waitqueue
 * @param timeout
 * @return 0 on success (object available), ETIMEDOUT on timeout, negative error
 *  in other cases.
 */
__kernel int8_t z_pend_current(struct dnode *waitqueue, k_timeout_t timeout);

/**
 * @brief Wake up the first thread pending on an object.
 * Switch thread before returning.
 *
 * Assumptions :
 * - interrupt flag is cleared when called
 * - waitqueue is not null
 * - Thread in the runqueue is suspended
 *
 * @param waitqueue
 * @param swap_data : available object information
 * @return uint8_t return 0 if a thread got the object, any other value
 * otherwise
 */
__kernel struct k_thread *z_unpend_first_thread(struct dnode *waitqueue);

/**
 * @brief Wake up the first thread pending on an object.
 * Set the first pending thread swap_data parameter if set.
 * Switch thread before returning.
 *
 * @see z_unpend_first_thread
 *
 * Assumptions :
 * - interrupt flag is cleared when called
 * - waitqueue is not null
 * - Thread in the runqueue is suspended
 *
 * @param waitqueue
 * @param set_swap_data
 * @return thread pointer if a thread got the object, NULL otherwise
 */
__kernel struct k_thread *z_unpend_first_and_swap(struct dnode *waitqueue,
						    void *set_swap_data);

/**
 * @brief Cancel the first pending thread on a waitqueue.
 * 
 * Raise "pend_canceled" flag on the thread, any wait function will return
 * with -ECANCELED.
 * 
 * @param waitqueue  
 */
__kernel void z_cancel_first_pending(struct dnode *waitqueue);

/**
 * @brief Cancel all pending threads on a waitqueue.
 * 
 * Raise "pend_canceled" flag on the thread, any wait function will return
 * with -ECANCELED.
 * 
 * @param waitqueue 
 * @return Number of canceled threads 
 */
__kernel uint8_t z_cancel_all_pending(struct dnode *waitqueue);

/**
 * @brief Wake up a thread.
 * 
 * Cancel scheduled wake up if any and add the thread to the runqueue.
 * 
 * @param thread  
 */
__kernel void z_wake_up(struct k_thread *thread);

#endif /* _AVRTOS_KERNEL_INTERNALS_H */