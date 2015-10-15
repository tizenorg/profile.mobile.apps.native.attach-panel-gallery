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

#include "ge-data.h"
#include "ge-debug.h"
#include "ge-util.h"
#include "ge-strings.h"
#include "ge-file-util.h"

/*
*  Free a ge_sel_album_s album
*/
int _ge_data_free_sel_album(ge_sel_album_s *album)
{
	GE_CHECK_VAL(album, -1);
	if (album->elist) {
		ge_sel_item_s *sit = NULL;
		EINA_LIST_FREE(album->elist, sit) {
			if (sit)
				_ge_data_util_free_sel_item(sit);
		}
	}
	GE_FREEIF(album->uuid);
	GE_FREE(album);
	return 0;
}

/* Remove album */
int _ge_data_remove_sel_album(ge_ugdata *ugd, ge_sel_album_s *salbum)
{
	if (ugd == NULL || ugd->selected_elist == NULL || salbum == NULL)
		return 0;

	/* Check album is created or not */
	ugd->selected_elist = eina_list_remove(ugd->selected_elist, salbum);
	_ge_data_free_sel_album(salbum);
	return 0;
}

/* Get count of selected items */
int _ge_data_get_sel_cnt(ge_ugdata *ugd)
{
	if (ugd == NULL || ugd->selected_elist == NULL)
		return 0;

	/* Check album is created or not */
	ge_sel_album_s *salbum = NULL;
	int cnt = 0;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(ugd->selected_elist, l, salbum) {
		if (salbum == NULL || salbum->elist == NULL)
			continue;
		cnt += eina_list_count(salbum->elist);
		salbum = NULL;
	}
	ge_dbg("Count: %d", cnt);
	return cnt;
}

/* Append items' path */
int _ge_data_get_sel_paths(ge_ugdata *ugd, char **filepath, char ***filepath_arr, int *cnt)
{
	if (filepath == NULL || ugd == NULL || ugd->selected_elist == NULL || filepath_arr == NULL) {
		ge_dbgE("Invalid input!");
		return -1;
	}

	Eina_List *path_list = NULL;
	GString *selected_path = g_string_new(NULL);
	int _cnt = 0;

	/* Get items path from album */
	ge_dbg("File count: %d", eina_list_count(ugd->selected_elist));

	ge_sel_item_s *sit = NULL;
	Eina_List *l = NULL;

	EINA_LIST_FOREACH(ugd->selected_elist, l, sit) {
		g_string_append(selected_path, sit->file_url);
			g_string_append_c(selected_path, ';');
			path_list = eina_list_append(path_list, sit);
			_cnt++;
			sit = NULL;
		}

	/* copy path from path_list to path_array */
	int idx = 0;
	sit = NULL;
	if (_cnt > 0) {
		*filepath_arr = (char **)calloc(_cnt, sizeof(char *));
		EINA_LIST_FREE(path_list, sit) {
			if (sit && sit->file_url) {
				(*filepath_arr)[idx] = strdup(sit->file_url);
				idx++;
				sit = NULL;
			} else {
				ge_sdbgE("Invalid file path");
			}
		}
		ge_dbg("array finished!");
	} else {
		ge_sdbgE("Copy to path_array failed!");
	}

	int str_len = strlen(selected_path->str);
	ge_dbg("path string length: %d", str_len);
	g_string_truncate(selected_path, str_len - 1);
	*filepath = g_string_free(selected_path, false);

	if (cnt)
		*cnt = _cnt;
	return 0;
}

int _ge_data_get_album_sel_cnt(ge_ugdata *ugd, char *uuid, int *cnt)
{
	GE_CHECK_VAL(cnt, -1);
	GE_CHECK_VAL(uuid, -1);
	GE_CHECK_VAL(ugd, -1);
	if (ugd->selected_elist == NULL) {
		ge_dbgW("Empty selected list!");
		return -1;
	}
	*cnt = 0;

	ge_sel_album_s *salbum = NULL;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(ugd->selected_elist, l, salbum) {
		if (salbum == NULL || salbum->elist == NULL ||
		    salbum->uuid == NULL) {
			ge_dbgW("Empty salbum!");
			continue;
		}
		if (!g_strcmp0(salbum->uuid, uuid)) {
			*cnt = eina_list_count(salbum->elist);
			return 0;
		}
		salbum = NULL;
	}
	return -1;
}

/* Check items */
int _ge_data_check_sel_items(ge_ugdata *ugd)
{
	if (ugd == NULL) {
		ge_dbgE("Error input!");
		return -1;
	}

	/* Check album is appended or not */
	ge_sel_album_s *salbum = NULL;
	Eina_List *l = NULL;
	Eina_List *l2 = NULL;
	ge_sel_item_s *sit = NULL;
	EINA_LIST_FOREACH(ugd->selected_elist, l, salbum) {
		if (salbum == NULL || salbum->uuid == NULL)
			continue;
		EINA_LIST_FOREACH(salbum->elist, l2, sit) {
			if (sit == NULL)
				continue;
			if (sit->uuid == NULL || sit->file_url == NULL)
				salbum->elist = eina_list_remove(salbum->elist,
								 sit);
			sit = NULL;
		}
		salbum = NULL;
	}
	return 0;
}

/* Remove sel albums */
int _ge_data_free_sel_albums(ge_ugdata *ugd)
{
	if (ugd == NULL || ugd->selected_elist == NULL)
		return 0;

	/* Check album is created or not */
	ge_sel_album_s *salbum = NULL;
	EINA_LIST_FREE(ugd->selected_elist, salbum) {
		if (salbum == NULL || salbum->elist == NULL)
			continue;
		_ge_data_free_sel_album(salbum);
		salbum = NULL;
	}
	return 0;
}

/* Check ID is in the list or not */
bool _ge_data_check_selected_id(Eina_List *sel_id_list, const char *id)
{
	GE_CHECK_FALSE(sel_id_list);
	GE_CHECK_FALSE(id);
	Eina_List *tmp_elist = NULL;
	ge_sel_item_s *sit = NULL;

	if (eina_list_count(sel_id_list) == 0) {
		ge_dbgE("sel_id_list is empty!");
	    	return false;
	}

	EINA_LIST_FOREACH(sel_id_list, tmp_elist, sit) {
		if (sit == NULL || sit->uuid == NULL) {
			ge_dbgE("Invalid p_id!");
			sit = NULL;
			continue;
		}
		if (g_strcmp0(id, sit->uuid)) {
			sit = NULL;
			continue;
		}

		 sit->valid = 1;
		 return true;
	}
	return false;
}

int _ge_data_update_items_cnt(ge_ugdata *ugd, ge_cluster *album)
{
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(album->cluster->uuid, -1);
	GE_CHECK_VAL(ugd, -1);
	int err = -1;
	int item_count = 0;

	ge_filter_s filter;
	memset(&filter, 0x00, sizeof(ge_filter_s));
	filter.sort_type = MEDIA_CONTENT_ORDER_DESC;
	g_strlcpy(filter.sort_keyword, GE_CONDITION_ORDER, KEYWORD_LENGTH);
	filter.collate_type = MEDIA_CONTENT_COLLATE_NOCASE;
	filter.offset = GE_GET_ALL_RECORDS;
	filter.count = GE_GET_ALL_RECORDS;
	g_strlcpy(filter.cond, GE_CONDITION_IMAGE_VIDEO, CONDITION_LENGTH);
	filter.with_meta = false;

	if (ugd->file_type_mode == GE_FILE_T_IMAGE)
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE, CONDITION_LENGTH);
	else if (ugd->file_type_mode == GE_FILE_T_VIDEO)
		g_strlcpy(filter.cond, GE_CONDITION_VIDEO, CONDITION_LENGTH);

	if (g_strcmp0(album->cluster->uuid, GE_ALBUM_ALL_ID)) {
		err = _ge_local_data_get_media_count(album->cluster->uuid,
						     &filter, &item_count);
	} else {
		/* "All albums" album */
		ge_dbg("all media count");
		err = _ge_local_data_get_all_media_count(&filter, &item_count);
		/* Update flag */
		if (ugd->cluster_list->b_updated) {
			ge_dbgW("Update flag: all media count!");
			ugd->cluster_list->all_medias_cnt = item_count;
		}
	}
	if (err < 0) {
		ge_dbgE("Get item count failed(%d)!", err);
		return -1;
	}

	ge_dbg("cluster media count : old=%d, new=%d", album->cluster->count,
	       item_count);
	album->cluster->count = item_count;

	return 0;
}

int _ge_data_free_clusters(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	if (ugd->cluster_list) {
		if (ugd->cluster_list->clist) {
			ge_dbg("Clear clusters");
			ge_cluster *album = NULL;
			EINA_LIST_FREE(ugd->cluster_list->clist, album) {
				if (album)
					_ge_data_util_free_cluster(album);

				album = NULL;
			}
			ugd->cluster_list->clist = NULL;
		}

		GE_FREE(ugd->cluster_list);
	}
	return 0;
}

int _ge_data_get_clusters(ge_ugdata *ugd, int type)
{
	GE_CHECK_VAL(ugd, -1);
	int length = 0;
	int all_local_cnt = 0;
	Eina_List *list = NULL;
	ge_album_s *album = NULL;
	ge_album_s *camera_album = NULL;
	ge_album_s *downloads_album = NULL;
	ge_cluster *camera_cluster = NULL;
	ge_cluster* gcluster = NULL;
	int medias_cnt = 0;
	int err = -1;
	ge_filter_s filter;
	int local_cnt = 0;
	int data_type = GE_ALBUM_DATA_DONE;
	bool b_8_albums_got = false;

	if (type == GE_ALBUM_DATA_NONE) {
		if (ugd->cluster_list == NULL) {
			ugd->cluster_list = _ge_data_util_calloc_clusters_list();
			GE_CHECK_VAL(ugd->cluster_list, -1);
		} else {
			if (ugd->cluster_list->clist) {
				ge_dbg("Clear clusters list");
				EINA_LIST_FREE(ugd->cluster_list->clist, gcluster) {
					if (gcluster) {
						_ge_data_util_free_cluster(gcluster);
						gcluster = NULL;
					}
				}
				ugd->cluster_list->clist = NULL;
			}
			ugd->cluster_list->data_type = GE_ALBUM_DATA_NONE;
			ugd->cluster_list->local_cnt = 0;
			ugd->cluster_list->web_cnt = 0;
		}
		ugd->cluster_list->b_updated = true;
	}
	GE_CHECK_VAL(ugd->cluster_list, -1);

 GE_DATA_AGAIN:

	/* Get local albums for first time and local albums is more than 9 */
	if (type == GE_ALBUM_DATA_NONE || type == GE_ALBUM_DATA_LOCAL) {
		/* Get real albums */
		memset(&filter, 0x00, sizeof(ge_filter_s));
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE_VIDEO,
			  CONDITION_LENGTH);
		filter.collate_type = MEDIA_CONTENT_COLLATE_NOCASE;
		filter.sort_type = MEDIA_CONTENT_ORDER_ASC;
		g_strlcpy(filter.sort_keyword, FOLDER_NAME, KEYWORD_LENGTH);
		if (type == GE_ALBUM_DATA_NONE) {
			_ge_local_data_get_album_by_path(GE_CAMERA_PATH_PHONE,
							 &camera_album);
			if (camera_album)
				list = eina_list_append(list, camera_album);
			camera_album = NULL;
			_ge_local_data_get_album_by_path(GE_CAMERA_PATH_MMC,
							 &camera_album);
			if (camera_album)
				list = eina_list_append(list, camera_album);
			_ge_local_data_get_album_by_path(GE_DOWNLOADS_PATH,
							 &downloads_album);
			if (downloads_album)
				list = eina_list_append(list, downloads_album);

			filter.count = GE_ALBUMS_FIRST_COUNT;
			filter.list_type = GE_ALBUM_LIST_FIRST;
		} else {
			filter.count = -1;
			filter.list_type = GE_ALBUM_LIST_ALL;
		}
		filter.offset = ugd->cluster_list->local_cnt;
		filter.with_meta = false;

		err = _ge_local_data_get_album_list(&filter, &list);
		if (err != 0) {
			ge_dbgW("No record");
			if (list)
				_ge_data_util_free_mtype_items(&list);
		} else {
			memset(&filter, 0x00, sizeof(ge_filter_s));
			if (ugd->file_type_mode == GE_FILE_T_IMAGE)
				g_strlcpy(filter.cond, GE_CONDITION_IMAGE,
					  CONDITION_LENGTH);
			else if (ugd->file_type_mode == GE_FILE_T_VIDEO)
				g_strlcpy(filter.cond, GE_CONDITION_VIDEO,
					  CONDITION_LENGTH);
			else
				g_strlcpy(filter.cond, GE_CONDITION_IMAGE_VIDEO,
					  CONDITION_LENGTH);
			filter.sort_type = MEDIA_CONTENT_ORDER_DESC;
			g_strlcpy(filter.sort_keyword, MEDIA_DISPLAY_NAME,
				  KEYWORD_LENGTH);
			filter.offset = GE_GET_ALL_RECORDS;
			filter.count = GE_GET_ALL_RECORDS;
			filter.with_meta = false;

			local_cnt = eina_list_count(list);
			ugd->cluster_list->local_cnt += local_cnt;
		}
		ge_dbg("Local albums count: %d", local_cnt);
	}
	/* Need to get albums again from local album */
	if (type == GE_ALBUM_DATA_NONE) {
		data_type = GE_ALBUM_DATA_LOCAL;
		if (local_cnt == GE_ALBUMS_FIRST_COUNT) {
			ge_dbg("8 albums got");
			b_8_albums_got = true;
		} else {
			b_8_albums_got = false;
		}
	}

	Eina_List *clist = ugd->cluster_list->clist;
	EINA_LIST_FREE(list, album) {
		if (album == NULL || album->uuid == NULL) {
			/* Invalid data, next one */
			ge_dbgE("Invalid ge_album_s!");
			continue;
		}
		ge_sdbg("Cluster ID: %s.", album->uuid);

		err = _ge_local_data_get_media_count(album->uuid,
						     &filter,
						     &medias_cnt);
		if (err == 0 && medias_cnt > 0) {
			album->count = medias_cnt;
			all_local_cnt += medias_cnt;
		} else {
			ge_dbgW("local album is empty!");
			_ge_data_type_free_geitem((void **)&album);
			continue;
		}
		gcluster = _ge_data_util_calloc_cluster();
		if (gcluster == NULL) {
			ge_dbgE("_ge_data_util_calloc_cluster failed!");
			_ge_data_type_free_geitem((void **)&album);
			continue;
		}

		gcluster->cluster = album;
		gcluster->ugd = ugd;
		length += album->count;

		clist = eina_list_append(clist, gcluster);
		if (type == GE_ALBUM_DATA_NONE) {
			if (_ge_data_is_camera_album(album)) {
				/*if (_ge_data_check_root_type(album->path, GE_ROOT_PATH_PHONE))
					camera_cluster = gcluster;
				else  MMC album
					camera_cluster = gcluster;*/
				camera_cluster = gcluster;
			}
		} else {
			if (ugd->cluster_list->append_cb)
				ugd->cluster_list->append_cb(ugd, gcluster);
		}
		ugd->cluster_list->clist = clist;
	}
	ge_dbgW("Get clusters Done[%d]!", all_local_cnt);
	if (type == GE_ALBUM_DATA_NONE && all_local_cnt) {
		/* Create "All" album if any file exists */
		gcluster = _ge_data_util_new_cluster_all(ugd, all_local_cnt);
		GE_CHECK_VAL(gcluster, -1);

		if (camera_cluster)
			clist = eina_list_append_relative(clist, gcluster,
							  camera_cluster);
		else
			clist= eina_list_prepend(clist, gcluster);

		ugd->cluster_list->clist = clist;
		ge_dbg("<All albums> added!");
	} else if (type == GE_ALBUM_DATA_NONE && b_8_albums_got) {
		ge_dbgW("Again!");
		goto GE_DATA_AGAIN;
	}

	_ge_data_check_sel_items(ugd);
	ugd->cluster_list->data_type = data_type;
	return data_type;
}

int _ge_data_get_cluster(ge_ugdata *ugd, char *uuid, ge_cluster **cluster)
{
	GE_CHECK_VAL(cluster, -1);
	GE_CHECK_VAL(uuid, -1);
	GE_CHECK_VAL(ugd, -1);
	int ret = -1;
	*cluster = NULL;
	ge_cluster *_cluster = NULL;

	if (!g_strcmp0(uuid, GE_ALBUM_ALL_ID)) {
		int cnt = 0;
		ret = _ge_data_get_item_cnt(ugd, GE_ALBUM_ALL_ID, GE_ALL, &cnt);
		_cluster = _ge_data_util_new_cluster_all(ugd, cnt);
	} else {
		ge_album_s *mcluster = NULL;
		ret = _ge_local_data_get_album_by_id(ugd->slideshow_album_id,
						     &mcluster);
		if (ret < 0 || mcluster == NULL) {
			if (mcluster)
				_ge_data_type_free_geitem((void **)&mcluster);
			return -1;
		}

		_cluster = _ge_data_util_calloc_cluster();
		if (_cluster == NULL) {
			ge_dbgE("_ge_data_util_calloc_cluster failed!");
			_ge_data_type_free_geitem((void **)&mcluster);
			return -1;
		}

		_cluster->cluster = mcluster;
		_cluster->ugd = ugd;
	}

	if (ret < 0 || _cluster == NULL) {
		if (_cluster) {
			_ge_data_util_free_cluster(_cluster);
			_cluster = NULL;
		}
		return -1;
	}

	*cluster = _cluster;
	return 0;
}

int _ge_data_free_cluster(ge_cluster *cluster)
{
	return _ge_data_util_free_cluster(cluster);
}

/* Clear items list */
int _ge_data_free_medias(Eina_List **elist)
{
	if (elist == NULL || *elist == NULL)
		return -1;
	ge_item *gitem = NULL;
	ge_dbg("Clear medias");
	EINA_LIST_FREE(*elist, gitem) {
		_ge_data_util_free_geitem(gitem);
		gitem = NULL;
	}
	return 0;
}

int _ge_data_get_medias(ge_ugdata *ugd, char *uuid, int type, int start_pos,
			int end_pos, Eina_List **pmedias_elist,
			ge_restore_selected_cb restore_cb, Eina_List *sel_id)
{
	GE_CHECK_VAL(pmedias_elist, -1);
	GE_CHECK_VAL(ugd, -1);
	Eina_List *itemlist = NULL;
	int err = -1;
	ge_filter_s filter;

	memset(&filter, 0, sizeof(ge_filter_s));
	ge_dbg("--start_pos[%d], end_pos[%d]--", start_pos, end_pos);
	if (ugd->file_type_mode == GE_FILE_T_IMAGE) {
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE, CONDITION_LENGTH);
	} else if (ugd->file_type_mode == GE_FILE_T_VIDEO) {
		g_strlcpy(filter.cond, GE_CONDITION_VIDEO, CONDITION_LENGTH);
	} else {
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE_VIDEO,
			  CONDITION_LENGTH);
	}

	filter.collate_type = MEDIA_CONTENT_COLLATE_NOCASE;
	filter.sort_type = MEDIA_CONTENT_ORDER_DESC;
	g_strlcpy(filter.sort_keyword, GE_CONDITION_ORDER, KEYWORD_LENGTH);
	filter.offset = start_pos;
	filter.count = end_pos - start_pos + 1;
	filter.with_meta = true;


	ge_dbg("--Album All--");
	err = _ge_local_data_get_all_albums_media_list(&filter,
							   &itemlist);

	if ((err == 0) && (itemlist != NULL)) {
		ge_media_s *item = NULL;
		ge_item* gitem = NULL;
		EINA_LIST_FREE(itemlist, item) {
			if (item == NULL || item->uuid == NULL) {
				ge_dbgE("Invalid item!");
				continue;
			}

			gitem = _ge_data_util_calloc_geitem();
			if (gitem == NULL) {
				ge_dbgE("_ge_data_util_calloc_geitem() failed");
				_ge_data_type_free_geitem((void **)&item);
				continue;
			}

			gitem->item = item;
			gitem->ugd = ugd;
			gitem->store_type = type;
			*pmedias_elist = eina_list_append(*pmedias_elist,
							  gitem);

			if (restore_cb && sel_id)
				restore_cb(sel_id, gitem);
			item = NULL;
			gitem = NULL;
		}
	} else {
		/* Free Mitems */
		if (itemlist)
			_ge_data_util_free_mtype_items(&itemlist);
	}
	ge_dbg("medias_elist=%p", *pmedias_elist);
	return err;
}

/* Update items list, especially used in thumbnails edit view */
int _ge_data_update_medias(ge_ugdata *ugd, char *uuid, int type,
			   Eina_List **pmedias_elist,
			   ge_restore_selected_cb restore_cb,
			   Eina_List *sel_id)
{
	GE_CHECK_VAL(pmedias_elist, -1);
	GE_CHECK_VAL(uuid, -1);
	GE_CHECK_VAL(ugd, -1);
	Eina_List* itemlist = NULL;
	int err = -1;
	ge_sdbg("Update content of %s", uuid);

	ge_filter_s filter;
	memset(&filter, 0x00, sizeof(ge_filter_s));
	if (ugd->file_type_mode == GE_FILE_T_IMAGE) {
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE, CONDITION_LENGTH);
	} else if (ugd->file_type_mode == GE_FILE_T_VIDEO) {
		g_strlcpy(filter.cond, GE_CONDITION_VIDEO, CONDITION_LENGTH);
	} else {
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE_VIDEO,
			  CONDITION_LENGTH);
	}

	filter.collate_type = MEDIA_CONTENT_COLLATE_NOCASE;
	filter.sort_type = MEDIA_CONTENT_ORDER_DESC;
	g_strlcpy(filter.sort_keyword, GE_CONDITION_ORDER, KEYWORD_LENGTH);
	filter.offset = GE_GET_ALL_RECORDS;
	filter.count = GE_GET_ALL_RECORDS;
	filter.with_meta = true;

	if (g_strcmp0(uuid, GE_ALBUM_ALL_ID)) {
		err = _ge_local_data_get_album_media_list(uuid, &filter,
							  &itemlist);
	} else {
		err = _ge_local_data_get_all_albums_media_list(&filter,
							       &itemlist);
	}

	if ((err != 0) || (itemlist == NULL)) {
		ge_dbgE("(err != 0) || (itemlist == NULL)");
		/* Free Mitems */
		if (itemlist)
			_ge_data_util_free_mtype_items(&itemlist);
		return err;
	}

	ge_media_s *item = NULL;
	ge_item* gitem = NULL;
	EINA_LIST_FREE(itemlist, item) {
		if (item == NULL || item->uuid == NULL) {
			ge_dbgE("Invalid item!");
			continue;
		}
		gitem = _ge_data_util_calloc_geitem();
		if (gitem == NULL) {
			ge_dbgE("_ge_data_util_calloc_geitem() failed");
			_ge_data_type_free_geitem((void **)&item);
			continue;
		}

		gitem->item = item;
		gitem->ugd = ugd;
		gitem->store_type = type;
		*pmedias_elist = eina_list_append(*pmedias_elist, gitem);

		if (restore_cb && sel_id)
			restore_cb(sel_id, gitem);

		item = NULL;
		gitem = NULL;
	}

	ge_dbg("medias_elist=%p", *pmedias_elist);
	return err;
}

int _ge_data_get_album_cover(ge_ugdata *ugd, ge_cluster *album,
			     ge_item **pgitem, media_content_order_e sort_type)
{
	GE_CHECK_VAL(pgitem, -1);
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(album->cluster->uuid, -1);
	GE_CHECK_VAL(ugd, -1);

	Eina_List *item_list = NULL;
	ge_media_s *item = NULL;
	ge_item* gitem = NULL;
	int err = -1;
	ge_filter_s filter;

	memset(&filter, 0x00, sizeof(ge_filter_s));
	if (ugd->file_type_mode == GE_FILE_T_IMAGE) {
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE, CONDITION_LENGTH);
	} else if (ugd->file_type_mode == GE_FILE_T_VIDEO) {
		g_strlcpy(filter.cond, GE_CONDITION_VIDEO, CONDITION_LENGTH);
	} else {
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE_VIDEO,
			  CONDITION_LENGTH);
	}

	filter.collate_type = MEDIA_CONTENT_COLLATE_NOCASE;
	filter.sort_type = sort_type;
	g_strlcpy(filter.sort_keyword, GE_CONDITION_ORDER, KEYWORD_LENGTH);
	filter.offset = 0;
	filter.count = 1;
	filter.with_meta = false;

	if (g_strcmp0(album->cluster->uuid, GE_ALBUM_ALL_ID)) {
		/*real album */
		err = _ge_local_data_get_album_cover(album->cluster->uuid,
						     &filter, &item_list);
		if (err != 0 || item_list == NULL) {
			ge_dbgE("Get albums media list failed[%d]!", err);
			goto DB_FAILED;
		}
	} else {
		/* add "All" album */
		err = _ge_local_data_get_all_albums_cover(&filter, &item_list);
		if (err != 0 || item_list == NULL) {
			ge_dbgE("Get all albums media list failed[%d]!", err);
			goto DB_FAILED;
		}
	}

	EINA_LIST_FREE(item_list, item) {
		if (item == NULL) {
			ge_dbgE("Invalid ge_meida_s!");
			goto DB_FAILED;
		}
		gitem = _ge_data_util_calloc_geitem();
		if (gitem == NULL) {
			ge_dbgE("_ge_data_util_calloc_geitem failed!");
			_ge_data_type_free_geitem((void **)&item);
			goto DB_FAILED;
		}
		gitem->item = item;
		gitem->album = album;
		gitem->store_type = album->cluster->type;
		/* Update flag */
		if (ugd->cluster_list->b_updated &&
		    !g_strcmp0(album->cluster->uuid, GE_ALBUM_ALL_ID)) {
			ge_dbgW("Update flag: last mtime!");
			ugd->cluster_list->b_updated = false;
			ugd->cluster_list->last_mtime = item->mtime;
		}
		*pgitem = gitem;
		break;
	}

	ge_sdbg("First items of [%s]", album->cluster->display_name);
	return 0;

 DB_FAILED:

	/* Free Mitems */
	if (item_list)
		_ge_data_util_free_mtype_items(&item_list);
	return -1;
}

int _ge_data_get_item_cnt(ge_ugdata *ugd, const char *cluster_id,
			  int album_type, int *item_cnt)
{
	GE_CHECK_VAL(item_cnt, -1);
	GE_CHECK_VAL(ugd, -1);
	int err = -1;
	ge_filter_s filter;
	memset(&filter,0x00,sizeof(ge_filter_s));
	ge_sdbg("cluster_id: %s", cluster_id);

	filter.sort_type = MEDIA_CONTENT_ORDER_DESC;
	g_strlcpy(filter.sort_keyword, GE_CONDITION_ORDER, KEYWORD_LENGTH);
	filter.collate_type = MEDIA_CONTENT_COLLATE_NOCASE;
	filter.offset = GE_GET_ALL_RECORDS;
	filter.count = GE_GET_ALL_RECORDS;
	filter.with_meta = false;
	if (ugd->file_type_mode == GE_FILE_T_IMAGE) {
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE, CONDITION_LENGTH);
	} else if (ugd->file_type_mode == GE_FILE_T_VIDEO) {
		g_strlcpy(filter.cond, GE_CONDITION_VIDEO, CONDITION_LENGTH);
	} else {
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE_VIDEO,
			  CONDITION_LENGTH);
	}

	GE_CHECK_VAL(cluster_id, -1);
	if (album_type == GE_PHONE || album_type == GE_MMC) {
		/*It's normal album*/
		err = _ge_local_data_get_media_count(cluster_id, &filter,
						     item_cnt);
	} else if (album_type == GE_ALL) {
		/* All albums */
		ge_dbg("All albums media count");
		err = _ge_local_data_get_all_media_count(&filter, item_cnt);
	}
	if (err < 0) {
		ge_dbg("Failed to get item count[err: %d]!", err);
		return -1;
	}

	ge_dbg("Item count: %d.", *item_cnt);
	return 0;
}

bool _ge_data_check_update(ge_ugdata *ugd)
{
	GE_CHECK_FALSE(ugd);
	GE_CHECK_FALSE(ugd->cluster_list);
	int err = -1;
	Eina_List *item_list = NULL;
	ge_media_s *mitem = NULL;
	int cnt = 0;
	ge_filter_s filter;

	memset(&filter, 0x00, sizeof(ge_filter_s));
	filter.sort_type = MEDIA_CONTENT_ORDER_DESC;
	g_strlcpy(filter.sort_keyword, GE_CONDITION_ORDER, KEYWORD_LENGTH);
	filter.collate_type = MEDIA_CONTENT_COLLATE_NOCASE;
	filter.with_meta = false;
	filter.offset = GE_FIRST_VIEW_START_POS;
	filter.count = GE_GET_ONE_RECORD;
	if (ugd->file_type_mode == GE_FILE_T_IMAGE)
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE, CONDITION_LENGTH);
	else if (ugd->file_type_mode == GE_FILE_T_VIDEO)
		g_strlcpy(filter.cond, GE_CONDITION_VIDEO, CONDITION_LENGTH);
	else
		g_strlcpy(filter.cond, GE_CONDITION_IMAGE_VIDEO,
			  CONDITION_LENGTH);

	/* Get latest item */
	err = _ge_local_data_get_all_albums_media_list(&filter, &item_list);
	if (err != 0 || item_list == NULL)
		goto DB_NEED_UPDATE;

	mitem = eina_list_nth(item_list, 0);
	/* Compare modified time */
	if (mitem == NULL || mitem->mtime != ugd->cluster_list->last_mtime)
		goto DB_NEED_UPDATE;

	/*  Get all medias count */
	err = _ge_data_get_item_cnt(ugd, GE_ALBUM_ALL_ID, GE_ALL, &cnt);
	if (err != 0)
		goto DB_NEED_UPDATE;

	/* Compare medias count */
	if (cnt != ugd->cluster_list->all_medias_cnt)
		goto DB_NEED_UPDATE;

	_ge_data_type_free_geitem((void **)&mitem);
	mitem = NULL;
	return false;

 DB_NEED_UPDATE:

	_ge_data_type_free_geitem((void **)&mitem);
	mitem = NULL;
	ge_dbgW("Need to update data and view!");
	return true;
}

int _ge_data_del_media_by_id(ge_ugdata* ugd, const char *media_id)
{
	GE_CHECK_VAL(media_id, -1);
	int ret = media_info_delete_from_db(media_id);
	if (ret != 0) {
		ge_dbgE("Delete media failed[%d]!", ret);
		return -1;
	}

	return 0;
}

int _ge_data_free_selected_medias(Eina_List **elist)
{
	if (elist && *elist) {
		ge_item* gitem = NULL;
		EINA_LIST_FREE(*elist, gitem) {
			if (gitem)
				gitem->checked = false;
		}
		*elist = NULL;
	}
	return 0;
}

/*
*   Check it's default album, Camera, Downloads
*/
bool _ge_data_is_default_album(const char *match_folder, ge_album_s *album)
{
	GE_CHECK_FALSE(album);
	GE_CHECK_FALSE(album->display_name);
	GE_CHECK_FALSE(match_folder);
	int ret = -1;

	/* Name is 'Camera shot' and folder locates in Phone */
	ret = g_strcmp0(album->display_name, match_folder);
	if (ret == 0 && album->type == GE_PHONE) {
		ge_dbg("Full path: %s", album->path);
		/* Get parent directory */
		char *parent_path = ge_file_dir_get(album->path);
		GE_CHECK_FALSE(parent_path);
		ge_dbg("Parent path: %s", parent_path);

		/* Parent directory is same as Phone root path, it's default folder */
		ret = g_strcmp0(parent_path, GE_ROOT_PATH_PHONE);
		GE_FREE(parent_path);

		if (ret == 0) {
			ge_dbgW("Default folder!");
			return true;
		}
	}

	return false;
}

/*
*   Check it's default album camera
*/
bool _ge_data_is_camera_album(ge_album_s *mcluster)
{
	GE_CHECK_FALSE(mcluster);
	GE_CHECK_FALSE(mcluster->display_name);

	/* Name is 'Camera' folder locates in Phone */
	if (!g_strcmp0(mcluster->display_name, GE_ALBUM_CAMERA_NAME)) {
		const char *root = NULL;
		if (mcluster->type == GE_PHONE)
			root = GE_ROOT_PATH_PHONE;
		else
			root = GE_ROOT_PATH_MMC;

		ge_dbg("Full path: %s", mcluster->path);
		char *parent_path = ge_file_dir_get(mcluster->path);
		GE_CHECK_FALSE(parent_path);
		ge_dbg("Parent path: %s.", parent_path);

		char *dcim_path = g_strdup_printf("%s/%s", root, GE_DCIM);
		if (dcim_path == NULL) {
			GE_GFREE(parent_path);
			return false;
		}
		/* And parent folder is Phone root path, it's default folder */
		bool ret = false;
		ret = !g_strcmp0(dcim_path, parent_path);

		GE_GFREE(dcim_path);
		GE_GFREE(parent_path);
		return ret;
	}

	return false;
}

bool _ge_data_check_root_type(const char *path, const char *root)
{
	if (path == NULL || root == NULL)
		return false;

	if (!strncmp(root, path, strlen(root))) {
		ge_dbg("Root path: %s", path);
		return true;
	}

	return false;
}

bool _ge_data_is_root_path(const char *path)
{
	GE_CHECK_FALSE(path);

	if (!g_strcmp0(GE_ROOT_PATH_PHONE, path) ||
	   !g_strcmp0(GE_ROOT_PATH_MMC, path)) {
		ge_dbg("Root path: %s", path);
		return true;
	}

	return false;
}

/* Creates a thumbnail image for given the media, asynchronously */
int _ge_data_create_thumb(ge_media_s *item,
			  media_thumbnail_completed_cb callback, void *data)
{
	GE_CHECK_VAL(item, -1);
	GE_CHECK_VAL(item->media_h, -1);
	int ret = -1;
	ge_sdbg("File[%s]", item->file_url);

	ret = media_info_create_thumbnail(item->media_h, callback, data);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Failed to create thumbnail[%d]!", ret);
		return -1;
	}
	item->b_create_thumb = true;
	return 0;
}

int _ge_data_cancel_thumb(ge_media_s *item)
{
	GE_CHECK_VAL(item, -1);
	GE_CHECK_VAL(item->media_h, -1);

	media_info_cancel_thumbnail(item->media_h);
	item->b_create_thumb = false;
	return 0;
}

int _ge_data_restore_selected(Eina_List *sel_ids, ge_item *gitem)
{
	GE_CHECK_VAL(gitem, -1);
	GE_CHECK_VAL(gitem->item, -1);
	ge_dbg("gitem->item->uuid:%s", gitem->item->uuid);
	if (sel_ids) {
		bool b_selected = false;
		b_selected = _ge_data_check_selected_id(sel_ids,
							gitem->item->uuid);
		if (b_selected) {
			ge_dbg("b_selected");
			b_selected = false;
			/* Set checkbox state */
			gitem->checked = true;
		}
	}
	return 0;
}

int _ge_data_init(ge_ugdata* ugd)
{
	ge_dbg("Connect to DB!");
	GE_CHECK_VAL(ugd, -1);
	int err = _ge_local_data_connect();
	if (err != 0) {
		ge_dbgE("Local connection failed[%d]!", err);
		return -1;
	}

	return 0;
}

int _ge_data_finalize(ge_ugdata* ugd)
{
	ge_dbg("Free memory and disconnect with DB!");
	GE_CHECK_VAL(ugd, -1);

	int err = _ge_local_data_disconnect();
	if (err != 0) {
		ge_dbgE("Local disconnection failed[%d]!", err);
		return -1;
	}

	return 0;
}

