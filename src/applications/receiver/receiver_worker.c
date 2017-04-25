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

bool can_start = false;

static void *receiver_handle_start(const maestro_note_t *request, maestro_note_t *response, 
	const maestro_player_info_t *pinfo) 
{
	logger_t logger = gru_logger_get();

	logger(INFO, "Just received a start request");
	can_start = true;
	return NULL;
}

static maestro_sheet_t *new_receiver_sheet(gru_status_t *status) {
	maestro_sheet_t *ret = maestro_sheet_new("/mpt/receiver", status);
	
	if (!ret) {	
		return NULL;
	}

	maestro_instrument_t *instrument = maestro_instrument_new(MAESTRO_NOTE_START, 
		receiver_handle_start, status);

	maestro_sheet_add_instrument(ret, instrument);

	return ret;
}

static void receiver_csv_name(const char *prefix, char *name, size_t len) 
{
	snprintf(name, len - 1, "%s-%d.csv.gz", prefix, getpid());
}

bool receiver_initialize_csv_writer(stats_writer_t *writer, const options_t *options, 
	gru_status_t *status) 
{
	csv_writer_latency_assign(&writer->latency);
	csv_writer_throughput_assign(&writer->throughput);

	char lat_fname[64] = {0};
	receiver_csv_name("receiver-latency", lat_fname, sizeof(lat_fname));

	stat_io_info_t lat_io_info = {0};
	lat_io_info.dest.name = lat_fname;
	lat_io_info.dest.location = (char *) options->logdir;

	if (!writer->latency.initialize(&lat_io_info, status)) {
		return false;
	}

	char tp_fname[64] = {0};
	receiver_csv_name("receiver-throughput", tp_fname, sizeof(tp_fname));

	stat_io_info_t tp_io_info = {0};
	tp_io_info.dest.name = tp_fname;
	tp_io_info.dest.location = (char *) options->logdir;

	if (!writer->throughput.initialize(&tp_io_info, status)) {
		return false;
	}
	
	return true;
}

bool receiver_initialize_out_writer(stats_writer_t *writer, const options_t *options, 
	gru_status_t *status) 
{
	out_writer_latency_assign(&writer->latency);
	out_writer_throughput_assign(&writer->throughput);

	return true;
}


bool receiver_initialize_writer(stats_writer_t *writer, const options_t *options, 
	gru_status_t *status) 
{
	if (options->logdir) {
		return receiver_initialize_csv_writer(writer, options, status);
	}
	
	return receiver_initialize_out_writer(writer, options, status);	
}

void receiver_start(const vmsl_t *vmsl, const options_t *options) {
	logger_t logger = gru_logger_get();
	gru_status_t status = gru_status_new();
	
	
	

	// maestro_sheet_t *sheet = new_receiver_sheet(&status);

	// if (!maestro_player_start(options, sheet, &status)) {
	// 	fprintf(stderr, "Unable to connect to maestro broker: %s\n", 
	// 		status.message);

	// 	return;
	// }

	// while (!can_start) {
	// 	logger(INFO, "Waiting for the start signal");
	// 	sleep(1);
	// }
	/*
	typedef struct worker_t_ {
	const vmsl_t *vmsl;
	const worker_options_t *options;
	const stats_writer_t *writer;
	worker_iteration_check can_continue;
} worker_t;
*/

	worker_t worker = {0};

	worker.vmsl = vmsl;
	worker_options_t wrk_opt = {0};
	worker.options = &wrk_opt;

	worker.options->uri = options->uri; 
	if (options->count == 0) {
		worker.options->duration_type = TEST_TIME;
		worker.options->duration.time = options->duration;
	}
	worker.options->parallel_count = options->parallel_count;
	worker.options->log_level = options->log_level;
	worker.options->message_size = options->message_size;
	worker.options->throttle = options->throttle;

	stats_writer_t writer = {0};
	worker.writer = &writer;
	receiver_initialize_writer(worker.writer, options, &status);

	worker.can_continue = worker_check;
	

	worker_ret_t ret = {0}; 
	worker_snapshot_t snapshot = {0};

	ret = abstract_receiver_worker_start(&worker, &snapshot, &status);
	if (ret != WORKER_SUCCESS) {
		fprintf(stderr, "Unable to execute worker: %s\n", status.message);

		return;
	}

	uint64_t elapsed = gru_time_elapsed_secs(snapshot.start, snapshot.now);

	logger(INFO, 
	 	"Summary: received %" PRIu64 " messages in %" PRIu64
	 	" seconds (rate: %.2f msgs/sec)",
	 	snapshot.count, elapsed, snapshot.throughput.rate);
	
	return;
}
