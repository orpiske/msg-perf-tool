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
#include "maestro_loop.h"


int maestro_loop(gru_status_t *status) {
	char *command = NULL;

	const options_t *options = get_options_object();

	// int create_foward_queue(gru_status_t *status);
	int queue = create_foward_queue(status);
	if (queue < 0) {
		fprintf(stderr, "Unable to initialize forward queue: %s\n", status->message);
		
		return 1;
	}

	maestro_cmd_ctxt_t *cmd_ctxt = maestro_cmd_ctxt_init(&options->maestro_uri, status);
	if (!cmd_ctxt) {
		fprintf(stderr, "Unable to initialize command processor: %s\n", status->message);
		
		return 1;
	}
	
	do  { 
		command = readline(RED "maestro" LIGHT_WHITE "> " RESET);
		if (command == NULL) {
			break;
		}

		add_history(command);
		if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
			break;
		}

		if (strcmp(command, "start-receiver") == 0) {
			maestro_cmd_start_receiver(cmd_ctxt, status);
		}

		if (strcmp(command, "collect") == 0) {
			maestro_cmd_collect(cmd_ctxt, queue, status);
		}

		if (strcmp(command, "flush") == 0) {
			maestro_cmd_flush(cmd_ctxt, queue);
		}
	} while (true);

	return 0;
}


