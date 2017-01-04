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
#include "sender_perf_main.h"

static void show_help() {
	printf("Usage: ");
	printf("\t-b\t--broker-url=<url> broker-url\n");
	printf("\t-c\t--count=<value> sends a fixed number of messages\n");
	printf("\t-l\t--log-level=<level> runs in the given verbose (info, stat, "
		   "debug, etc) level mode\n");
	printf("\t-p\t--parallel-count=<value> number of parallel connections to the "
		   "server\n");
	printf("\t-d\t--duration=<value> runs for a fixed amount of time (in "
		   "minutes)\n");
	printf("\t-s\t--size=<value> message size (in bytes)\n");
	printf("\t-L\t--logdir=<logdir> a directory to save the logs (mandatory for "
		   "--daemon)\n");
	printf("\t-t\t--throttle=<value> sets a fixed rate of messages (in messages "
		   "per second per connection)\n");
	printf("\t-D\t--daemon run as a daemon in the background\n");
	printf("\t-h\t--help show this help\n");
}

int perf_main(int argc, char **argv) {
	int c = 0;
	int option_index = 0;

	if (argc < 2) {
		show_help();

		return EXIT_FAILURE;
	}

	options_t *options = options_new();

	if (!options) {
		return EXIT_FAILURE;
	}

	set_options_object(options);

	const char *apphome = gru_base_app_home("mpt");
	config_init(options, apphome, "mpt-sender.ini");

	gru_logger_set(gru_logger_default_printer);

	while (1) {

		static struct option long_options[] = {{"broker-url", true, 0, 'b'},
			{"count", true, 0, 'c'}, {"log-level", true, 0, 'l'},
			{"parallel-count", true, 0, 'p'}, {"duration", true, 0, 'd'},
			{"size", true, 0, 's'}, {"logdir", true, 0, 'L'}, {"throttle", true, 0, 't'},
			{"daemon", false, 0, 'D'}, {"help", false, 0, 'h'}, {0, 0, 0, 0}};

		c = getopt_long(argc, argv, "b:c:l:p:d:s:L:t:Dh", long_options, &option_index);
		if (c == -1) {
			if (optind == 1) {
				// Will use defaults from the configuration file
				break;
			}
			break;
		}

		switch (c) {
			case 'b':
				strncpy(options->url, optarg, sizeof(options->url) - 1);
				break;
			case 'c':
				options->count = strtol(optarg, NULL, 10);
				break;
			case 'l':
				options->log_level = gru_logger_get_level(optarg);
				break;
			case 'p':
				options->parallel_count = atoi(optarg);
				break;
			case 'd':
				options->duration = gru_duration_from_minutes(atoi(optarg));
				break;
			case 's':
				options->message_size = atoi(optarg);
				break;
			case 'L':
				strncpy(options->logdir, optarg, sizeof(options->logdir) - 1);
				break;
			case 't':
				options->throttle = atoi(optarg);
				break;
			case 'D':
				options->daemon = true;
				break;
			case 'h':
				show_help();
				return EXIT_SUCCESS;
			default:
				printf("Invalid or missing option\n");
				show_help();
				return EXIT_FAILURE;
		}
	}

	init_controller(options->daemon, options->logdir, "mpt-sender-controller");

	vmsl_t *vmsl = vmsl_init();

	if (!vmsl_assign_by_url(options->url, vmsl)) {
		goto err_exit;
	}

	/**
	 * TODO: fix this
	 */
	int childs[32];
	int child = 0;

	logger_t logger = gru_logger_get();

	if (options->parallel_count > 1) {
		logger(INFO, "Creating %d concurrent operations", options->parallel_count);
		for (uint16_t i = 0; i < options->parallel_count; i++) {
			child = fork();

			if (child == 0) {
				if (strlen(options->logdir) > 0) {
					gru_status_t status = {0};

					bool ret = remap_log(options->logdir, "mpt-sender", getppid(),
						getpid(), stderr, &status);
					if (!ret) {
						fprintf(stderr, "Unable to remap log: %s", status.message);

						goto err_exit;
					}
				}

				sender_start(vmsl, options);
				goto success_exit;
			} else {
				if (child > 0) {
					childs[i] = child;

				} else {
					printf("Error\n");
				}
			}
		}

		if (child > 0) {
			setsid();
			int status = 0;
			for (uint16_t i = 0; i < options->parallel_count; i++) {
				waitpid(childs[i], &status, 0);

				logger(INFO, "Child process %d terminated with status %d", childs[i],
					status);
			}
		}
	} else {
		if (strlen(options->logdir) > 0) {
			gru_status_t status = {0};

			remap_log(options->logdir, "mpt-sender", 0, getpid(), stderr, &status);
		}

		sender_start(vmsl, options);
	}

	logger(INFO, "Test execution with parent ID %d terminated successfully\n", getpid());

success_exit:
	vmsl_destroy(&vmsl);
	options_destroy(&options);
	return EXIT_SUCCESS;

err_exit:
	vmsl_destroy(&vmsl);
	options_destroy(&options);
	return EXIT_FAILURE;
}