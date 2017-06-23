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
#include "maestro_deserialize.h"
#include "maestro_note.h"

static bool maestro_deserialize_note_assign(const msgpack_object obj,
	void *out,
	gru_status_t *status) {
	switch (obj.type) {
		case MSGPACK_OBJECT_BOOLEAN: {
			(*(bool *) out) = obj.via.boolean;
			break;
		}
		case MSGPACK_OBJECT_POSITIVE_INTEGER: {
			(*(int64_t *) out) = obj.via.i64;
			break;
		}
		case MSGPACK_OBJECT_NEGATIVE_INTEGER: {
			(*(uint64_t *) out) = obj.via.u64;
			break;
		}
#if defined(MSGPACK_LEGACY_SUPPORT)
		case MSGPACK_OBJECT_DOUBLE: {
			(*(double *) out) = obj.via.dec;
			break;
		}

		case MSGPACK_OBJECT_RAW: {
			snprintf((char *) out,
				obj.via.raw.size + 1,
				"%.*s",
				obj.via.raw.size,
				obj.via.raw.ptr);
			break;
#else
		case MSGPACK_OBJECT_FLOAT: {
			(*(double *) out) = obj.via.f64;
			break;
		}
		case MSGPACK_OBJECT_STR: {
			snprintf((char *) out,
				obj.via.str.size + 1,
				"%.*s",
				obj.via.str.size,
				obj.via.str.ptr);
			break;
#endif
		}
		default: {
			gru_status_set(status, GRU_FAILURE, "Unsupported type: %d", obj.type);
			return false;
		}
	}

	return true;
}

#if defined(MSGPACK_LEGACY_SUPPORT)
#define size_memb data.via.raw.size
#else
#define size_memb data.via.str.size
#endif

static bool maestro_deserialize_note_set_request(const msg_content_data_t *in,
	maestro_note_t *note,
	msgpack_unpacked *msg,
	size_t *offset,
	gru_status_t *status) {
	if (!maestro_note_payload_prepare(note, status)) {
		return false;
	}

	// Set command
	msgpack_unpack_return ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to unpack set command: invalid and/or missing command");

		return false;
	}

	if (!maestro_deserialize_note_assign(
			msg->data, &note->payload->request.set.opt, status)) {
		return false;
	}

	// Set value
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to unpack set value: invalid and/or missing value");

		return false;
	}

	note->payload->request.set.value = gru_alloc(msg->size_memb + 1, status);

	gru_alloc_check(note->payload->request.set.value, false);

	if (!maestro_deserialize_note_assign(
			msg->data, note->payload->request.set.value, status)) {
		return false;
	}

	return true;
}

static bool maestro_deserialize_note_ping_request(const msg_content_data_t *in,
	maestro_note_t *note,
	msgpack_unpacked *msg,
	size_t *offset,
	gru_status_t *status) {
	if (!maestro_note_payload_prepare(note, status)) {
		return false;
	}

	// Ping timestamp
	msgpack_unpack_return ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to unpack ping timestamp: invalid and/or missing timestamp");

		return false;
	}

  	note->payload->request.ping.ts = gru_alloc(msg->size_memb + 1, status);
  	gru_alloc_check(note->payload->request.ping.ts, false);

	if (!maestro_deserialize_note_assign(
			msg->data, note->payload->request.ping.ts, status)) {
		return false;
	}

	return true;
}

static bool maestro_deserialize_note_ping_response(const msg_content_data_t *in,
												  maestro_note_t *note,
												  msgpack_unpacked *msg,
												  size_t *offset,
												  gru_status_t *status) {
  if (!maestro_note_payload_prepare(note, status)) {
	return false;
  }

  // Ping - client ID
  msgpack_unpack_return ret = msgpack_unpack_next(msg, in->data, in->size, offset);
  if (ret != MSGPACK_UNPACK_SUCCESS) {
	gru_status_set(status,
				   GRU_FAILURE,
				   "Unable to unpack ping response: invalid and/or missing ID");

	return false;
  }

  note->payload->response.ping.id = gru_alloc(msg->size_memb + 1, status);
  gru_alloc_check(note->payload->response.ping.id, false);

  if (!maestro_deserialize_note_assign(
	  msg->data, note->payload->response.ping.id, status)) {
	return false;
  }

  // Ping - client name
  ret = msgpack_unpack_next(msg, in->data, in->size, offset);
  if (ret != MSGPACK_UNPACK_SUCCESS) {
	gru_status_set(status,
				   GRU_FAILURE,
				   "Unable to unpack ping response: invalid and/or missing name");

	return false;
  }

  note->payload->response.ping.name = gru_alloc(msg->size_memb + 1, status);
  gru_alloc_check(note->payload->response.ping.name, false);

  if (!maestro_deserialize_note_assign(
	  msg->data, note->payload->response.ping.name, status)) {
	return false;
  }

  // Ping - elapsed
  ret = msgpack_unpack_next(msg, in->data, in->size, offset);
  if (ret != MSGPACK_UNPACK_SUCCESS) {
	gru_status_set(status,
				   GRU_FAILURE,
				   "Unable to unpack ping response: invalid and/or missing elapsed time");

	return false;
  }

  if (!maestro_deserialize_note_assign(msg->data, &note->payload->response.ping.elapsed, status)) {
	return false;
  }

  return true;
}


static bool maestro_deserialize_note_stats_response(const msg_content_data_t *in,
												   maestro_note_t *note,
												   msgpack_unpacked *msg,
												   size_t *offset,
												   gru_status_t *status) {
	if (!maestro_note_payload_prepare(note, status)) {
		return false;
	}

	// Stats - client ID
	msgpack_unpack_return ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
					   GRU_FAILURE,
					   "Unable to unpack stats response: invalid and/or missing ID");

		return false;
	}

	note->payload->response.stats.id = gru_alloc(msg->size_memb + 1, status);
	gru_alloc_check(note->payload->response.stats.id, false);

	if (!maestro_deserialize_note_assign(
		msg->data, note->payload->response.stats.id, status)) {
		return false;
	}

	// Stats - client name
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
					   GRU_FAILURE,
					   "Unable to unpack stats response: invalid and/or missing name");

		return false;
	}

	note->payload->response.stats.name = gru_alloc(msg->size_memb + 1, status);
	gru_alloc_check(note->payload->response.stats.name, false);

	if (!maestro_deserialize_note_assign(
		msg->data, note->payload->response.stats.name, status)) {
		return false;
	}

	// Stats - child count
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
					   GRU_FAILURE,
					   "Unable to unpack stats response: invalid and/or missing child count");

		return false;
	}


	if (!maestro_deserialize_note_assign(msg->data, &note->payload->response.stats.child_count, status)) {
		return false;
	}

	// Stats - role
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
					   GRU_FAILURE,
					   "Unable to unpack stats response: invalid and/or missing role");

		return false;
	}

	note->payload->response.stats.role = gru_alloc(msg->size_memb + 1, status);
	gru_alloc_check(note->payload->response.stats.role, false);

	if (!maestro_deserialize_note_assign(msg->data, note->payload->response.stats.role, status)) {
		return false;
	}

	// Stats - roleinfo
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
					   GRU_FAILURE,
					   "Unable to unpack stats response: invalid and/or missing roleinfo");

		return false;
	}

	note->payload->response.stats.roleinfo = gru_alloc(msg->size_memb + 1, status);
	gru_alloc_check(note->payload->response.stats.roleinfo, false);

	if (!maestro_deserialize_note_assign(
		msg->data, note->payload->response.stats.roleinfo, status)) {
		return false;
	}


	// Stats - stats type
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
					   GRU_FAILURE,
					   "Unable to unpack stats response: invalid and/or missing stat type");

		return false;
	}

	if (!maestro_deserialize_note_assign(msg->data, &note->payload->response.stats.stat_type, status)) {
		return false;
	}

	// Stats - perf TS
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
					   GRU_FAILURE,
					   "Unable to unpack stats response: invalid and/or missing perf timestamp");

		return false;
	}


	note->payload->response.stats.stats.perf.timestamp = gru_alloc(msg->size_memb + 1, status);
	gru_alloc_check(note->payload->response.stats.stats.perf.timestamp, false);

	if (!maestro_deserialize_note_assign(
		msg->data, note->payload->response.stats.stats.perf.timestamp, status)) {
		return false;
	}

	// Stats - perf count
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
					   GRU_FAILURE,
					   "Unable to unpack stats response: invalid and/or missing perf count");

		return false;
	}

	if (!maestro_deserialize_note_assign(msg->data, &note->payload->response.stats.stats.perf.count, status)) {
		return false;
	}

	// Stats - perf count
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
					   GRU_FAILURE,
					   "Unable to unpack stats response: invalid and/or missing perf rate");

		return false;
	}

	if (!maestro_deserialize_note_assign(msg->data, &note->payload->response.stats.stats.perf.rate, status)) {
		return false;
	}

	// Stats - perf latency
	ret = msgpack_unpack_next(msg, in->data, in->size, offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
					   GRU_FAILURE,
					   "Unable to unpack stats response: invalid and/or missing perf latency");

		return false;
	}

	if (!maestro_deserialize_note_assign(msg->data, &note->payload->response.stats.stats.perf.latency, status)) {
		return false;
	}

	return true;
}


bool maestro_deserialize_note(const msg_content_data_t *in,
	maestro_note_t *note,
	gru_status_t *status) {
	size_t offset = 0;
	msgpack_unpacked msg;

	msgpack_unpacked_init(&msg);

	msgpack_unpack_return ret;

	// Deserialize type
	ret = msgpack_unpack_next(&msg, in->data, in->size, &offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to unpack protocol data: invalid and/or missing note type");

		return false;
	}

	if (msg.data.type != MSGPACK_OBJECT_POSITIVE_INTEGER) {
		gru_status_set(
			status, GRU_FAILURE, "Unable to unpack protocol data: invalid note type");
	} else {
		if (!maestro_deserialize_note_assign(msg.data, &note->type, status)) {
			goto err_exit;
		}
	}

	ret = msgpack_unpack_next(&msg, in->data, in->size, &offset);
	if (ret != MSGPACK_UNPACK_SUCCESS) {
		gru_status_set(status,
			GRU_FAILURE,
			"Unable to unpack protocol data: invalid and/or missing command");

		return false;
	} else {
		if (!maestro_deserialize_note_assign(msg.data, &note->command, status)) {
			goto err_exit;
		}
	}

	bool pl_ret = true;

	switch (note->command) {
		case MAESTRO_NOTE_SET: {
			if (note->type == MAESTRO_TYPE_REQUEST) {
				pl_ret =
					maestro_deserialize_note_set_request(in, note, &msg, &offset, status);
			}

			break;
		}
		case MAESTRO_NOTE_PING: {
			if (note->type == MAESTRO_TYPE_REQUEST) {
				pl_ret = maestro_deserialize_note_ping_request(in, note, &msg, &offset, status);
			}
			else {
			  pl_ret = maestro_deserialize_note_ping_response(in, note, &msg, &offset, status);
			}

			break;
		}
		case MAESTRO_NOTE_STATS: {
			if (note->type == MAESTRO_TYPE_RESPONSE) {
				pl_ret = maestro_deserialize_note_stats_response(in, note, &msg, &offset, status);
			}
		}
	}

	if (!pl_ret) {
		goto err_exit;
	}

	msgpack_unpacked_destroy(&msg);
	return true;

err_exit:
	msgpack_unpacked_destroy(&msg);
	return false;
}