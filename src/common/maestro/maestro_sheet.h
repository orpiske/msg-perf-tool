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

#ifndef MAESTRO_SHEET_H
#define MAESTRO_SHEET_H


#include <common/gru_status.h>
#include <collection/gru_list.h>
#include <log/gru_logger.h>

#include "msg_content_data.h"
#include "mpt-debug.h"

#include "maestro_note.h"
#include "maestro_instrument.h"


typedef struct maestro_sheet_t_ {
	char *location; /** Sheet location (ie.: /mpt/sender/<ppid>) **/
	gru_list_t *instruments; /** A list of instruments */ 
} maestro_sheet_t;

maestro_sheet_t *maestro_sheet_new(const char *location, gru_status_t *status);

void maestro_sheet_add_instrument(maestro_sheet_t *sheet, 
	maestro_instrument_t *instrument);

void maestro_sheet_play(const maestro_sheet_t *sheet, const msg_content_data_t *cont, 
	msg_content_data_t *resp, gru_status_t *status);



#endif /* MAESTRO_SHEET_H */
