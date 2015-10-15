/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#ifndef _GE_DB_UPDATE_H_
#define _GE_DB_UPDATE_H_

#include "ge-ugdata.h"

bool _ge_db_update_reg_cb(ge_ugdata *ugd);
bool _ge_db_update_finalize(ge_ugdata *ugd);
int _ge_db_update_get_info(void *data,
			   media_content_db_update_item_type_e *update_item,
			   media_content_db_update_type_e *update_type);

#endif //_GE_DB_UPDATE_H_


