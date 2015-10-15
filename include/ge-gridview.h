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

#ifndef _GE_GRIDVIEW_H_
#define _GE_GRIDVIEW_H_

#include "gallery-efl.h"

int _ge_grid_get_selected_ids(ge_ugdata *ugd, Eina_List **ids_list);
int _ge_grid_create_thumb(ge_cluster *album);
int _ge_grid_create_split_view_thumb(ge_cluster *album);
int _ge_grid_create_view(ge_ugdata *ugd, ge_thumbs_s *thumbs_d);
int _ge_grid_update_view(ge_ugdata *ugd);
Evas_Object *_ge_grid_get_layout(ge_ugdata *ugd);
int _ge_grid_update_lang(ge_ugdata *ugd);
int _ge_grid_update_data(ge_ugdata *ugd);
int _ge_grid_get_store_type(ge_ugdata *ugd);
int _ge_grid_create_main_view(ge_ugdata *ugd);
void _ge_grid_change_selection_mode(ge_ugdata *ugd, bool state);
int _ge_grid_sel_one(ge_ugdata *ugd, char *file_url);
bool _ge_is_grid_view(ge_ugdata *ugd);

#endif //_GE_GRIDVIEW_H_

