/**
 Copyright 2016 Otavio Rodolfo Piske

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */
#include "receiver_worker.h"
#include "vmsl.h"

void receiver_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	const uint32_t tp_interval = 10;
	uint64_t last_count = 0;
	msg_content_data_t content_storage = {0}; 

	if (!maestro_player_start(options, &status)) {
		fprintf(stderr, "Unable to connect to maestro broker: %s\n", 
			status.message);

		return;
	}

	stat_io_t *stat_io = statistics_init(RECEIVER, &status);
	if (!stat_io) {
		logger(FATAL, "Unable to initialize statistics engine: %s", status.message);
		gru_status_reset(&status);

		return;
	}

	msg_opt_t opt = {
		.direction = MSG_DIRECTION_RECEIVER, 
		.qos = MSG_QOS_AT_MOST_ONCE, 
		.statistics = MSG_STAT_DEFAULT,
	};

	opt.uri = options->uri;

	msg_ctxt_t *msg_ctxt = vmsl->init(stat_io, opt, NULL, &status);
	if (!msg_ctxt) {
		goto err_exit;
	}

	vmsl_stat_t ret = vmsl->subscribe(msg_ctxt, NULL, &status);
	if (vmsl_stat_error(ret)) {
		goto err_exit;
	}

	msg_content_data_init(&content_storage, options->message_size, &status);
	if (!gru_status_success(&status)) {
		goto err_exit;
	}

	gru_timestamp_t last;
	gru_timestamp_t start = gru_time_now();
	time_t last_calc = 0;

	statistics_latency_header(stat_io);
	statistics_throughput_header(stat_io);

	// install_timer(30);
	install_interrupt_handler();

	while (can_continue(options, 0)) {
		vmsl_stat_t rstat = vmsl->receive(msg_ctxt, &content_storage, &status);
		if (unlikely(vmsl_stat_error(rstat))) {
			logger(ERROR, "Error receiving data: %s\n", status.message);

			gru_status_reset(&status);
			break;
		}

		last = gru_time_now();

		if (last_calc <= (last.tv_sec - tp_interval)) {
			uint64_t processed_count = content_storage.count - last_count;

			statistics_throughput_partial(stat_io, last, tp_interval, processed_count);
			
			last_count = content_storage.count;
			last_calc = last.tv_sec;
		}
	}

	vmsl->stop(msg_ctxt, &status);
	vmsl->destroy(msg_ctxt, &status);

	statistics_destroy(&stat_io);

	uint64_t elapsed = statistics_diff(start, last);
	double rate = ((double) content_storage.count / (double) elapsed) * 1000;

	uint64_t total_received = content_storage.count;

	logger(STAT,
		"summary;received;%" PRIu64 ";elapsed;%" PRIu64 ";rate;%.2f",
		total_received,
		elapsed,
		rate);

	logger(INFO,
		"Summary: received %" PRIu64 " messages in %" PRIu64
		" milliseconds (rate: %.2f msgs/sec)",
		total_received,
		elapsed,
		rate);
	logger(INFO, "Errors: received %" PRIu64, content_storage.errors);

	msg_content_data_release(&content_storage);
	return;

err_exit:
	fprintf(stderr, "%s", status.message);
	statistics_destroy(&stat_io);
	msg_content_data_release(&content_storage);

	if (msg_ctxt) {
		vmsl->destroy(msg_ctxt, &status);
	}

	gru_status_reset(&status);
	return;
}