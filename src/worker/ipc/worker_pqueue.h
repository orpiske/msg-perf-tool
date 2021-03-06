/**
 *    Copyright 2017 Otavio Rodolfo Piske
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#ifndef MPT_WORKER_PQUEUE_H
#define MPT_WORKER_PQUEUE_H

#include <fcntl.h>
#include <sys/stat.h>
#ifdef HAVE_MQUEUE_H
 #include <mqueue.h>
#endif

#include <common/gru_status.h>
#include <common/gru_alloc.h>
#include <log/gru_logger.h>

#include "worker_queue_perm.h"
#include "worker_queue_stat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct worker_pqueue_t_ worker_pqueue_t;

worker_pqueue_t *worker_pqueue_new(const char *name, queue_perm_t perm, long msg_size, gru_status_t *status);
void worker_pqueue_destroy(worker_pqueue_t **ptr);
worker_queue_stat_t worker_pqueue_write(const worker_pqueue_t * const worker_pqueue, const void *data, size_t len, void *payload);
worker_queue_stat_t worker_pqueue_read(const worker_pqueue_t * const worker_pqueue, void *dest, size_t len);

#ifdef __cplusplus
}
#endif

#endif //MPT_WORKER_PQUEUE_H
