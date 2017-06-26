/**
 Copyright 2017 Otavio Rodolfo Piske

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
#include "vmslh.h"

bool vmslh_add(gru_list_t *list, vmslh_callback_fn callback, void *payload, gru_status_t *status) {
	vmslh_callback_t *wrapper = gru_alloc(sizeof(vmslh_callback_t), status);
	gru_alloc_check(wrapper, false);

	wrapper->call = callback;
	wrapper->payload = payload;

	if (!gru_list_append(list, wrapper)) {
		gru_dealloc((void **)&wrapper);
		gru_status_set(status, GRU_FAILURE, "Unable to append VMSL handler wrapper");

		return false;
	}

	return true;
}

void vmslh_run(gru_list_t *list, void *ctxt, void *msg) {
	if (!list) {
		return;
	}

	gru_node_t *node = list->root;

	while (node) {
		vmslh_callback_t *callback = (vmslh_callback_t *) node->data;

		callback->call(ctxt, msg, callback->payload);

		node = node->next;
	}
}

static void vmslh_callback_cleanup(void **ptr) {
	vmslh_callback_t *cb = *ptr;

	if (!cb) {
		return;
	}

	gru_dealloc((void **) &cb);
}

vmslh_handlers_t vmslh_new(gru_status_t *status) {
	vmslh_handlers_t handlers = {0};

	handlers.before_send = gru_list_new(status);
	return handlers;
}

void vmslh_cleanup(vmslh_handlers_t *handlers) {
	if (!handlers) {
		return;
	}

	gru_list_clean(handlers->before_connect, vmslh_callback_cleanup);
	gru_list_clean(handlers->after_connect, vmslh_callback_cleanup);
	gru_list_clean(handlers->before_send, vmslh_callback_cleanup);
	gru_list_clean(handlers->after_send, vmslh_callback_cleanup);
	gru_list_clean(handlers->before_receive, vmslh_callback_cleanup);
	gru_list_clean(handlers->after_receive, vmslh_callback_cleanup);

	gru_list_destroy(&handlers->before_connect);
	gru_list_destroy(&handlers->after_connect);
	gru_list_destroy(&handlers->before_send);
	gru_list_destroy(&handlers->after_send);
	gru_list_destroy(&handlers->before_receive);
	gru_list_destroy(&handlers->after_receive);

}