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

#ifndef _GE_UTIL_H_
#define _GE_UTIL_H_

#include "gallery-efl.h"
#include "ge-icon.h"
#include "ge-file-util.h"

#define GE_FILE_EXISTS(path) \
	(path && (1 == ge_file_exists(path)) && (ge_file_size(path) > 0))

#define GE_ICON_SET_FILE(icon, img) \
		elm_image_file_set(icon, GE_IMAGES_EDJ_FILE, img)


void _ge_set_current_item(ge_item* gitem);
int ge_reg_db_update_noti(ge_ugdata* ugd);
int ge_dereg_db_update_noti(void);
Eina_Bool ge_update_view(ge_ugdata* ugd, int mode);
bool _ge_is_image_valid(void *data, char *filepath);
char *_ge_get_duration_string(unsigned int v_dur);
ge_view_mode _ge_get_view_mode(ge_ugdata *ugd);
int _ge_set_view_mode(ge_ugdata *ugd, ge_view_mode mode);
double _ge_get_win_factor(Evas_Object *win, bool b_hide_indicator, int *width,
			  int *height);
int _ge_addhome_image(ge_ugdata *ugd, char *path);
int _ge_send_result(ge_ugdata *ugd);
int _ge_atoi(const char *number);
char *_ge_str(char *str_id);

#endif //_GE_UTIL_H_

