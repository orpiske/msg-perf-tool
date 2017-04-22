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
#ifndef RECEIVERD_WORKER_H
#define RECEIVERD_WORKER_H

#include <inttypes.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include <common/gru_status.h>

#include "contrib/options.h"
#include "msgctxt.h"
#include "process_utils.h"
#include "vmsl.h"
#include "msg_content_data.h"
#include "maestro/maestro_player.h"
#include "maestro/maestro_sheet.h"

typedef enum test_duration_type_t_ {
	MESSAGE_COUNT, 
	TEST_TIME,
} test_duration_type_t;

typedef struct worker_options_t_ {
	gru_uri_t uri;

	test_duration_type_t duration_type;
	union { 
		uint64_t count;
		gru_duration_t time;
	} duration;

	log_level_t log_level;
	uint16_t parallel_count;
	size_t message_size;
	uint32_t throttle;
} worker_options_t;


int receiverd_worker_start(const options_t *options);

#endif /* RECEIVERD_WORKER_H */