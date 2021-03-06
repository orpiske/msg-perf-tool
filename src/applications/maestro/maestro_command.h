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
#ifndef MAESTRO_COMMAND_H
#define MAESTRO_COMMAND_H

#include <stdio.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "msgctxt.h"
#include "vmsl.h"
#include "vmsl_assign.h"

#include "maestro_cmd_ctxt.h"

#include "maestro/maestro_debug.h"
#include "maestro/maestro_easy.h"
#include "maestro/maestro_note.h"
#include "maestro/maestro_serialize.h"
#include "maestro/maestro_deserialize.h"
#include "maestro/maestro_topics.h"


#ifdef __cplusplus
extern "C" {
#endif

int maestro_cmd_start_receiver(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);
int maestro_cmd_stop_receiver(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);
int maestro_cmd_start_sender(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);
int maestro_cmd_stop_sender(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);
int maestro_cmd_start_inspector(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);
int maestro_cmd_stop_inspector(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);

int maestro_cmd_start_all(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);
int maestro_cmd_stop_all(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);

int maestro_cmd_collect(maestro_cmd_ctxt_t *cmd_ctxt, gru_list_t *strings,
	gru_status_t *status);
int maestro_cmd_flush(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);
int maestro_cmd_set_opt(maestro_cmd_ctxt_t *cmd_ctxt,
	gru_list_t *strings,
	gru_status_t *status);
int maestro_cmd_ping(maestro_cmd_ctxt_t *cmd_ctxt, gru_status_t *status);
int maestro_cmd_stats(maestro_cmd_ctxt_t *cmd_ctxt, gru_list_t *strings,
	gru_status_t *status);
int maestro_cmd_halt(maestro_cmd_ctxt_t *cmd_ctxt, gru_list_t *strings,
	gru_status_t *status);

#ifdef __cplusplus
}
#endif

#endif /* MAESTRO_COMMAND_H */
