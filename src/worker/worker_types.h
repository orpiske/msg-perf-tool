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

#ifndef WORKER_TYPES_H
#define WORKER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time/gru_duration.h>

#include "vmsl.h"
#include "statistics/stats_types.h"
#include "statistics/stats_writer.h"
/**
 * Common return type for the worker
 */
typedef enum worker_ret_t_ {
	WORKER_FAILURE,
	WORKER_SUCCESS,
} worker_ret_t;

/**
 * Represents a snapshot of the current work iteration
 */
typedef struct worker_snapshot_t_ {
	uint64_t count;
	gru_timestamp_t start;
	gru_timestamp_t now;
	stat_latency_t latency;
	stat_throughput_t throughput;
} worker_snapshot_t;

typedef bool(*worker_iteration_check)(const worker_options_t *options, 
	const worker_snapshot_t *snapshot);

/**
 * Abstracts the "operational" parts of the test execution, options, etc.
 */ 
typedef struct worker_t_ {
	const vmsl_t *vmsl;
	worker_options_t *options;
	stats_writer_t *writer;
	worker_iteration_check can_continue;
} worker_t;

#ifdef __cplusplus
}
#endif


#endif /* WORKER_TYPES_H */
