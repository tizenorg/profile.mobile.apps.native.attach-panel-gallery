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

#ifndef _GE_LOCAL_DATA_H_
#define _GE_LOCAL_DATA_H_

#include "ge-data-util.h"

int _ge_local_data_connect(void);
int _ge_local_data_disconnect(void);
int _ge_local_data_get_album_by_path(char *path, ge_album_s **album);
int _ge_local_data_get_album_list(ge_filter_s *condition, Eina_List **elilst);
int _ge_local_data_get_media_count(const char *cluster_id, ge_filter_s *filter,
				   int *item_cnt);
int _ge_local_data_get_all_media_count(ge_filter_s *filter, int *item_cnt);
int _ge_local_data_get_media(const char *media_id, ge_media_s **mitem);
int _ge_local_data_get_album_cover(char *album_id, ge_filter_s *condition,
				   Eina_List **elist);
int _ge_local_data_get_all_albums_cover(ge_filter_s *condition,
					Eina_List **elist);
int _ge_local_data_get_album_media_list(char *album_id, ge_filter_s *condition,
					Eina_List **elist);
int _ge_local_data_get_all_albums_media_list(ge_filter_s *condition,
					     Eina_List **elist);
int _ge_local_data_get_album_by_id(char *album_id, ge_album_s **cluster);
int _ge_local_data_get_media_by_id(char *media_id, ge_media_s **mitem);
int _ge_local_data_get_duration(media_info_h media, int *duration);

#endif //_GE_LOCAL_DATA_H_

