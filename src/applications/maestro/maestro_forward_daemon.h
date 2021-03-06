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
#ifndef MAESTRO_FORWARD_DAEMON_H
#define MAESTRO_FORWARD_DAEMON_H

#include <stdint.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/types.h>

#include <readline/history.h>
#include <readline/readline.h>

#include <common/gru_colors.h>

#include "maestro_cmd_ctxt.h"
#include "maestro_command.h"
#include "maestro_forward_queue.h"
#include "mpt-debug.h"

/**
 * Amount of time to wait if no data is available. It does the same as WORKER_NO_DATA_WAIT
 * but uses a longer wait because throughput is not important for the forward daemon
 */
#define MAESTRO_FWD_DATA_WAIT 500000

#ifdef __cplusplus
extern "C" {
#endif

int maestro_forward_daemon_run(const options_t *options);

#ifdef __cplusplus
}
#endif

#endif /* MAESTRO_FORWARDD_H_DAEMON*/
