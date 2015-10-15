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

#include <media_content.h>
#include "gallery-efl.h"
#include "ge-data.h"
#include "ge-local-data.h"
#include "ge-util.h"
#include "ge-debug.h"

#define GE_MONITOE_TIME_DELAY 1.0f

struct ge_db_noti_t {
	Ecore_Timer *db_timer; /*For update db data*/
	/*Make ture calling db update callback after other operations complete*/
	Ecore_Idler *db_idl;
	//media_content_noti_h cloud_h; /* Notify handle fro cloud content updating in DB */
	media_content_db_update_item_type_e update_item;
	media_content_db_update_type_e update_type;
	int count; /* All media count got from DB */
};

static Eina_Bool __ge_db_update_idler(void *data)
{
	GE_CHECK_FALSE(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK_FALSE(ugd->db_noti_d);
	ge_db_noti_s *db_noti = ugd->db_noti_d;
	ge_update_view(ugd, GE_UPDATE_VIEW_INOTIFY);
	evas_object_smart_callback_call(ugd->naviframe,
					"gallery,db,data,updated", ugd);
	GE_IF_DEL_IDLER(db_noti->db_idl);
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __ge_db_update_timer_cb(void *data)
{
	GE_CHECK_FALSE(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK_FALSE(ugd->db_noti_d);
	ge_db_noti_s *db_noti = ugd->db_noti_d;

	GE_IF_DEL_IDLER(db_noti->db_idl);
	db_noti->db_idl = ecore_idler_add(__ge_db_update_idler, data);

	GE_IF_DEL_TIMER(db_noti->db_timer);
	return ECORE_CALLBACK_CANCEL;
}

static int __ge_db_update_add_timer(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->db_noti_d, -1);
	ge_db_noti_s *db_noti = ugd->db_noti_d;

	GE_IF_DEL_TIMER(db_noti->db_timer);
	db_noti->db_timer = ecore_timer_add(GE_MONITOE_TIME_DELAY,
					    __ge_db_update_timer_cb, ugd);
	ge_dbgW("TIMER[1.0f] added!");
	return 0;
}

static int __ge_db_update_op(media_content_error_e error, int pid,
			     media_content_db_update_item_type_e update_item,
			     media_content_db_update_type_e update_type,
			     media_content_type_e media_type, char *uuid,
			     char *path, char *mime_type, void *data)
{
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK_VAL(ugd->db_noti_d, -1);
	ge_db_noti_s *db_noti = ugd->db_noti_d;

	if (MEDIA_CONTENT_ERROR_NONE != error) {
		ge_dbgE("Update db error[%d]!", error);
		return -1;
	}
	if (update_item == MEDIA_ITEM_FILE &&
	    MEDIA_CONTENT_TYPE_IMAGE != media_type &&
	    MEDIA_CONTENT_TYPE_VIDEO != media_type) {
		ge_dbg("Media type is wrong");
		return -1;
	} else if (update_item == MEDIA_ITEM_DIRECTORY && media_type == -1) {
		/* Batch operation, DB wouldn't return media type  */
		int cnt = 0;
		int ret = -1;
		ret = _ge_data_get_item_cnt(ugd, GE_ALBUM_ALL_ID, GE_ALL, &cnt);
		if (ret != 0 || cnt == 0) {
			ge_dbgE("Empty!");
		}
		ge_dbg("old: %d, new: %d", db_noti->count, cnt);
		if (cnt == db_noti->count) {
			ge_dbg("Nothing changed");
			if (path &&
			    !strcmp(path, GE_ROOT_PATH_MMC)) {
				ge_dbg("MMC insert or remove!");
			} else if (path) {
				ge_album_s *album = NULL;
				_ge_local_data_get_album_by_path(path, &album);
				if (!album) {
					return -1;
				} else {
					_ge_data_type_free_geitem((void **)&album);
					ge_dbgW("Updated album contains images");
				}
			} else {
				return -1;
			}
		} else {
			db_noti->count = cnt;
		}
	}

	db_noti->update_item = update_item;
	db_noti->update_type = update_type;

	__ge_db_update_add_timer(ugd);
	return 0;
}

static void __ge_db_update_cb(media_content_error_e error, int pid,
			      media_content_db_update_item_type_e update_item,
			      media_content_db_update_type_e update_type,
			      media_content_type_e media_type, char *uuid,
			      char *path, char *mime_type, void *data)
{
	ge_dbg("update_item[%d], update_type[%d], media_type[%d]", update_item,
	       update_type, media_type);
	GE_CHECK(data);
	ge_dbg("uuid[%s], path[%s]", uuid, path);
	__ge_db_update_op(error, pid, update_item, update_type, media_type,
			  uuid, path, mime_type, data);
}

/*
static void __ge_db_update_coud_cb(media_content_error_e error, int pid,
				   media_content_db_update_item_type_e update_item,
				   media_content_db_update_type_e update_type,
				   media_content_type_e media_type, char *uuid,
				   char *path, char *mime_type, void *data)
{
	ge_dbg("update_item[%d], update_type[%d], media_type[%d]", update_item,
	       update_type, media_type);
	GE_CHECK(data);
	__ge_db_update_op(error, pid, update_item, update_type, media_type,
			  uuid, path, mime_type, data);
}
*/

int _ge_db_update_get_info(void *data,
			   media_content_db_update_item_type_e *update_item,
			   media_content_db_update_type_e *update_type)
{
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK_VAL(ugd->db_noti_d, -1);

	if (update_item)
		*update_item = ugd->db_noti_d->update_item;
	if (update_type)
		*update_type = ugd->db_noti_d->update_type;

	return 0;
}

/*Add media-content update callback*/
bool _ge_db_update_reg_cb(ge_ugdata *ugd)
{
	GE_CHECK_FALSE(ugd);
	/*Init data*/
	ge_db_noti_s *db_noti = (ge_db_noti_s *)calloc(1, sizeof(ge_db_noti_s));
	GE_CHECK_FALSE(db_noti);
	ugd->db_noti_d = db_noti;

	int cnt = 0;
	_ge_data_get_item_cnt(ugd, GE_ALBUM_ALL_ID, GE_ALL, &cnt);
	db_noti->count = cnt;

	int ret = -1;

	ge_dbg("Set db updated callback");
	ret = media_content_set_db_updated_cb(__ge_db_update_cb, ugd);
	if (ret != MEDIA_CONTENT_ERROR_NONE)
		ge_dbgE("Set db updated cb failed[%d]!", ret);
//	ret = media_content_set_db_updated_cloud_cb(&(ugd->db_noti_d->cloud_h),
//						    __ge_db_update_coud_cb,
//						    ugd);
//	if (ret != MEDIA_CONTENT_ERROR_NONE)
//		ge_dbgE("Set db updated cloud cb failed[%d]!", ret);
	return true;
}

bool _ge_db_update_finalize(ge_ugdata *ugd)
{
	int ret = -1;
	ge_dbg("Unset db updated callback");

	ret = media_content_unset_db_updated_cb();
	if (ret != MEDIA_CONTENT_ERROR_NONE)
		ge_dbgE("UNSet db updated cb failed[%d]!", ret);

	GE_CHECK_FALSE(ugd);
	GE_CHECK_FALSE(ugd->db_noti_d);
	ge_db_noti_s *db_noti = ugd->db_noti_d;

//	if (db_noti->cloud_h) {
//		ret = media_content_unset_db_updated_cloud_cb(db_noti->cloud_h);
//		if (ret != MEDIA_CONTENT_ERROR_NONE)
//			ge_dbgE("UNSet db updated cloud cb failed[%d]!", ret);
//		ugd->db_noti_d->cloud_h = NULL;
//	}
	GE_IF_DEL_TIMER(db_noti->db_timer);
	GE_IF_DEL_IDLER(db_noti->db_idl);
	GE_FREE(ugd->db_noti_d);
	return true;
}


