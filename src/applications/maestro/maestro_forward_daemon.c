/**
 *   Copyright 2017 Otavio Rodolfo Piske
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include "maestro_forward_daemon.h"

static char *locations[] = MAESTRO_CLI_TOPICS;

static void maestro_loop_reply_run(maestro_cmd_ctxt_t *cmd_ctxt,
	int queue,
	gru_status_t *status) {
	logger_t logger = gru_logger_get();

	msg_content_data_t resp = {0};
	msg_content_data_init(&resp, MAESTRO_NOTE_SIZE, status);

	logger(GRU_DEBUG, "Forward loop running and waiting for data");
	do {
		vmsl_stat_t rstat = cmd_ctxt->vmsl.receive(cmd_ctxt->msg_ctxt, &resp, status);
		if (!(rstat & VMSL_SUCCESS)) {
			gru_status_set(status, GRU_FAILURE, "Error receiving maestro message");

			break;
		}

		if (rstat & VMSL_NO_DATA) {
			usleep(MAESTRO_FWD_DATA_WAIT);
			continue;
		}

		mpt_trace("Forwarding: %d %s ", rstat, resp.data);

		int ret = msgsnd(queue, resp.data, resp.size, IPC_NOWAIT);
		if (ret < 0) {
			gru_status_set(status,
				GRU_FAILURE,
				"Unable to send message through the local forward queue");
		}

		msg_content_data_reset(&resp);
	} while (true);
	logger(GRU_DEBUG, "Finalizing loop");
}

static void maestro_loop_reply(const options_t *options, gru_status_t *status) {
	logger_t logger = gru_logger_get();
	gru_uri_t uri = gru_uri_clone(options_get_maestro_uri(), status);
	if (!gru_status_success(status)) {
		return;
	}

	logger(GRU_DEBUG, "Initializing command context for: %s", options_get_maestro_host());

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_RECEIVER,
		.statistics = MSG_STAT_NONE,
	};

	logger(GRU_DEBUG, "Generating unique ID");
	msg_conn_info_gen_id(&opt.conn_info);

	gru_uri_set_path(&uri, MAESTRO_TOPIC);
	opt.uri = uri;

	logger(GRU_DEBUG, "Initializing command context for: %s", uri.host);
	maestro_cmd_ctxt_t *cmd_ctxt = maestro_cmd_ctxt_new(&uri, status);
	if (!cmd_ctxt) {
		gru_status_set(status, GRU_FAILURE, "Unable to initialize command context");

		return;
	}

	if (!maestro_cmd_ctxt_start(cmd_ctxt, opt, status)) {
		return;
	}

	vmsl_mtopic_spec_t location = {0};

	location.count = MAESTRO_CLI_COUNT;
	location.topics = locations;
	location.qos = MAESTRO_DEFAULT_QOS;

	if (!maestro_cmd_ctxt_forwarder(cmd_ctxt, &location, status)) {
		return;
	}

	logger(GRU_DEBUG, "Starting main forward loop");
	maestro_loop_reply_run(cmd_ctxt, cmd_ctxt->queue, status);
	if (!gru_status_success(status)) {
		logger(GRU_ERROR, "Error while running the forward loop: %s", status->message);
	}

	logger(GRU_DEBUG, "Finalizing forward daemon");
	maestro_cmd_ctxt_destroy(&cmd_ctxt);
}

int maestro_forward_daemon_run(const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();

	logger(GRU_DEBUG, "Initializing reply loop");
	maestro_loop_reply(options, &status);
	if (!gru_status_success(&status)) {
		logger(GRU_ERROR, "Failed to initialize reply loop: %s", status.message);
		return 1;
	}

	return 0;
}