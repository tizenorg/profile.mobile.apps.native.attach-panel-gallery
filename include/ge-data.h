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

#ifndef _GE_DATA_H_
#define _GE_DATA_H_

#include "ge-local-data.h"
#include "gallery-efl.h"

#ifdef _cplusplus
extern "C" {
#endif

#define GE_ALBUMS_FIRST_COUNT 8

typedef enum {
	GE_SORT_BY_NONE,	/**< No Sort */
	GE_SORT_BY_NAME_DESC, 	/**< Sort by display name descending */
	GE_SORT_BY_NAME_ASC, 	/**< Sort by display name ascending */
	GE_SORT_BY_DATE_DESC, 	/**< Sort by modified_date descending */
	GE_SORT_BY_DATE_ASC, 	/**< Sort by modified_date ascending */
} ge_sort_type_e;

typedef enum {
	GE_ALBUM_DATA_NONE,	/**< First time, default type */
	GE_ALBUM_DATA_LOCAL, 	/**< There some local/web albums need to be got */
	GE_ALBUM_DATA_WEB, 	/**< There some web albums need to be got */
	GE_ALBUM_DATA_DONE, 	/**< Data getting done */
} ge_album_data_e;

typedef int (*ge_restore_selected_cb)(Eina_List *sel_id, ge_item *gitem);

int _ge_data_get_sel_cnt(ge_ugdata *ugd);
int _ge_data_get_sel_paths(ge_ugdata *ugd, char **filepath, char ***filepath_arr, int *cnt);
int _ge_data_get_album_sel_cnt(ge_ugdata *ugd, char *uuid, int *cnt);
int _ge_data_check_sel_items(ge_ugdata *ugd);
int _ge_data_free_sel_albums(ge_ugdata *ugd);
bool _ge_data_check_selected_id(Eina_List *sel_id_list, const char *id);
int _ge_data_update_items_cnt(ge_ugdata *ugd, ge_cluster *album);
int _ge_data_free_clusters(ge_ugdata *ugd);
int _ge_data_get_clusters(ge_ugdata *ugd, int type);
int _ge_data_get_cluster(ge_ugdata *ugd, char *uuid, ge_cluster **cluster);
int _ge_data_free_cluster(ge_cluster *cluster);
int _ge_data_free_medias(Eina_List **elist);
int _ge_data_get_medias(ge_ugdata *ugd, char *uuid, int type, int start_pos,
			int end_pos, Eina_List **pmedias_elist,
			ge_restore_selected_cb restore_cb, Eina_List *sel_id);
int _ge_data_update_medias(ge_ugdata *ugd, char *uuid, int type,
			   Eina_List **pmedias_elist,
			   ge_restore_selected_cb restore_cb,
			   Eina_List *sel_id);
int _ge_data_get_album_cover(ge_ugdata *ugd, ge_cluster *album,
			     ge_item **pgitem, media_content_order_e sort_type);
int _ge_data_get_item_cnt(ge_ugdata *ugd, const char *cluster_id,
			  int album_type, int *item_cnt);
bool _ge_data_check_update(ge_ugdata *ugd);
int _ge_data_del_media_by_id(ge_ugdata* ugd, const char *media_id);
int _ge_data_free_selected_medias(Eina_List **elist);
bool _ge_data_is_default_album(const char *match_folder, ge_album_s *mcluster);
bool _ge_data_is_camera_album(ge_album_s *mcluster);
bool _ge_data_check_root_type(const char *path, const char *root);
bool _ge_data_is_root_path(const char *path);
int _ge_data_create_thumb(ge_media_s *item,
			  media_thumbnail_completed_cb callback, void *data);
int _ge_data_cancel_thumb(ge_media_s *item);
int _ge_data_restore_selected(Eina_List *sel_ids, ge_item *gitem);
int _ge_data_init(ge_ugdata* ugd);
int _ge_data_finalize(ge_ugdata* ugd);

#ifdef _cplusplus
}
#endif
#endif /* _GE_DATA_H_ */

