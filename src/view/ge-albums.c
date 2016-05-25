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

#include "ge-albums.h"
#include "ge-gridview.h"
#include "ge-ui-util.h"
#include "ge-util.h"
#include "ge-data.h"
#include "ge-icon.h"
#include "ge-tile.h"
#include "ge-rotate-bg.h"
#include "ge-button.h"
#include "ge-main-view.h"
#include "ge-strings.h"
#include "ge-ugdata.h"
#include "ge-nocontent.h"
#include <notification.h>

static Elm_Gengrid_Item_Class *gic_first = NULL;
static Elm_Gengrid_Item_Class *no_content = NULL;


#define DEFAULT_THUMBNAIL "/opt/usr/share/media/.thumb/thumb_default.png"

/* Only for local medias */
static void __ge_albums_create_thumb_cb(media_content_error_e error,
                                        const char *path, void *user_data)
{
	if (GE_FILE_EXISTS(path) && error == MEDIA_CONTENT_ERROR_NONE) {
		GE_CHECK(user_data);
		ge_cluster *album = (ge_cluster *)user_data;
		album->cover->item->b_create_thumb = false;
		GE_CHECK(album->griditem);
		elm_gengrid_item_update(album->griditem);
	} else {
		ge_dbgE("Error number[%d]", error);
	}
}

/* Use file to create new thumb if possible */
static int __ge_albums_create_thumb(ge_item *gitem, ge_cluster *album)
{
	GE_CHECK_VAL(gitem, -1);
	GE_CHECK_VAL(gitem->item, -1);
	GE_CHECK_VAL(gitem->item->file_url, -1);
	GE_CHECK_VAL(album, -1);

	if (GE_FILE_EXISTS(gitem->item->file_url)) {
		_ge_data_create_thumb(gitem->item, __ge_albums_create_thumb_cb,
		                      album);
		return 0;
	}
	return -1;
}

static void __ge_split_albums_realized(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK(ei);
	Elm_Object_Item *it = (Elm_Object_Item *)ei;
	ge_cluster *album = elm_object_item_data_get(it);
	GE_CHECK(album);
	GE_CHECK(album->cluster);
	GE_CHECK(album->cover);
	GE_CHECK(album->cover->item);
	ge_dbg("realized");
	if (album->select) {
		elm_object_item_signal_emit((Elm_Object_Item *)ei, "elm,state,focused", "elm");
		ugd->selected_griditem = it;
		album->select = false;
	}
	int sel_cnt;
	_ge_data_get_album_sel_cnt(ugd, album->cluster->uuid, &sel_cnt);
	if (sel_cnt > 0) {
		elm_object_item_signal_emit((Elm_Object_Item *)ei,
		                            "elm,state,elm.text.badge,visible",
		                            "elm");
		album->sel_cnt = sel_cnt;
	} else {
		album->sel_cnt = 0;
		elm_object_item_signal_emit((Elm_Object_Item *)ei,
		                            "elm,state,elm.text.badge,hidden",
		                            "elm");
	}
}

static void __ge_albums_realized(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(ei);
	Elm_Object_Item *it = (Elm_Object_Item *)ei;
	ge_cluster *album = elm_object_item_data_get(it);
	GE_CHECK(album);
	GE_CHECK(album->cluster);
	GE_CHECK(album->cover);
	GE_CHECK(album->cover->item);

	ge_dbg("realized");
	if (!GE_FILE_EXISTS(album->cover->item->thumb_url) &&
	        GE_FILE_EXISTS(album->cover->item->file_url) &&
	        (album->cluster->type == GE_PHONE ||
	         album->cluster->type == GE_MMC ||
	         album->cluster->type == GE_ALL)) {
		__ge_albums_create_thumb(album->cover, album);
	}

	GE_CHECK(album->ugd);
	ge_ugdata *ugd = album->ugd;
	if (ugd->b_multifile) {
		if (album->sel_cnt > 0)
			elm_object_item_signal_emit(album->griditem,
			                            "elm,state,elm.text.badge,visible",
			                            "elm");
		else
			elm_object_item_signal_emit(album->griditem,
			                            "elm,state,elm.text.badge,hidden",
			                            "elm");
	}
}

static void __ge_albums_unrealized(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("unrealized");
	GE_CHECK(ei);
	Elm_Object_Item *it = (Elm_Object_Item *)ei;
	ge_cluster *album = elm_object_item_data_get(it);
	GE_CHECK(album);
	GE_CHECK(album->cluster);
	GE_CHECK(album->cover);
	GE_CHECK(album->cover->item);

	/* Checking for local files only */
	if (album->cluster->type == GE_PHONE ||
	        album->cluster->type == GE_MMC ||
	        album->cluster->type == GE_ALL) {
		if (album->cover->item->b_create_thumb) {
			_ge_data_cancel_thumb(album->cover->item);

			ge_dbgW("in the if part");
		}
	}
}

static int __ge_albums_open_album(ge_cluster *album)
{
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(album->ugd, -1);
	ge_ugdata *ugd = album->ugd;
	ge_sdbg("Album: %s", album->cluster->display_name);

	if (_ge_get_view_mode(ugd) != GE_VIEW_ALBUMS) {
		ge_dbgE("Wrong mode!");
		ugd->view_mode = GE_VIEW_ALBUMS;
		//return -1;
		ge_dbgE("new mode album view is assigned");
	}

	if (ugd->album_select_mode == GE_ALBUM_SELECT_T_ONE) {
		ge_dbg("One album selected, return album id");
		app_control_add_extra_data(ugd->service,
		                           GE_ALBUM_SELECT_RETURN_PATH,
		                           album->cluster->path);
		ge_dbg("return folder-path: %s", album->cluster->path);
		ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_SUCCEEDED);
		if (!ugd->is_attach_panel) {
			ug_destroy_me(ugd->ug);
			ugd->ug = NULL;
		}
		return 0;
	}

	/* Add thumbnails view */
	_ge_grid_create_thumb(album);
	ge_dbg("Done");
	return 0;
}

static int __ge_split_albums_open_album(ge_cluster *album)
{
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(album->ugd, -1);
	ge_sdbg("Album: %s", album->cluster->display_name);

	/* Add thumbnails view */
	_ge_grid_create_split_view_thumb(album);
	ge_dbg("Done");
	return 0;
}

/* Add idler to make mouse click sound, other sound couldn' be played */
static Eina_Bool __ge_albums_sel_idler_cb(void *data)
{
	ge_dbg("Select album ---");
	GE_CHECK_FALSE(data);
	ge_cluster *album_item = (ge_cluster*)data;
	GE_CHECK_FALSE(album_item->cluster);
	GE_CHECK_FALSE(album_item->ugd);
	ge_ugdata *ugd = album_item->ugd;
	if (ugd->ug == NULL) {
		ge_dbg("UG already destroyed, return!");
		goto GE_ALBUMS_DONE;
	}

	if (album_item->cover) {
		_ge_data_util_free_item(album_item->cover);
		album_item->cover = NULL;
	}

	__ge_albums_open_album(album_item);

GE_ALBUMS_DONE:

	ecore_idler_del(ugd->sel_album_idler);
	ugd->sel_album_idler = NULL;
	ge_dbg("Select album +++");
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __ge_split_albums_sel_idler_cb(void *data)
{
	ge_dbg("Select album ---");
	GE_CHECK_FALSE(data);
	ge_cluster *album_item = (ge_cluster*)data;
	GE_CHECK_FALSE(album_item->cluster);
	GE_CHECK_FALSE(album_item->ugd);
	ge_ugdata *ugd = album_item->ugd;
	if (ugd->ug == NULL) {
		ge_dbg("UG already destroyed, return!");
		goto GE_ALBUMS_DONE;
	}
	if (album_item->cover) {
		_ge_data_util_free_item(album_item->cover);
		album_item->cover = NULL;
	}
	__ge_split_albums_open_album(album_item);

GE_ALBUMS_DONE:
	ecore_idler_del(ugd->sel_album_idler);
	ugd->sel_album_idler = NULL;
	ge_dbg("Select album +++");
	return ECORE_CALLBACK_CANCEL;
}

static void __ge_albums_sel_cb(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(data);
	ge_cluster *album_item = (ge_cluster*)data;
	GE_CHECK(album_item->cluster);
	GE_CHECK(album_item->ugd);
	ge_ugdata *ugd = album_item->ugd;
	if (ugd->ug == NULL) {
		ge_dbg("UG already destroyed, return!");
		return;
	}
	ugd->album_item = album_item;
	ge_dbg("");
	if (ugd->sel_album_idler) {
		ge_dbg("Has selected an album");
		return;
	}

	elm_gengrid_item_selected_set(ei, EINA_FALSE);

	Ecore_Idler *idl = NULL;
	idl = ecore_idler_add(__ge_albums_sel_idler_cb, data);
	ugd->sel_album_idler = idl;
	/* Save scroller position before clearing gengrid */
	_ge_ui_save_scroller_pos(obj);
}

static void __ge_split_albums_sel_cb(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(data);
	ge_cluster *album_item = (ge_cluster*)data;
	GE_CHECK(album_item->cluster);
	GE_CHECK(album_item->ugd);
	ge_ugdata *ugd = album_item->ugd;
	if (ugd->ug == NULL) {
		ge_dbg("UG already destroyed, return!");
		return;
	}
	ugd->album_item = album_item;
	ge_dbg("");
	if (ugd->sel_album_idler) {
		ge_dbg("Has selected an album");
		return;
	}
	elm_gengrid_item_selected_set(ei, EINA_FALSE);
	Ecore_Idler *idl = NULL;
	idl = ecore_idler_add(__ge_split_albums_sel_idler_cb, data);
	ugd->sel_album_idler = idl;
	/* Save scroller position before clearing gengrid */
	_ge_ui_save_scroller_pos(obj);
}

static char *__ge_split_albums_get_text(void *data, Evas_Object *obj, const char *part)
{
#if 0
	GE_CHECK_NULL(part);
	GE_CHECK_NULL(data);
	ge_cluster *album = (ge_cluster *)data;
	GE_CHECK_NULL(album->cluster);
	GE_CHECK_NULL(album->cluster->uuid);
	GE_CHECK_NULL(album->ugd);
	ge_ugdata *ugd = album->ugd;
	char buf[GE_FILE_PATH_LEN_MAX] = { 0, };

	if (!g_strcmp0(part, "elm.text.name")) {
		GE_CHECK_NULL(album->cluster->display_name);
		int len;
		if (_ge_data_is_root_path(album->cluster->path)) {
			_ge_data_update_items_cnt(ugd, album);
			len = strlen(GE_ALBUM_ROOT_NAME);
			snprintf(buf, sizeof(buf), "%d", (int)(album->cluster->count));
			if (len > 5) {
				snprintf(buf, sizeof(buf), "%.5s... %d", GE_ALBUM_ROOT_NAME, (int)(album->cluster->count));
			} else {
				snprintf(buf, sizeof(buf), "%s %d", GE_ALBUM_ROOT_NAME, (int)(album->cluster->count));
			}
			buf[strlen(buf)] = '\0';
		} else if (album->cluster->display_name &&
		           strlen(album->cluster->display_name)) {
			char *new_name = _ge_ui_get_i18n_album_name(album->cluster);
			_ge_data_update_items_cnt(ugd, album);
			len = strlen(new_name);
			snprintf(buf, sizeof(buf), "%d", (int)(album->cluster->count));
			if (len > 5) {
				snprintf(buf, sizeof(buf), "%.5s... %d", new_name, (int)(album->cluster->count));
			} else {
				snprintf(buf, sizeof(buf), "%s %d", new_name, (int)(album->cluster->count));
			}
			buf[strlen(buf)] = '\0';
		}

		/* Show blue folder name */
		if (!g_strcmp0(album->cluster->uuid, GE_ALBUM_ALL_ID) ||
		        _ge_data_is_camera_album(album->cluster)) {
			Elm_Object_Item *grid_it = album->griditem;
			Evas_Object *it_obj = NULL;
			it_obj = elm_object_item_widget_get(grid_it);
			GE_CHECK_NULL(it_obj);
			edje_object_signal_emit(it_obj, "elm,name,show,blue",
			                        "elm");
			edje_object_message_signal_process(it_obj);
		}
	} else if (!g_strcmp0(part, "elm.text.date")) {
		if (album->cover) {
			_ge_data_util_free_item(album->cover);
			album->cover = NULL;
		}

		ge_item *item = NULL;
		_ge_data_get_album_cover(ugd, album, &item,
		                         MEDIA_CONTENT_ORDER_DESC);
		if (item == NULL || item->item == NULL) {
			album->cover_thumbs_cnt = 0;
			_ge_data_util_free_item(item);
			return NULL;
		}

		album->cover = item;
		album->cover_thumbs_cnt = GE_ALBUM_COVER_THUMB_NUM;
	} else if (!g_strcmp0(part, "elm.text.count")) {
		_ge_data_update_items_cnt(ugd, album);
		snprintf(buf, sizeof(buf), "%d", (int)(album->cluster->count));
		buf[strlen(buf)] = '\0';
	} else if (!g_strcmp0(part, "elm.text.badge") && ugd->b_multifile) {
		int sel_cnt = 0;
		_ge_data_get_album_sel_cnt(ugd, album->cluster->uuid, &sel_cnt);
		ge_dbg("count :%d", sel_cnt);
		if (sel_cnt > 0) {
			elm_object_item_signal_emit(album->griditem,
			                            "elm,state,elm.text.badge,visible",
			                            "elm");
			album->sel_cnt = sel_cnt;
			snprintf(buf, sizeof(buf), "%d", sel_cnt);
		} else {
			album->sel_cnt = 0;
			elm_object_item_signal_emit(album->griditem,
			                            "elm,state,elm.text.badge,hidden",
			                            "elm");
		}
	}
	return strdup(buf);
#endif
	return NULL;
}

static ge_icon_type __ge_albums_set_bg_file(Evas_Object *bg, void *data)
{
	GE_CHECK_VAL(data, -1);
	ge_item *git = (ge_item *)data;
	GE_CHECK_VAL(git->album, -1);
	ge_cluster *album = git->album;
	char *bg_path = GE_ICON_NO_THUMBNAIL;
	ge_icon_type ret_val = GE_ICON_CORRUPTED_FILE;

	if (git == NULL || git->item == NULL) {
		ge_dbgE("Invalid item :%p", git);
		goto GE_ALBUMS_FAILED;
	}

	ret_val = GE_ICON_NORMAL;
	if (GE_FILE_EXISTS(git->item->thumb_url)) {
		bg_path = git->item->thumb_url;
	} else if (album && (album->cluster->type == GE_MMC ||
	                     album->cluster->type == GE_PHONE ||
	                     album->cluster->type == GE_ALL)) {
		__ge_albums_create_thumb(git, album);
	} else {
		ret_val = -1;
	}

GE_ALBUMS_FAILED:

#ifdef _USE_ROTATE_BG_GE
	_ge_rotate_bg_set_image_file(bg, bg_path);
#else
	elm_bg_file_set(bg, bg_path, NULL);
#endif

	return ret_val;
}

static Evas_Object *__ge_split_albums_get_content(void *data, Evas_Object *obj, const char *part)
{
	GE_CHECK_NULL(part);
	GE_CHECK_NULL(strlen(part));
	GE_CHECK_NULL(data);
	ge_cluster *album = (ge_cluster *)data;
	GE_CHECK_NULL(album->cluster);
	GE_CHECK_NULL(album->cluster->uuid);
	ge_dbg("");

	Evas_Object *_obj = NULL;
	if (!g_strcmp0(part, GE_TILE_ICON)) {
		_obj = _ge_tile_show_part_icon(obj, part,
		                               album->cover_thumbs_cnt,
		                               __ge_albums_set_bg_file,
		                               (void *)album->cover);
	}
	return _obj;
}

static int __ge_albums_append(ge_ugdata *ugd, ge_cluster *album)
{
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(ugd, -1);

	album->griditem = elm_gengrid_item_append(ugd->albums_view,
	                  ugd->album_gic, album,
	                  __ge_albums_sel_cb, album);
	ge_sdbg("Append [%s], id[%s]", album->cluster->display_name,
	        album->cluster->uuid);
	_ge_tile_update_item_size(ugd, ugd->albums_view, ugd->rotate_mode,
	                          false);
	album->index = elm_gengrid_items_count(ugd->albums_view);
	return 0;
}

int __ge_split_albums_append(ge_ugdata *ugd, ge_cluster *album)
{
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(ugd, -1);

	album->griditem = elm_gengrid_item_append(ugd->albums_view,
	                  ugd->album_gic, album,
	                  __ge_split_albums_sel_cb, album);
	ge_sdbg("Append [%s], id[%s]", album->cluster->display_name,
	        album->cluster->uuid);
	_ge_tile_update_item_size(ugd, ugd->albums_view, ugd->rotate_mode,
	                          false);
	album->index = elm_gengrid_items_count(ugd->albums_view);
	return 0;
}

static Eina_Bool __ge_albums_append_idler_cb(void *data)
{
	ge_dbg("Append album ---");
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK_VAL(ugd->cluster_list, -1);

	int old_cnt = eina_list_count(ugd->cluster_list->clist);
	_ge_data_get_clusters(ugd, ugd->cluster_list->data_type);
	ugd->cluster_list->data_type = GE_ALBUM_DATA_NONE;
	int new_cnt = eina_list_count(ugd->cluster_list->clist);
	if (old_cnt != new_cnt)
		_ge_tile_update_item_size(ugd, ugd->albums_view,
		                          ugd->rotate_mode, false);
	ecore_idler_del(ugd->album_idler);
	ugd->album_idler = NULL;
	ge_dbg("Append album +++");
	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool __ge_split_albums_append_idler_cb(void *data)
{
	ge_dbg("Append album ---");
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK_VAL(ugd->cluster_list, -1);

	int old_cnt = eina_list_count(ugd->cluster_list->clist);
	ge_dbg("Albums list length old: %d", old_cnt);
	_ge_data_get_clusters(ugd, ugd->cluster_list->data_type);
	ugd->cluster_list->data_type = GE_ALBUM_DATA_NONE;
	int new_cnt = eina_list_count(ugd->cluster_list->clist);
	ge_dbg("Albums list length new: %d", new_cnt);
	ecore_idler_del(ugd->album_idler);
	ugd->album_idler = NULL;
	ge_dbg("Append album +++");
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool __ge_albums_create_idler_cb(void *data)
{
	ge_dbg("Create album ---");
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	int length = eina_list_count(ugd->cluster_list->clist);
	int i = 0;
	ge_cluster *album = NULL;
	ge_dbg("Albums list length: %d", length);

	/* First 8 albums is got from DB, and plus 'All albums', it's 9 totally */
	for (i = GE_ALBUMS_FIRST_COUNT + 1; i < length; i++) {
		album = eina_list_nth(ugd->cluster_list->clist, i);
		__ge_albums_append(ugd, album);
	}
	/* Restore previous position of scroller */
	_ge_ui_restore_scroller_pos(ugd->albums_view);

	ecore_idler_del(ugd->album_idler);
	ugd->album_idler = NULL;
	ge_dbg("Create album +++");
	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool __ge_split_view_albums_create_idler_cb(void *data)
{
	ge_dbg("Create album ---");
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	int length = eina_list_count(ugd->cluster_list->clist);
	int i = 0;
	ge_cluster *album = NULL;
	ge_dbg("Albums list length: %d", length);

	/* First 8 albums is got from DB, and plus 'All albums', it's 9 totally */
	for (i = GE_ALBUMS_FIRST_COUNT + 1; i < length; i++) {
		album = eina_list_nth(ugd->cluster_list->clist, i);
		__ge_split_albums_append(ugd, album);
	}
	/* Restore previous position of scroller */
	_ge_ui_restore_scroller_pos(ugd->albums_view);

	ecore_idler_del(ugd->album_idler);
	ugd->album_idler = NULL;
	ge_dbg("Create album +++");
	return ECORE_CALLBACK_CANCEL;
}

int __ge_split_view_append_albums(ge_ugdata *ugd, Evas_Object *parent, bool is_update)
{
	GE_CHECK_VAL(parent, -1);
	GE_CHECK_VAL(ugd, -1);
	int i = 0;
	ge_cluster *album = NULL;
	int length = 0;
	if (elm_gengrid_items_count(parent) > 0) {
		/* Save scroller position before clearing gengrid */
		_ge_ui_save_scroller_pos(parent);
		elm_gengrid_clear(parent);
	}
	if (is_update) {
		_ge_data_get_clusters(ugd, GE_ALBUM_DATA_NONE);
	}
	if (ugd->cluster_list && ugd->cluster_list->clist) {
		length = eina_list_count(ugd->cluster_list->clist);
	} else {
		return -1;
	}
	ge_dbg("Albums list length: %d", length);

	if (ugd->th) {
		elm_object_theme_set(parent, ugd->th);
	}
	int grid_cnt = 0;
	Elm_Gengrid_Item_Class *gic = elm_gengrid_item_class_new();
	if (gic == NULL) {
		return -1;
	}
	gic->item_style = "gallery_efl/albums_split_view";
	gic->func.text_get = __ge_split_albums_get_text;
	gic->func.content_get = __ge_split_albums_get_content;
	for (i = 0; i < length; i++) {
		album = eina_list_nth(ugd->cluster_list->clist, i);
		if (!album || !album->cluster || !album->cluster->display_name) {
			ge_dbgE("Invalid parameter, return ERROR code!");
			elm_gengrid_clear(parent);

			if (gic) {
				elm_gengrid_item_class_free(gic);
				gic = NULL;
			}
			return -1;
		}

		if (album->cluster->type == GE_ALL) {
			continue;
		}

		album->griditem = elm_gengrid_item_append(parent,
		                  gic,
		                  album,
		                  __ge_split_albums_sel_cb,
		                  album);
		if (!strcmp(album->cluster->uuid, ugd->album_item->cluster->uuid)) {
			album->select = true;
		} else {
			album->select = false;
		}
		album->index = grid_cnt;
		grid_cnt++;
		ge_sdbg("Append [%s], id=%s.", album->cluster->display_name,
		        album->cluster->uuid);
	}
	/* Restore previous position of scroller */
	_ge_ui_restore_scroller_pos(parent);

	if (gic) {
		elm_gengrid_item_class_free(gic);
		gic = NULL;
	}
	/* NOT jsut for updating view, but for updating view and data together */
	if (ugd->cluster_list->data_type == GE_ALBUM_DATA_LOCAL ||
	        ugd->cluster_list->data_type == GE_ALBUM_DATA_WEB) {
		if (ugd->album_idler) {
			ecore_idler_del(ugd->album_idler);
			ugd->album_idler = NULL;
		}
		Ecore_Idler *idl = NULL;
		idl = ecore_idler_add(__ge_split_albums_append_idler_cb, ugd);
		ugd->album_idler = idl;
	}
	if (grid_cnt) {
		return 0;
	} else {
		ge_dbgW("None albums appended to view!");
		return -1;
	}
}

static int __ge_albums_append_albums(ge_ugdata *ugd, Evas_Object *parent, bool is_update)
{
	GE_CHECK_VAL(parent, -1);
	GE_CHECK_VAL(ugd, -1);
	int i = 0;
	ge_cluster *album = NULL;
	int length = 0;

	if (elm_gengrid_items_count(parent) > 0) {
		/* Save scroller position before clearing gengrid */
		_ge_ui_save_scroller_pos(parent);
		elm_gengrid_clear(parent);
	}
	if (is_update) {
		_ge_data_get_clusters(ugd, GE_ALBUM_DATA_NONE);
	}
	if (ugd->cluster_list && ugd->cluster_list->clist) {
		length = eina_list_count(ugd->cluster_list->clist);
	} else {
		return -1;
	}
	ge_dbg("Albums list length: %d", length);

	if (ugd->th) {
		elm_object_theme_set(parent, ugd->th);
	}

	/* Jus for updating view, not updating data and view together */
	if (ugd->cluster_list->data_type == GE_ALBUM_DATA_NONE &&
	        length > GE_ALBUMS_FIRST_COUNT + 1) {
		length = GE_ALBUMS_FIRST_COUNT + 1;
		if (ugd->album_idler) {
			ecore_idler_del(ugd->album_idler);
			ugd->album_idler = NULL;
		}
		Ecore_Idler *idl = NULL;
		idl = ecore_idler_add(__ge_albums_create_idler_cb, ugd);
		ugd->album_idler = idl;
	}

	int grid_cnt = 0;
	for (i = 0; i < length; i++) {
		album = eina_list_nth(ugd->cluster_list->clist, i);
		GE_CHECK_VAL(album, -1);
		GE_CHECK_VAL(album->cluster, -1);
		GE_CHECK_VAL(album->cluster->display_name, -1);

		if (album->cluster->type == GE_ALL) {
			continue;
		}

		album->griditem = elm_gengrid_item_append(parent,
		                  ugd->album_gic,
		                  album,
		                  __ge_albums_sel_cb,
		                  album);
		album->index = grid_cnt;
		grid_cnt++;
		ge_sdbg("Append [%s], id=%s.", album->cluster->display_name,
		        album->cluster->uuid);
	}
	/* Restore previous position of scroller */
	_ge_ui_restore_scroller_pos(parent);

	/* NOT jsut for updating view, but for updating view and data together */
	if (ugd->cluster_list->data_type == GE_ALBUM_DATA_LOCAL ||
	        ugd->cluster_list->data_type == GE_ALBUM_DATA_WEB) {
		if (ugd->album_idler) {
			ecore_idler_del(ugd->album_idler);
			ugd->album_idler = NULL;
		}
		Ecore_Idler *idl = NULL;
		idl = ecore_idler_add(__ge_albums_append_idler_cb, ugd);
		ugd->album_idler = idl;
	}

	if (grid_cnt) {
		_ge_tile_update_item_size(ugd, parent, ugd->rotate_mode, false);
		return 0;
	} else {
		ge_dbgW("None albums appended to view!");
		return -1;
	}
}

static int __ge_albums_del_cbs(Evas_Object *view)
{
	if (view == NULL) {
		return -1;
	}
	ge_dbg("Delete albums callbacks!");
	evas_object_smart_callback_del(view, "realized",
	                               __ge_albums_realized);
	evas_object_smart_callback_del(view, "unrealized",
	                               __ge_albums_unrealized);
	return 0;
}

static void _ge_grid_move_stop_cb(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("");
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	int x, y, w, h;
	int ret;
	elm_scroller_region_get(obj, &x, &y, &w, &h);
	if (ugd->thumbs_d) {
		ugd->thumbs_d->tot_selected = 0;
	}

	app_control_h app_control = NULL;
	ret = app_control_create(&app_control);
	if (ret == APP_CONTROL_ERROR_NONE) {
		if (y == 0) {
			app_control_add_extra_data(app_control, ATTACH_PANEL_FLICK_MODE_KEY, ATTACH_PANEL_FLICK_MODE_ENABLE);
		} else {
			app_control_add_extra_data(app_control, ATTACH_PANEL_FLICK_MODE_KEY, ATTACH_PANEL_FLICK_MODE_DISABLE);
		}
		ret = ug_send_result_full(ugd->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
	}
	app_control_destroy(app_control);
}

Evas_Object* __ge_add_albums_split_view(ge_ugdata *ugd, Evas_Object *parent)
{
	ge_dbg("");
	GE_CHECK_NULL(parent);
	GE_CHECK_NULL(ugd);
	Evas_Object *grid = elm_gengrid_add(parent);
	GE_CHECK_NULL(grid);
	_ge_ui_reset_scroller_pos(grid);
	elm_gengrid_align_set(grid, 0.5f, 0.0);
	elm_gengrid_horizontal_set(grid, EINA_FALSE);
	elm_scroller_bounce_set(grid, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(grid, ELM_SCROLLER_POLICY_OFF,
	                        ELM_SCROLLER_POLICY_AUTO);
	elm_gengrid_multi_select_set(grid, EINA_TRUE);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(grid, "unrealized",
	                               __ge_albums_unrealized, ugd);
	evas_object_smart_callback_add(grid, "realized", __ge_split_albums_realized,
	                               ugd);
	if (__ge_split_view_append_albums(ugd, grid, false) != 0) {
		ge_dbgW("Failed to append album items!");
		__ge_albums_del_cbs(grid);
		evas_object_del(grid);
		grid = NULL;
	} else {
		evas_object_show(grid);
	}
	return grid;
}

static Evas_Object* __ge_albums_add_view(ge_ugdata *ugd, Evas_Object *parent)
{
	ge_dbg("");
	GE_CHECK_NULL(parent);
	Evas_Object *layout = ge_ui_load_edj(parent, GE_EDJ_FILE,
	                                     GE_GRP_ALBUMVIEW);
	GE_CHECK_NULL(layout);
	evas_object_show(layout);
	return layout;
}

static Eina_Bool __ge_main_back_cb(void *data, Elm_Object_Item *it)
{
	ge_dbg("");
	GE_CHECK_FALSE(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	Eina_List *l = NULL;
	ge_item *gitem = NULL;

	void *pop_cb = evas_object_data_get(ugd->naviframe,
	                                    GE_NAVIFRAME_POP_CB_KEY);
	if (pop_cb) {
		Eina_Bool(*_pop_cb)(void * ugd);
		_pop_cb = pop_cb;

		if (_pop_cb(ugd)) {
			/* Just pop edit view, dont destroy me */
			return EINA_FALSE;
		}
	}

	app_control_h app_control = NULL;
	app_control_create(&app_control);
	app_control_add_extra_data(app_control, GE_FILE_SELECT_RETURN_COUNT, "0");
	app_control_add_extra_data(app_control, GE_FILE_SELECT_RETURN_PATH, NULL);
	app_control_add_extra_data(app_control, APP_CONTROL_DATA_SELECTED, NULL);
	app_control_add_extra_data_array(app_control, APP_CONTROL_DATA_PATH, NULL, 0);
	ug_send_result_full(ugd->ug, app_control, APP_CONTROL_RESULT_FAILED);
	app_control_destroy(app_control);

	if (!ugd->is_attach_panel) {
		ug_destroy_me(ugd->ug);
		ge_dbg("ug_destroy_me");
	}

	EINA_LIST_FOREACH(ugd->thumbs_d->medias_elist, l, gitem) {
		if (gitem) {
			gitem->checked = false;
		}
	}

	ugd->selected_elist = eina_list_free(ugd->selected_elist);
	/*If return ture, ug will pop naviframe first.*/
	return EINA_FALSE;
}

void AppGetResource(const char *edj_file_in, char *edj_path_out, int edj_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}

Evas_Object *ge_thumb_show_part_icon_image(Evas_Object *obj, char *path,
        int item_w, int item_h)
{
	GE_CHECK_NULL(obj);

	Evas_Object *layout = elm_layout_add(obj);
	GE_CHECK_NULL(layout);

	Evas_Object *bg = elm_bg_add(layout);
	if (bg == NULL) {
		evas_object_del(layout);
		return NULL;
	}

	elm_bg_file_set(bg, path, NULL);
	elm_bg_load_size_set(bg, item_w, item_h);
	evas_object_size_hint_max_set(bg, item_w, item_h);
	evas_object_size_hint_aspect_set(bg, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);


	elm_layout_theme_set(layout, "gengrid", "photoframe",
	                     "default_layout");
	elm_object_part_content_set(layout, "elm.swallow.icon", bg);

	evas_object_show(layout);
	return layout;
}

static void
__ge_grid_done_cb(void *data, Evas_Object *obj, void *ei)
{
	ge_dbgE("Transferring Data to caller");
	ge_ugdata *ugd = (ge_ugdata*)data;
	GE_CHECK(ugd);

	/* file selection mode */
	char *paths = NULL;
	char **path_array = NULL; /* the array size is sel_cnt */
	int sel_cnt = 0;
	int i = 0;

	_ge_data_get_sel_paths(ugd, &paths, &path_array, &sel_cnt);
	if (sel_cnt <= 0) {
		ge_dbgE("Invalid selected path count!");
		goto GE_SEND_RESULT_FINISHED;
	}

	char t_str[32] = {0,};
	eina_convert_itoa(sel_cnt, t_str);

	app_control_add_extra_data(ugd->service,
	                           GE_FILE_SELECT_RETURN_COUNT, t_str);
	app_control_add_extra_data(ugd->service, GE_FILE_SELECT_RETURN_PATH,
	                           paths);
	app_control_add_extra_data_array(ugd->service, APP_CONTROL_DATA_SELECTED,
	                                 (const char **)path_array, sel_cnt);
	app_control_add_extra_data_array(ugd->service, APP_CONTROL_DATA_PATH,
	                                 (const char **)path_array, sel_cnt);
	ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_SUCCEEDED);
	ugd->selected_elist = eina_list_free(ugd->selected_elist);

GE_SEND_RESULT_FINISHED:

	if (paths) {
		g_free(paths);
		paths = NULL;
	}
	if (path_array) {
		for (i = 0; i < sel_cnt; i++) {
			GE_FREEIF(path_array[i]);
		}
		GE_FREE(path_array);
	}

	if (!ugd->is_attach_panel) {
		ug_destroy_me(ugd->ug);
	}
}

static void
__ge_check_state_changed_cb(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(obj);
	GE_CHECK(data);
	ge_item *gitem = (ge_item *)data;
	GE_CHECK(gitem->item);
	GE_CHECK(gitem->item->file_url);
	GE_CHECK(gitem->ugd);
	ge_ugdata *ugd = gitem->ugd;
	GE_CHECK(ugd->service);
	GE_CHECK(ugd->thumbs_d);
	Eina_List *l = NULL;
	ge_sel_item_s *sit = NULL;
	ge_sel_item_s *sit1 = NULL;
	Eina_Bool state = elm_check_state_get(obj);

	sit = _ge_data_util_new_sel_item(gitem);
	GE_CHECK(sit);
#ifdef FEATURE_SIZE_CHECK
	struct stat stFileInfo;
	stat(sit->file_url, &stFileInfo);
#endif
	if (state == EINA_TRUE) {
		if (!g_strcmp0(gitem->item->thumb_url, DEFAULT_THUMBNAIL)) {
			elm_check_state_set(obj, EINA_FALSE);
			char *pStrWarning = g_strdup_printf(
			                        GE_STR_UNSUPPORTED_FILE);
			notification_status_message_post(pStrWarning);
			GE_FREE(pStrWarning);
			_ge_data_util_free_sel_item(sit);
			return;
		}
		if ((ugd->max_count != -1) && (gitem->ugd->thumbs_d->tot_selected + 1 > ugd->max_count)) {
			elm_check_state_set(obj, EINA_FALSE);
			char *noti_str = GE_STR_MAX_PICTURES_SELECTED;
			char *pStrWarning = g_strdup_printf(
			                        noti_str,
			                        ugd->max_count);
			notification_status_message_post(pStrWarning);
			GE_FREE(pStrWarning);
			_ge_data_util_free_sel_item(sit);
			return;
		}
#ifdef FEATURE_SIZE_CHECK
		if (ugd->limitsize > 0 && ugd->selsize + stFileInfo.st_size > ugd->limitsize) {
			elm_check_state_set(obj, EINA_FALSE);
			notification_status_message_post("maximum of 2MB can be selected");
			_ge_data_util_free_sel_item(sit);
			return;
		}
#endif
		ge_sdbg("append Path: %s", sit->file_url);
		ugd->selected_elist = eina_list_append(ugd->selected_elist, sit);
		ugd->thumbs_d->tot_selected++;
#ifdef FEATURE_SIZE_CHECK
		ugd->selsize += stFileInfo.st_size;
#endif
		gitem->checked = TRUE;
	} else {
		ge_sdbg("remove Path: %s", sit->file_url);
		EINA_LIST_FOREACH(ugd->selected_elist, l, sit1) {
			if (sit1 && !strcmp(sit->file_url, sit1->file_url)) {
				ugd->selected_elist = eina_list_remove(ugd->selected_elist, sit1);
				_ge_data_util_free_sel_item(sit1);
			}
		}
		ugd->thumbs_d->tot_selected--;
#ifdef FEATURE_SIZE_CHECK
		ugd->selsize -= stFileInfo.st_size;
#endif
		gitem->checked = FALSE;
		_ge_data_util_free_sel_item(sit);
	}

	char *pd_selected = GE_STR_PD_SELECTED;
	char *text = NULL;
	Evas_Object *btn = NULL;

	btn = elm_object_item_part_content_get(ugd->nf_it , GE_NAVIFRAME_TITLE_RIGHT_BTN);
	if (btn == NULL) {
		ge_dbgE("Failed to get part information");
	}

	if (ugd->thumbs_d->tot_selected == 0) {
		elm_object_disabled_set(btn, EINA_TRUE);
	} else {
		elm_object_disabled_set(btn, EINA_FALSE);
	}

	/* Update the label text */
	if (ugd->thumbs_d->tot_selected >= 0) {
		text = g_strdup_printf(pd_selected, ugd->thumbs_d->tot_selected);
		elm_object_item_text_set(ugd->nf_it, text);
	}
}

static Evas_Object *
__ge_nocontent_get(void *data, Evas_Object *obj, const char *part)
{
	ge_ugdata *ugd = (ge_ugdata *)data;

	Evas_Object *layout = ge_ui_load_edj(ugd->albums_view, GE_EDJ_FILE,
	                                     "nocontent");
	ge_dbg("Nocontents label: %s", GE_STR_NO_ITEMS);
	/* Full view nocontents */
	Evas_Object *noc_lay = elm_layout_add(layout);
	GE_CHECK_NULL(noc_lay);

	elm_layout_theme_set(noc_lay, "layout", "nocontents", "text");

	_ge_ui_set_translate_part_str(noc_lay, "elm.text", GE_STR_NO_ITEMS);
	_ge_ui_set_translate_part_str(noc_lay, "elm.help.text", GE_STR_NO_ITEMS_HELP_TEXT);

	elm_layout_signal_emit(noc_lay, "text,disabled", "");
	elm_layout_signal_emit(noc_lay, "align.center", "elm");
	elm_object_part_content_set(layout, "swallow", noc_lay);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	GE_CHECK_NULL(layout);
	evas_object_show(layout);
	return layout;

}


static Evas_Object *
__ge_gengrid_item_content_get(void *data, Evas_Object *obj, const char *part)
{
	ge_item *gitem = NULL;

	if (strlen(part) <= 0) {
		ge_dbgE("part length <= 0");
		return NULL;
	}

	gitem = (ge_item*)data;
	GE_CHECK_NULL(gitem->item);
	GE_CHECK_NULL(gitem->ugd);
	ge_ugdata *ugd = gitem->ugd;
	GE_CHECK_NULL(ugd->thumbs_d);

	Evas_Object *layout = NULL;
	char *path = NULL;
	char *sd_card_image_path = GE_ICON_MEMORY_CARD;
	Evas_Object *icon;

	if (!g_strcmp0(part, "content_swallow")) {

		/* Use default image */
		if (!g_strcmp0(gitem->item->thumb_url, DEFAULT_THUMBNAIL)) {
			path = GE_ICON_CONTENTS_BROKEN;
		} else {
			path = gitem->item->thumb_url;
		}

		if (gitem->item->type == MEDIA_CONTENT_TYPE_IMAGE) {
			layout = ge_thumb_show_part_icon_image(obj, path,
			                                       ugd->thumbs_d->icon_w,
			                                       ugd->thumbs_d->icon_h);
		}


		return layout;

	} else if (!g_strcmp0(part, "checkbox_swallow") && ugd->thumbs_d->b_editmode == true) {
		Evas_Object* ck = NULL;
		ck = elm_check_add(obj);
		GE_CHECK_NULL(ck);

		evas_object_propagate_events_set(ck, EINA_FALSE);
		evas_object_repeat_events_set(ck, EINA_FALSE);
		elm_check_state_set(ck, gitem->checked);
		ugd->thumbs_d->check = ck;
		evas_object_smart_callback_add(ck, "changed",
		                               __ge_check_state_changed_cb, gitem);
		evas_object_show(ck);

		return ck;
	} else if (!g_strcmp0(part, "sd_card_icon")) {
		if (gitem->item->storage_type == (media_content_storage_e)GE_MMC) {
			icon = elm_icon_add(obj);
			elm_image_file_set(icon, sd_card_image_path, NULL);
			evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			return icon;
		}
	}

	return layout;
}

static char*
__ge_gengrid_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	return strdup(GE_STR_GALLERY);
}

static void
__ge_gallery_ug_result_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data)
{
	ge_ugdata *ugd = (ge_ugdata *)data;
	char **pathArray = NULL;
	int arrayLength = 0;
	int i = 0;

	if ((result != APP_CONTROL_RESULT_SUCCEEDED) || !reply) {
		ge_dbgE("ug-gallery-efl data get failed.");
		return;
	}

	app_control_get_extra_data_array(reply, APP_CONTROL_DATA_PATH, &pathArray, &arrayLength);

	if (arrayLength != 0) {
		ge_dbg("Receiving data from ug-gallery-efl.");
		char t_str[32] = { 0, };
		eina_convert_itoa(arrayLength, t_str);

		app_control_add_extra_data(ugd->service,
		                           GE_FILE_SELECT_RETURN_COUNT, t_str);
		app_control_add_extra_data_array(ugd->service, APP_CONTROL_DATA_SELECTED,
		                                 (const char **)pathArray, arrayLength);
		app_control_add_extra_data_array(ugd->service, APP_CONTROL_DATA_PATH,
		                                 (const char **)pathArray, arrayLength);
		ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_SUCCEEDED);

		if (pathArray) {
			for (i = 0; i < arrayLength; i++) {
				GE_FREEIF(pathArray[i]);
			}
			GE_FREE(pathArray);
		}

		if (!ugd->is_attach_panel) {
			ug_destroy_me(ugd->ug);
		}
	}
}

static void
__ge_launch_ug_gallery(void *data, Evas_Object *obj, void *ei)
{
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK(ugd);

	char maximum_count [10];
	snprintf(maximum_count, 10, "%d", ugd->max_count);

	elm_gengrid_item_selected_set(ei, EINA_FALSE);

	app_control_h svc_handle = NULL;

	if (!app_control_create(&svc_handle)) {
		app_control_set_operation(svc_handle, APP_CONTROL_OPERATION_PICK);
		app_control_set_launch_mode(svc_handle, APP_CONTROL_LAUNCH_MODE_GROUP);
		app_control_set_app_id(svc_handle,  "org.tizen.ug-gallery-efl");
		app_control_set_mime(svc_handle, "image/*");
		app_control_add_extra_data(svc_handle, APP_CONTROL_DATA_TOTAL_COUNT, maximum_count);
		app_control_add_extra_data(svc_handle, "launch-type", "select-multiple");
		app_control_add_extra_data(svc_handle, "file-type", "image");
		app_control_add_extra_data(svc_handle, "hide-personal", "true");
		app_control_send_launch_request(svc_handle, __ge_gallery_ug_result_cb, data);
		int ret = app_control_destroy(svc_handle);

		if (ret == 0) {
			ge_dbg("Launched ug-gallery-efl successfully.");
		} else {
			ge_dbgE("Launching ug-gallery-efl Failed.");
		}
	} else {
		ge_dbgE("app_control_service_handle creation failed. Unable to Launch ug-gallery-efl.");
	}
}

static int
__ge_grid_select_one(ge_item *gitem, char *file_url)
{
	GE_CHECK_VAL(file_url, -1);
	GE_CHECK_VAL(gitem, -1);
	ge_ugdata *ugd = (ge_ugdata *)gitem->ugd;
	GE_CHECK_VAL(ugd, -1);
	ge_sdbg("Service add:%s", file_url);

	int ret = -1;

	if (!g_strcmp0(gitem->item->thumb_url, DEFAULT_THUMBNAIL)) {
		char *pStrWarning = g_strdup_printf(GE_STR_UNSUPPORTED_FILE);
		notification_status_message_post(pStrWarning);
		GE_FREE(pStrWarning);
		return ret;
	}

	char **path_array = (char **)calloc(1, sizeof(char *));
	if (!path_array) {
		ge_dbgW("failed to allocate path");
		return ret;
	}
	path_array[0] = strdup(file_url);

	ret = app_control_add_extra_data(ugd->service, GE_FILE_SELECT_RETURN_PATH, file_url);

	if (ret != APP_CONTROL_ERROR_NONE) {
		ge_dbgW("Add file path failed!");
	}
	ret = app_control_add_extra_data_array(ugd->service, APP_CONTROL_DATA_SELECTED,
	                                       (const char **)path_array, 1);
	ret = app_control_add_extra_data_array(ugd->service, APP_CONTROL_DATA_PATH,
	                                       (const char **)path_array, 1);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ge_dbgW("Add selected path failed!");
	}
	ret = ug_send_result_full(ugd->ug, ugd->service,
	                          APP_CONTROL_RESULT_SUCCEEDED);
	if (ret != 0) {
		ge_dbgW("Send result failed!");
	}
	if (!ugd->is_attach_panel) {
		ug_destroy_me(ugd->ug);
	}

	GE_FREEIF(path_array[0]);
	GE_FREEIF(path_array);
	return ret;
}

static void
__ge_gengrid_item_sel_cb(void *data, Evas_Object *obj, void *ei)
{

	GE_CHECK(data);
	ge_item *gitem = (ge_item*)data;
	GE_CHECK(gitem->item);
	GE_CHECK(gitem->ugd);
	ge_ugdata *ugd = gitem->ugd;
	char *pd_selected = GE_STR_PD_SELECTED;
	char *text = NULL;
	Evas_Object *btn = NULL;
	Eina_List *l = NULL;
	ge_sel_item_s *sit = NULL;
	ge_sel_item_s *sit1 = NULL;

	elm_gengrid_item_selected_set(ei, EINA_FALSE);

	if (ugd->file_select_mode == GE_FILE_SELECT_T_ONE) {
		int ret = __ge_grid_select_one(gitem, gitem->item->file_url);

		if (ret != 0) {
			ge_dbgE("Data Transfer Failed.");
		}
	} else if (ugd->file_select_mode == GE_FILE_SELECT_T_MULTIPLE) {
		Evas_Object *ck = elm_object_item_part_content_get((Elm_Object_Item *) ei, "checkbox_swallow");

		Eina_Bool bl = elm_check_state_get(ck);

		if (bl == EINA_FALSE) {
			if (!g_strcmp0(gitem->item->thumb_url, DEFAULT_THUMBNAIL)) {
				char *pStrWarning = g_strdup_printf(
				                        GE_STR_UNSUPPORTED_FILE);
				notification_status_message_post(pStrWarning);
				GE_FREE(pStrWarning);
				return;
			}
			if ((ugd->max_count != -1) && (gitem->ugd->thumbs_d->tot_selected + 1 > ugd->max_count)) {
				char *noti_str = GE_STR_MAX_PICTURES_SELECTED;
				char *pStrWarning = g_strdup_printf(noti_str,
				                        ugd->max_count);
				notification_status_message_post(pStrWarning);
				GE_FREE(pStrWarning);
				return;
			}
			sit = _ge_data_util_new_sel_item(gitem);
			if (!sit) {
				ge_dbgE("Invalid select item");
				return;
			}
#ifdef FEATURE_SIZE_CHECK
			struct stat stFileInfo;
			stat(sit->file_url, &stFileInfo);
			if (ugd->limitsize > 0 && ugd->selsize + stFileInfo.st_size > ugd->limitsize) {
				notification_status_message_post("maximum of 2MB can be selected");
				_ge_data_util_free_sel_item(sit);
				return;
			}
#endif
			ge_sdbg("append Path: %s", sit->file_url);
			ugd->selected_elist = eina_list_append(ugd->selected_elist, sit);
			elm_check_state_set(ck, EINA_TRUE);
			gitem->ugd->thumbs_d->tot_selected++;
#ifdef FEATURE_SIZE_CHECK
			ugd->selsize += stFileInfo.st_size;
#endif
			gitem->checked = TRUE;
		} else {
			sit = _ge_data_util_new_sel_item(gitem);
			if (!sit) {
				ge_dbgE("Invalid select item");
				return;
			}
#ifdef FEATURE_SIZE_CHECK
			struct stat stFileInfo;
			stat(sit->file_url, &stFileInfo);
#endif
			ge_sdbg("remove Path: %s", sit->file_url);
			EINA_LIST_FOREACH(ugd->selected_elist, l, sit1) {
				if (sit1 && !strcmp(sit->file_url, sit1->file_url)) {
					ugd->selected_elist = eina_list_remove(ugd->selected_elist, sit1);
					_ge_data_util_free_sel_item(sit1);
				}
			}
			elm_check_state_set(ck, EINA_FALSE);
			gitem->ugd->thumbs_d->tot_selected--;
#ifdef FEATURE_SIZE_CHECK
			ugd->selsize -= stFileInfo.st_size;
#endif
			gitem->checked = FALSE;
			_ge_data_util_free_sel_item(sit);
		}

		btn = elm_object_item_part_content_get(ugd->nf_it, GE_NAVIFRAME_TITLE_RIGHT_BTN);
		if (btn == NULL) {
			ge_dbgE("Failed to get part information");
		}

		if (ugd->thumbs_d->tot_selected == 0) {
			elm_object_disabled_set(btn, EINA_TRUE);
		} else {
			elm_object_disabled_set(btn, EINA_FALSE);
		}

		if (gitem->ugd->thumbs_d->tot_selected >= 0) {
			text = g_strdup_printf(pd_selected, gitem->ugd->thumbs_d->tot_selected);
			elm_object_item_text_set(gitem->ugd->nf_it, text);
		}
	}
}

static void
__ge_cancel_cb(void *data, Evas_Object *obj, void *ei)
{
	ge_ugdata *app_data = (ge_ugdata *)data;
	Eina_List *l = NULL;
	ge_item *gitem = NULL;

	if (app_data->is_attach_panel && (app_data->attach_panel_display_mode == ATTACH_PANEL_FULL_MODE)) {

		int ret;
		app_control_h app_control = NULL;
		ret = app_control_create(&app_control);
		if (ret == APP_CONTROL_ERROR_NONE) {
			app_control_add_extra_data(app_control, ATTACH_PANEL_FLICK_MODE_KEY, ATTACH_PANEL_FLICK_MODE_ENABLE);
			app_control_add_extra_data_array(app_control, APP_CONTROL_DATA_PATH,
			                                 NULL, 0);
			/*ret = app_control_add_extra_data(app_control, "__ATTACH_PANEL_SHOW_PANEL__", "false");

			if (ret != APP_CONTROL_ERROR_NONE) {
				ge_dbgW("Attach panel show failed");
			}*/
			ret = ug_send_result_full(app_data->ug, app_control, APP_CONTROL_RESULT_FAILED);
		}
		app_control_destroy(app_control);
	}

	EINA_LIST_FOREACH(app_data->thumbs_d->medias_elist, l, gitem) {
		if (gitem) {
			gitem->checked = false;
		}
	}

	app_data->selected_elist = eina_list_free(app_data->selected_elist);
	elm_naviframe_item_pop(app_data->naviframe);
}

int
ge_update_gengrid(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->thumbs_d, -1);

	int ret = 0;
	int i = 0;
	ge_item* gitem = NULL;
	int item_cnt = 0;
	char *pd_selected = GE_STR_PD_SELECTED;
	ge_sel_item_s *sit = NULL;
	Eina_List *l = NULL;
	int win_x = 0;
	int win_y = 0;
	int win_w = 0;
	int win_h = 0;

	elm_win_screen_size_get(ugd->win, &win_x, &win_y, &win_w, &win_h);

	int size = (win_w / 4);

	if (ugd->rotate_mode == GE_ROTATE_LANDSCAPE_UPSIDEDOWN || ugd->rotate_mode == GE_ROTATE_LANDSCAPE) {
		size = (win_h / 7);
	}

	if (ugd->thumbs_d->medias_elist) {
		_ge_data_util_free_mtype_items(&ugd->thumbs_d->medias_elist);
	}

	ugd->thumbs_d->medias_cnt = 0;

	ugd->thumbs_d->tot_selected = 0;

	ret = _ge_data_get_medias(ugd, ugd->thumbs_d->album_uuid,
	                          GE_ALL,
	                          0,
	                          GE_GET_UNTIL_LAST_RECORD,
	                          &(ugd->thumbs_d->medias_elist),
	                          NULL, NULL);

	if (ret != 0) {
		ge_dbgE("###Get items list over[%d]###", ret);
		return ret;
	}

	ugd->thumbs_d->medias_cnt = eina_list_count(ugd->thumbs_d->medias_elist);
	ge_dbg("Grid view updated media count: %d", ugd->thumbs_d->medias_cnt);

	if (ugd->thumbs_d->medias_cnt == 0) {
		_ge_data_util_free_mtype_items(&ugd->selected_elist);
	} else {
		EINA_LIST_FOREACH(ugd->selected_elist, l, sit) {
			bool flag = false;
			for (i = 0; i < ugd->thumbs_d->medias_cnt; i++) {
				gitem = eina_list_nth(ugd->thumbs_d->medias_elist, i);
				if (gitem == NULL || gitem->item == NULL ||
						gitem->item->uuid == NULL) {
					ge_dbgE("Invalid gitem!");
					continue;
				}
				if (sit && strcmp(sit->file_url, gitem->item->file_url) == 0) {
					flag = true;
					break;
				}
			}
			if (flag == false) {
				ugd->selected_elist = eina_list_remove(ugd->selected_elist, sit);
				_ge_data_util_free_sel_item(sit);
			}
		}
	}

	elm_gengrid_clear(ugd->thumbs_d->gengrid);
	if (ugd->thumbs_d->medias_cnt > 0) {
		if (ugd->nocontents) {
			evas_object_del(ugd->nocontents);
			ugd->nocontents = NULL;
		}
		elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, size, size);
		elm_gengrid_item_append(ugd->thumbs_d->gengrid,
		                        gic_first,
		                        NULL, __ge_launch_ug_gallery,
		                        ugd);

		for (i = 0; i < ugd->thumbs_d->medias_cnt; i++) {
			gitem = eina_list_nth(ugd->thumbs_d->medias_elist, i);
			if (gitem == NULL || gitem->item == NULL ||
			        gitem->item->uuid == NULL) {
				ge_dbgE("Invalid gitem!");
				continue;
			}

			if (!gitem->item->file_url) {
				ge_dbgE("file_url is invalid!");
				_ge_data_del_media_by_id(ugd, gitem->item->uuid);
				_ge_data_util_free_item(gitem);
				ugd->thumbs_d->medias_cnt--;
				i--;
				gitem = NULL;
				continue;
			}

			if (ugd->selected_elist) {
				EINA_LIST_FOREACH(ugd->selected_elist, l, sit) {
					if (sit && strcmp(sit->file_url, gitem->item->file_url) == 0) {
						gitem->checked = true;
						ugd->thumbs_d->tot_selected++;
					}
				}
			}

			gitem->ugd = ugd;
			gitem->elm_item = elm_gengrid_item_append(ugd->thumbs_d->gengrid,
			                  ugd->thumbs_d->gic,
			                  gitem, __ge_gengrid_item_sel_cb,
			                  gitem);

			item_cnt++;
			gitem->sequence = item_cnt + 1;
		}
	} else {
		if (ugd->attach_panel_display_mode == ATTACH_PANEL_FULL_MODE) {
			elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, win_w, FULL_MODE_PORTRAIT_HEIGHT);
		} else {
			elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, win_w, HALF_MODE_PORTRAIT_HEIGHT);
		}

		ugd->nocontents = _ge_nocontents_create(ugd);
		elm_gengrid_item_append(ugd->thumbs_d->gengrid,
		                        no_content	,
		                        ugd, NULL,
		                        ugd);
	}

	Evas_Object *btn = NULL;

	btn = elm_object_item_part_content_get(ugd->nf_it , GE_NAVIFRAME_TITLE_RIGHT_BTN);
	if (btn == NULL) {
		ge_dbgE("Failed to get part information");
	}

	if (ugd->thumbs_d->tot_selected == 0) {
		elm_object_disabled_set(btn, EINA_TRUE);
	} else {
		elm_object_disabled_set(btn, EINA_FALSE);
	}


	char *text = g_strdup_printf(pd_selected, ugd->thumbs_d->tot_selected);
	elm_object_item_text_set(ugd->nf_it, text);

	return 0;
}

int
ge_create_gengrid(ge_ugdata *ugd)
{
	ge_dbgE("");
	GE_CHECK_VAL(ugd, -1);

	ugd->albums_view = __ge_albums_add_view(ugd, ugd->albums_view_ly);

	ugd->attach_panel_display_mode = ATTACH_PANEL_HALF_MODE;

	ge_dbgE("!!!!! album view !!!!!!!! is pushed in the naviframe instead of content set");

	int i = 0;
	ge_item* gitem = NULL;
	int item_cnt = 0;
	int ret = -1;

	ge_thumbs_s *thumbs_d = NULL;
	thumbs_d = (ge_thumbs_s *)calloc(1, sizeof(ge_thumbs_s));
	GE_CHECK_VAL(thumbs_d, -1);
	ugd->thumbs_d = thumbs_d;
	ugd->thumbs_d->store_type = GE_ALL;
	ugd->thumbs_d->b_multifile = true;
	ugd->thumbs_d->b_mainview = true;
	ugd->thumbs_d->b_editmode = false;
	ugd->thumbs_d->tot_selected = 0;

	ugd->attach_panel_display_mode = ATTACH_PANEL_HALF_MODE;
	ugd->file_select_mode = GE_FILE_SELECT_T_ONE;

	ret = _ge_data_get_medias(ugd, ugd->thumbs_d->album_uuid,
	                          GE_ALL,
	                          0,
	                          GE_GET_UNTIL_LAST_RECORD,
	                          &(ugd->thumbs_d->medias_elist),
	                          NULL, NULL);

	if (ret != 0) {
		ge_dbgE("###Get items list over[%d]###", ret);
	}

	ugd->thumbs_d->medias_cnt = eina_list_count(ugd->thumbs_d->medias_elist);
	ge_dbg("Grid view all medias count: %d", ugd->thumbs_d->medias_cnt);

	Evas_Object *gengrid;
	char edj_path[PATH_MAX] = {0, };


	AppGetResource(GE_EDJ_FILE, edj_path, (int)PATH_MAX);

	gengrid = elm_gengrid_add(ugd->albums_view);
	ugd->thumbs_d->gengrid = gengrid;
	elm_theme_extension_add(NULL, edj_path);

	int win_x = 0;
	int win_y = 0;
	int win_w = 0;
	int win_h = 0;

	elm_win_screen_size_get(ugd->win, &win_x, &win_y, &win_w, &win_h);
	int size = (win_w / 4);

	_ge_ui_reset_scroller_pos(gengrid);
	elm_gengrid_align_set(gengrid, 0.0, 0.0);
#ifdef _USE_SCROL_HORIZONRAL
	/* horizontal scrolling */
	elm_gengrid_horizontal_set(gengrid, EINA_TRUE);
	elm_scroller_bounce_set(gengrid, EINA_TRUE, EINA_FALSE);
#else
	elm_gengrid_horizontal_set(gengrid, EINA_FALSE);
	elm_scroller_bounce_set(gengrid, EINA_FALSE, EINA_TRUE);
#endif
	elm_scroller_policy_set(gengrid, ELM_SCROLLER_POLICY_OFF,
	                        ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (ugd->is_attach_panel && (ugd->attach_panel_display_mode != ATTACH_PANEL_FULL_MODE)) {
		elm_scroller_movement_block_set(gengrid, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
	}

	if (ugd->is_attach_panel) {
		evas_object_smart_callback_add(gengrid, "scroll,anim,stop",
		                               _ge_grid_move_stop_cb, ugd);
		evas_object_smart_callback_add(gengrid, "scroll,drag,stop",
		                               _ge_grid_move_stop_cb, ugd);
	}

	ugd->thumbs_d->gic = elm_gengrid_item_class_new();

	if (ugd->thumbs_d->gic) {
		ugd->thumbs_d->gic->item_style = "gengrid_items";
		ugd->thumbs_d->gic->func.text_get = NULL;
		ugd->thumbs_d->gic->func.content_get = __ge_gengrid_item_content_get;
		ugd->thumbs_d->gic->func.state_get = NULL;
		ugd->thumbs_d->gic->func.del = NULL;
	}

	gic_first = elm_gengrid_item_class_new();

	if (gic_first) {
		gic_first->item_style = "gengrid_first_item";
		gic_first->func.text_get = __ge_gengrid_text_get_cb;
		gic_first->func.content_get = NULL;
		gic_first->func.state_get = NULL;
		gic_first->func.del = NULL;

	}

	no_content = elm_gengrid_item_class_new();

	if (no_content) {
		no_content->item_style = "no_content";
		no_content->func.text_get = NULL;
		no_content->func.content_get = __ge_nocontent_get;
		no_content->func.state_get = NULL;
		no_content->func.del = NULL;
	}

	if (ugd->thumbs_d->medias_cnt > 0) {
		elm_gengrid_item_size_set(gengrid, size, size);
		elm_gengrid_item_append(gengrid,
		                        gic_first,
		                        NULL, __ge_launch_ug_gallery,
		                        ugd);
		/* From zeroth IMAGE item to last one */
		for (i = 0; i < ugd->thumbs_d->medias_cnt; i++) {
			gitem = eina_list_nth(ugd->thumbs_d->medias_elist, i);
			if (gitem == NULL || gitem->item == NULL ||
			        gitem->item->uuid == NULL) {
				ge_dbgE("Invalid gitem!");
				continue;
			}

			if (!gitem->item->file_url) {
				ge_dbgE("file_url is invalid!");
				_ge_data_del_media_by_id(ugd, gitem->item->uuid);
				_ge_data_util_free_item(gitem);
				ugd->thumbs_d->medias_cnt--;
				i--;
				gitem = NULL;
				continue;
			}

			gitem->ugd = ugd;
			gitem->elm_item = elm_gengrid_item_append(gengrid,
			                  ugd->thumbs_d->gic,
			                  gitem, __ge_gengrid_item_sel_cb,
			                  gitem);

			item_cnt++;
			gitem->sequence = item_cnt + 1;
		}
	} else {
		ugd->nocontents = _ge_nocontents_create(ugd);
		elm_gengrid_item_size_set(gengrid, win_w, HALF_MODE_PORTRAIT_HEIGHT);
		elm_gengrid_item_append(gengrid,
		                        no_content,
		                        ugd, NULL,
		                        ugd);
	}
	evas_object_show(gengrid);

	elm_object_part_content_set(ugd->albums_view, "contents", gengrid);

	char *pd_selected = GE_STR_PD_SELECTED;
	ugd->nf_it = elm_naviframe_item_push(ugd->naviframe, g_strdup_printf(pd_selected, ugd->thumbs_d->tot_selected), NULL, NULL, ugd->albums_view, "basic/transparent");

	/* Cancel Button */
	Evas_Object *btn1 = elm_button_add(ugd->naviframe);
	elm_object_style_set(btn1, "naviframe/title_left");
	//elm_object_text_set(btn1, GE_STR_ID_CANCEL_CAP);
	_ge_ui_set_translate_str(btn1, "IDS_TPLATFORM_ACBUTTON_CANCEL_ABB");
	elm_object_item_part_content_set(ugd->nf_it, GE_NAVIFRAME_TITLE_LEFT_BTN, btn1);
	evas_object_show(btn1);
	evas_object_smart_callback_add(btn1, "clicked", __ge_cancel_cb, ugd);

	/* Done Button*/
	Evas_Object *btn2 = elm_button_add(ugd->naviframe);
	elm_object_style_set(btn2, "naviframe/title_right");
	//elm_object_text_set(btn2, GE_STR_ID_DONE_CAP);
	_ge_ui_set_translate_str(btn2, "IDS_TPLATFORM_ACBUTTON_DONE_ABB");
	elm_object_disabled_set(btn2, EINA_TRUE);
	elm_object_item_part_content_set(ugd->nf_it, GE_NAVIFRAME_TITLE_RIGHT_BTN, btn2);
	evas_object_show(btn2);
	evas_object_smart_callback_add(btn2, "clicked", __ge_grid_done_cb, (void *)ugd);

	elm_naviframe_item_title_enabled_set(ugd->nf_it, EINA_FALSE, EINA_FALSE);

	elm_naviframe_item_pop_cb_set(ugd->nf_it, __ge_main_back_cb, ugd);

	return 0;
}

int _ge_albums_update_view(ge_ugdata *ugd)
{
	ge_dbg("");
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->cluster_list, -1);
	int sel_cnt = 0;

	/* Reset view mode */
	_ge_set_view_mode(ugd, GE_VIEW_ALBUMS);

	/* Changed to show no contents if needed */
	if (!ugd->cluster_list->clist ||
	        (eina_list_count(ugd->cluster_list->clist) == 0)) {
		_ge_data_get_clusters(ugd, GE_ALBUM_DATA_NONE);
		ugd->cluster_list->append_cb = __ge_albums_append;
		if (!ugd->cluster_list->clist ||
		        (eina_list_count(ugd->cluster_list->clist) == 0)) {
			ge_dbgW("Clusters list is empty!");
			goto ALBUMS_FAILED;
		}
	}

	if (ugd->nocontents && ugd->nocontents == ugd->albums_view) {
		/* It is nocontents, unset it first then create albums view*/
		evas_object_del(ugd->nocontents);
		ugd->nocontents = NULL;

		ugd->albums_view = __ge_albums_add_view(ugd, ugd->naviframe);
		GE_CHECK_VAL(ugd->albums_view, -1);
		Evas_Object *tmp = NULL;
		tmp = elm_object_part_content_get(ugd->albums_view_ly, "contents");
		if (tmp != NULL) {
			ge_dbgE("tmp was not empty");
			elm_object_part_content_unset(ugd->albums_view_ly, "contents");
			evas_object_hide(tmp);
		}

		elm_object_part_content_set(ugd->albums_view_ly, "contents",
		                            ugd->albums_view);

		ge_dbgE("!!!!! album view !!!!!!!! is pushed in the naviframe instead of content set");
		ugd->nf_it = elm_naviframe_item_push(ugd->naviframe, GE_ALBUM_NAME, NULL, NULL, ugd->albums_view, NULL);

		if (ugd->nf_it != NULL) {
			ge_dbgE("!!!! album view !!!!! is push successfully in the nf");
		} else {
			ge_dbgE("!!!!!! album view !!!!!1111 failed to push in nf ");
		}



		evas_object_show(ugd->albums_view);
	} else {
		if (__ge_albums_append_albums(ugd, ugd->albums_view, true) != 0) {
			goto ALBUMS_FAILED;
		} else {
			elm_naviframe_item_pop(ugd->naviframe);
			ugd->nf_it = elm_naviframe_item_push(ugd->naviframe, GE_ALBUM_NAME, NULL, NULL, ugd->albums_view, NULL);

			if (ugd->nf_it != NULL) {
				ge_dbgE("!!!! album view !!!!! is push successfully in the nf");
			} else {
				ge_dbgE("!!!!!! album view !!!!!1111 failed to push in nf ");
			}
		}
	}

	sel_cnt = _ge_data_get_sel_cnt(ugd);
	if (ugd->done_it != NULL) {
		if (sel_cnt > 0 && (ugd->max_count < 0 || sel_cnt <= ugd->max_count)) {
			_ge_ui_disable_item(ugd->done_it, false);
		} else {
			_ge_ui_disable_item(ugd->done_it, true);
		}
	} else {
		ge_dbgW("done item is NULL");
	}

	Elm_Object_Item *nf_it = elm_naviframe_bottom_item_get(ugd->naviframe);
	_ge_ui_update_label_text(nf_it, sel_cnt, ugd->albums_view_title);
	return 0;

ALBUMS_FAILED:

	sel_cnt = _ge_data_get_sel_cnt(ugd);
	if (ugd->done_it != NULL) {
		if (sel_cnt > 0 && (ugd->max_count < 0 || sel_cnt <= ugd->max_count)) {
			_ge_ui_disable_item(ugd->done_it, false);
		} else {
			_ge_ui_disable_item(ugd->done_it, true);
		}
	} else {
		ge_dbgW("done item is NULL");
	}

	if (ugd->albums_view && ugd->albums_view != ugd->nocontents) {
		__ge_albums_del_cbs(ugd->albums_view);
	}

	evas_object_del(ugd->albums_view);

	ge_dbgW("@@@@@@@  To create nocontents view @@@@@@@@");
	ugd->nocontents = ge_ui_create_nocontents(ugd);
	ugd->albums_view = ugd->nocontents;
	GE_CHECK_VAL(ugd->albums_view, -1);
	evas_object_show(ugd->albums_view);

	elm_object_part_content_set(ugd->albums_view_ly, "contents",
	                            ugd->albums_view);


	ge_dbgE("!!!!! album view !!!!!!!! is pushed in the naviframe instead of content set");
	ugd->nf_it = elm_naviframe_item_push(ugd->naviframe, GE_ALBUM_NAME, NULL, NULL, ugd->albums_view, NULL);

	if (ugd->nf_it != NULL) {
		ge_dbgE("!!!! album view !!!!! is push successfully in the nf");
	} else {
		ge_dbgE("!!!!!! album view !!!!!1111 failed to push in nf ");
	}

	return -1;
}

