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

#include <stdio.h>
#include <media_content.h>
#include <media_info.h>
#include <glib.h>
#include <string.h>
#include "ge-data-util.h"
#include "ge-local-data.h"
#include "ge-debug.h"

static bool __ge_local_data_clone_album(media_folder_h folder, bool b_path,
                                        ge_album_s **palbum)
{
	GE_CHECK_FALSE(folder);
	GE_CHECK_FALSE(palbum);
	ge_album_s *album = NULL;

	album = (ge_album_s *)calloc(1, sizeof(ge_album_s));
	if (album == NULL) {
		ge_dbgE("Failed to calloc!");
		return false;
	}
	album->gtype = GE_TYPE_ALBUM;

	if (b_path) {
		if (media_folder_get_path(folder, &(album->path)) != MEDIA_CONTENT_ERROR_NONE) {
			ge_dbgE("Get folder path failed!");
			goto GE_LOCAL_FAILED;
		}
	}
	if (media_folder_get_folder_id(folder, &(album->uuid)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get folder id failed!");
		goto GE_LOCAL_FAILED;
	}
	if (media_folder_get_modified_time(folder, &(album->mtime)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get modified time failed!");
		goto GE_LOCAL_FAILED;
	}

	media_content_storage_e storage_type;
	if (media_folder_get_storage_type(folder, &storage_type) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get storage type failed!");
		goto GE_LOCAL_FAILED;
	}

	if (storage_type == MEDIA_CONTENT_STORAGE_INTERNAL) { /* The device's internal storage */
		album->type = GE_PHONE;
	} else if (storage_type == MEDIA_CONTENT_STORAGE_EXTERNAL) { /* The device's external storage */
		album->type = GE_MMC;
	} else {
		ge_dbgE("Undefined mode[%d]!", storage_type);
	}

	if (media_folder_get_name(folder, &(album->display_name)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get folder name failed!");
		goto GE_LOCAL_FAILED;
	}

	*palbum = album;
	return true;

GE_LOCAL_FAILED:

	_ge_data_type_free_geitem((void **)(&album));
	*palbum = NULL;
	return false;
}

static bool __ge_local_data_get_album_list_cb(media_folder_h folder,
        void *data)
{
	GE_CHECK_FALSE(data);
	ge_transfer_data_s *tmp_data = (ge_transfer_data_s *)data;
	GE_CHECK_FALSE(tmp_data->userdata);
	GE_CHECK_FALSE(folder);
	Eina_List **elist = (Eina_List **)(tmp_data->userdata);
	ge_album_s *album = NULL;

	char *path = NULL;
	if (media_folder_get_path(folder, &path) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get folder path failed!");
		return false;
	}
	GE_CHECK_FALSE(path);

	if (tmp_data->list_type != GE_ALBUM_LIST_PATH) {
		if (!g_strcmp0(GE_CAMERA_PATH_PHONE, path) ||
		        !g_strcmp0(GE_CAMERA_PATH_MMC, path) ||
		        !g_strcmp0(GE_DOWNLOADS_PATH, path)) {
			ge_dbgW("Camera or Downloads!");
			GE_FREE(path);
			return true;
		}
	}

	__ge_local_data_clone_album(folder, false, &album);
	if (album == NULL) {
		GE_FREE(path);
		ge_dbgE("Failed clone album!");
		return false;
	}
	album->path = path;

	*elist = eina_list_append(*elist, album);
	return true;
}

static bool __ge_local_data_get_media_list_cb(media_info_h media, void *data)
{
	GE_CHECK_FALSE(data);
	ge_transfer_data_s *td = (ge_transfer_data_s *)data;
	GE_CHECK_FALSE(td->userdata);
	GE_CHECK_FALSE(media);
	ge_media_s *item = NULL;

	Eina_List **elist = (Eina_List **)(td->userdata);

	if (_ge_data_util_clone_media(media, &item, td->with_meta)) {
		*elist = eina_list_append(*elist, item);
		return true;
	} else {
		return false;
	}
}

static bool __ge_local_data_get_cover_cb(media_info_h media, void *data)
{
	GE_CHECK_FALSE(data);
	GE_CHECK_FALSE(media);
	Eina_List **elist = (Eina_List **)data;
	ge_media_s *item = NULL;

	item = (ge_media_s *)calloc(1, sizeof(ge_media_s));
	GE_CHECK_FALSE(item);
	item->gtype = GE_TYPE_MEDIA;

	if (media_info_clone(&(item->media_h), media) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Clone media handle error");
		goto GE_LOCAL_FAILED;
	}
	if (media_info_get_file_path(media, &(item->file_url)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media file path error");
		goto GE_LOCAL_FAILED;
	}
	if (media_info_get_media_type(media, (media_content_type_e *)&(item->type)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media type error");
		goto GE_LOCAL_FAILED;
	}
	if (media_info_get_thumbnail_path(media, &(item->thumb_url)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media thumbnail path error");
		goto GE_LOCAL_FAILED;
	}
	ge_sdbg("thumb_url: %s", item->thumb_url);
	*elist = eina_list_append(*elist, item);
	return true;

GE_LOCAL_FAILED:

	_ge_data_type_free_geitem((void **)(&item));
	return false;
}

int _ge_local_data_connect(void)
{
	int ret = -1;

	ret = media_content_connect();
	if (ret == MEDIA_CONTENT_ERROR_NONE) {
		ge_dbg("DB connection is success");
		return 0;
	} else {
		ge_dbgE("DB connection is failed!");
		return -1;
	}
}

int _ge_local_data_disconnect(void)
{
	int ret = -1;

	ret = media_content_disconnect();
	if (ret == MEDIA_CONTENT_ERROR_NONE) {
		ge_dbg("DB disconnection is success");
		return 0;
	} else {
		ge_dbgE("DB disconnection is failed!");
		return -1;
	}
}

int _ge_local_data_get_media_by_id(char *media_id, ge_media_s **mitem)
{
	GE_CHECK_VAL(mitem, -1);

	if (media_id == NULL) {
		//ge_dbg("Create a empty media");
		_ge_data_type_new_media(mitem);
		return 0;
	}

	int ret = -1;
	Eina_List *list = NULL;
	ge_media_s *_mitem = NULL;
	media_info_h media_h = NULL;
	int i = 0;
	ge_sdbg("media id: %s", media_id);

	ret = media_info_get_media_from_db(media_id, &media_h);
	if (ret != MEDIA_CONTENT_ERROR_NONE || media_h == NULL) {
		ge_dbgE("Failed to get media handle[%d]!", ret);
		if (media_h) {
			media_info_destroy(media_h);
		}
		return -1;
	}

	ge_transfer_data_s tran_data;
	memset(&tran_data, 0x00, sizeof(ge_transfer_data_s));
	tran_data.userdata = (void **)&list;
	tran_data.album_id = NULL;
	tran_data.with_meta = false;

	bool b_ret = __ge_local_data_get_media_list_cb(media_h, &tran_data);

	media_info_destroy(media_h);

	if (b_ret && list) {
		*mitem = eina_list_nth(list, 0);
		i = 1;
		ret = 0;
	} else {
		ge_dbgE("Failed to get media list!");
		ret = -1;
	}

	/* Free other items */
	if (list) {
		int len = eina_list_count(list);
		ge_dbg("len: %d", len);

		for (; i < len; i++) {
			_mitem = eina_list_nth(list, i);
			_ge_data_type_free_geitem((void **)(&_mitem));
		}

		eina_list_free(list);
	}

	return ret;
}

int _ge_local_data_get_album_by_path(char *path, ge_album_s **album)
{
	GE_CHECK_VAL(path, -1);
	GE_CHECK_VAL(album, -1);
	int ret = -1;
	Eina_List *list = NULL;
	ge_filter_s condition;
	ge_album_s *_item = NULL;
	int i = 0;

	if (strlen(path) <= 0) {
		ge_dbgE("Invalid path!");
		return -1;
	}
	ge_dbg("path: %s", path);

	memset(&condition, 0x00, sizeof(ge_filter_s));
	condition.collate_type = MEDIA_CONTENT_COLLATE_NOCASE;
	condition.sort_type = MEDIA_CONTENT_ORDER_DESC;
	condition.offset = -1;
	condition.count = -1;
	condition.with_meta = false;
	condition.list_type = GE_ALBUM_LIST_PATH;

	snprintf(condition.cond, CONDITION_LENGTH, "%s AND %s=\'%s\'",
	         GE_CONDITION_IMAGE_VIDEO, FOLDER_PATH, path);

	ret = _ge_local_data_get_album_list(&condition, &list);
	if (ret != 0 || NULL == list) {
		ge_dbgE("Failed to get album list[%d]!", ret);
		ret = -1;
	} else if (NULL != list) {
		*album = eina_list_nth(list, 0);
		i = 1;
		ret = 0;
	}

	/* Free other items */
	if (list) {
		int len = eina_list_count(list);
		ge_dbg("len: %d", len);

		for (; i < len; i++) {
			_item = eina_list_nth(list, i);
			_ge_data_type_free_geitem((void **)(&_item));
		}

		eina_list_free(list);
	}

	return ret;
}

int _ge_local_data_get_album_list(ge_filter_s *condition, Eina_List **elilst)
{
	GE_CHECK_VAL(elilst, -1);
	GE_CHECK_VAL(condition, -1);
	int ret = -1;
	filter_h filter = NULL;

	ret = _ge_data_util_create_filter(condition, &filter);
	if (ret != 0) {
		ge_dbgE("Create filter failed[%d]!", ret);
		return -1;
	}

	ge_transfer_data_s tran_data;
	memset(&tran_data, 0x00, sizeof(ge_transfer_data_s));
	tran_data.userdata = (void **)elilst;
	tran_data.album_id = NULL;
	tran_data.with_meta = false;
	tran_data.list_type = condition->list_type;

	ge_dbg("Get folders--start");
	ret = media_folder_foreach_folder_from_db(filter,
	        __ge_local_data_get_album_list_cb,
	        &tran_data);
	ge_dbg("Get folders--over");

	_ge_data_util_destroy_filter(filter);

	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get folders failed[%d]!", ret);
		return -1;
	}

	return 0;
}

int _ge_local_data_get_media_count(const char *cluster_id, ge_filter_s *condition,
                                   int *item_cnt)
{
	GE_CHECK_VAL(cluster_id, -1);
	GE_CHECK_VAL(condition, -1);
	GE_CHECK_VAL(item_cnt, -1);
	int ret = -1;
	filter_h filter = NULL;

	ret = _ge_data_util_create_filter(condition, &filter);
	if (ret != 0) {
		ge_dbgE("Create filter failed[%d]!", ret);
		return -1;
	}

	ge_dbg("Get media count--start");
	ret = media_folder_get_media_count_from_db(cluster_id, filter,
	        item_cnt);
	ge_dbg("Get media count--over");

	_ge_data_util_destroy_filter(filter);

	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media count failed[d]!", ret);
		return -1;
	}

	return 0;
}

int _ge_local_data_get_all_media_count(ge_filter_s *condtion, int *item_cnt)
{
	GE_CHECK_VAL(condtion, -1);
	GE_CHECK_VAL(item_cnt, -1);
	int ret = -1;
	filter_h filter = NULL;

	ret = _ge_data_util_create_filter(condtion, &filter);
	if (ret != 0) {
		ge_dbgE("Create filter failed[%d]!", ret);
		return -1;
	}

	ge_dbg("Get media count--start");
	ret = media_info_get_media_count_from_db(filter, item_cnt);
	ge_dbg("Get media count--over");

	_ge_data_util_destroy_filter(filter);

	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media count failed[d]!", ret);
		return -1;
	}

	return 0;
}

int _ge_local_data_get_media(const char *media_id, ge_media_s **mitem)
{
	GE_CHECK_VAL(mitem, -1);

	if (media_id == NULL) {
		ge_dbg("Create a empty media");
		_ge_data_type_new_media(mitem);
		return 0;
	}

	int ret = -1;
	Eina_List *list = NULL;
	ge_media_s *_mitem = NULL;
	int i = 0;

	ge_sdbg("media id: %s", media_id);
	media_info_h media_h = NULL;

	ret = media_info_get_media_from_db(media_id, &media_h);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media handle failed[%d]!", ret);
		if (media_h) {
			media_info_destroy(media_h);
		}
		return -1;
	}

	ge_transfer_data_s tran_data;
	memset(&tran_data, 0x00, sizeof(ge_transfer_data_s));
	tran_data.userdata = (void **)&list;
	tran_data.album_id = NULL;
	tran_data.with_meta = false;

	ge_dbg("Get media list--start");
	bool b_ret = __ge_local_data_get_media_list_cb(media_h, &tran_data);
	ge_dbg("Get media list--over");

	media_info_destroy(media_h);

	if (b_ret && list) {
		*mitem = eina_list_nth(list, 0);
		i = 1;
		ret = 0;
	} else {
		ge_dbgE("Failed to get media list!");
		ret = -1;
	}

	/* Free other items */
	if (list) {
		int len = eina_list_count(list);
		ge_dbg("len: %d", len);

		for (; i < len; i++) {
			_mitem = eina_list_nth(list, i);
			_ge_data_type_free_geitem((void **)(&_mitem));
		}

		eina_list_free(list);
	}

	return ret;
}

int _ge_local_data_get_album_cover(char *album_id, ge_filter_s *condition,
                                   Eina_List **elist)
{
	GE_CHECK_VAL(elist, -1);
	GE_CHECK_VAL(album_id, -1);
	GE_CHECK_VAL(condition, -1);
	int ret = -1;
	filter_h filter = NULL;

	ret = _ge_data_util_create_filter(condition, &filter);
	if (ret != 0) {
		ge_dbgE("Create filter failed!");
		return -1;
	}

	ge_dbg("Get medias--start");
	ret = media_folder_foreach_media_from_db(album_id, filter,
	        __ge_local_data_get_cover_cb,
	        elist);
	ge_dbg("Get medias--over");

	_ge_data_util_destroy_filter(filter);

	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get medias failed[d]!", ret);
		return -1;
	}

	return 0;
}

int _ge_local_data_get_all_albums_cover(ge_filter_s *condition,
                                        Eina_List **elist)
{
	GE_CHECK_VAL(elist, -1);
	GE_CHECK_VAL(condition, -1);
	int ret = -1;
	filter_h filter = NULL;

	ret = _ge_data_util_create_filter(condition, &filter);
	if (ret != 0) {
		ge_dbgE("Create filter failed!");
		return -1;
	}

	ge_dbg("Get all medias--start");
	ret = media_info_foreach_media_from_db(filter,
	                                       __ge_local_data_get_cover_cb,
	                                       elist);
	ge_dbg("Get all medias--over");

	_ge_data_util_destroy_filter(filter);

	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get all medias failed[d]!", ret);
		return -1;
	}

	return 0;
}

int _ge_local_data_get_album_media_list(char *album_id, ge_filter_s *condition,
                                        Eina_List **elist)
{
	GE_CHECK_VAL(elist, -1);
	GE_CHECK_VAL(album_id, -1);
	GE_CHECK_VAL(condition, -1);
	int ret = -1;
	filter_h filter = NULL;

	ret = _ge_data_util_create_filter(condition, &filter);
	if (ret != 0) {
		ge_dbgE("Create filter failed!");
		return -1;
	}

	ge_transfer_data_s tran_data;
	memset(&tran_data, 0x00, sizeof(ge_transfer_data_s));
	tran_data.userdata = (void **)elist;
	tran_data.album_id = album_id;
	tran_data.with_meta = condition->with_meta;

	ge_dbg("Get medias--start");
	ret = media_folder_foreach_media_from_db(album_id, filter,
	        __ge_local_data_get_media_list_cb,
	        &tran_data);
	ge_dbg("Get medias--over");

	_ge_data_util_destroy_filter(filter);

	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get medias failed[d]!", ret);
		return -1;
	}

	return 0;
}

int _ge_local_data_get_all_albums_media_list(ge_filter_s *condition,
        Eina_List **elist)
{
	GE_CHECK_VAL(elist, -1);
	GE_CHECK_VAL(condition, -1);
	int ret = -1;
	filter_h filter = NULL;

	ret = _ge_data_util_create_filter(condition, &filter);
	if (ret != 0) {
		ge_dbgE("Create filter failed!");
		return -1;
	}

	ge_transfer_data_s tran_data;
	memset(&tran_data, 0x00, sizeof(ge_transfer_data_s));
	tran_data.userdata = (void **)elist;
	tran_data.album_id = NULL;
	tran_data.with_meta = condition->with_meta;

	ge_dbg("Get all medias--start");
	ret = media_info_foreach_media_from_db(filter,
	                                       __ge_local_data_get_media_list_cb,
	                                       &tran_data);
	ge_dbg("Get all medias--over");

	_ge_data_util_destroy_filter(filter);

	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get all medias failed[d]!", ret);
		return -1;
	}

	return 0;
}

int _ge_local_data_get_album_by_id(char *album_id, ge_album_s **cluster)
{
	GE_CHECK_VAL(cluster, -1);
	GE_CHECK_VAL(album_id, -1);
	media_folder_h folder_h = NULL;

	if (media_folder_get_folder_from_db(album_id, &folder_h) != MEDIA_CONTENT_ERROR_NONE) {
		ge_sdbgE("Failed to get album[%s]!", album_id);
		if (folder_h) {
			media_folder_destroy(folder_h);
		}
		return -1;
	}
	if (folder_h == NULL) {
		return -1;
	}
	if (!__ge_local_data_clone_album(folder_h, true, cluster)) {
		ge_sdbgE("Failed to clone album[%s]!", album_id);
		media_folder_destroy(folder_h);
		return -1;
	}
	media_folder_destroy(folder_h);
	return 0;
}

int _ge_local_data_get_duration(media_info_h media, int *duration)
{
	GE_CHECK_VAL(duration, -1);
	GE_CHECK_VAL(media, -1);
	video_meta_h video_handle = NULL;
	int ret = -1;

	ret = media_info_get_video(media, &video_handle);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Failed to get video handle[%d]!", ret);
		return -1;
	}

	ret = video_meta_get_duration(video_handle, duration);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get video duration failed[%d]!", ret);
		video_meta_destroy(video_handle);
		return -1;
	}

	video_meta_destroy(video_handle);
	return 0;
}

