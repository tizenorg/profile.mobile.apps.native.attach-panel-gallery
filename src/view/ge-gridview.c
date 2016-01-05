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

#include "ge-gridview.h"
#include "ge-ui-util.h"
#include "ge-util.h"
#include "ge-ext-ug-load.h"
#include "ge-data.h"
#include "ge-exif.h"
#include "ge-thumb.h"
#include "ge-albums.h"
#include "ge-button.h"
#include "ge-main-view.h"
#include "ge-strings.h"
#include "ge-tile.h"
#include <string.h>

int a = 0;
int b = 0;
int A = 0;
/* Icon swallowed to part "elm.swallow.icon" */
#define GE_GRID_ICON_SIZE 156
#define GE_GRID_CHECKBOX "elm.swallow.end"
int back_cnt = 0;

static void __ge_grid_check_changed(void *data, Evas_Object *obj, void *ei);

static void __ge_grid_done_cb(void *data, Evas_Object *obj, void *ei);

static void __ge_grid_thumb_created_cb(media_content_error_e error,
                                       const char *path, void *user_data)
{
	if (GE_FILE_EXISTS(path) && error == MEDIA_CONTENT_ERROR_NONE) {
		GE_CHECK(user_data);
		ge_item *gitem = (ge_item *)user_data;
		GE_CHECK(gitem->item);
		ge_dbg("Update item with new thumb path[%s]", path);
		/* Update thumb path */
		GE_FREEIF(gitem->item->thumb_url);
		gitem->item->thumb_url = strdup(path);
		gitem->item->b_create_thumb = false;
		elm_gengrid_item_update(gitem->elm_item);
	} else {
		ge_dbgE("Invalid thumb path! Error number[%d]", error);
	}
}

/* Append item */
int __ge_grid_append_sel_item(ge_ugdata *ugd, ge_item *gitem)
{
	if (ugd == NULL || ugd->thumbs_d == NULL ||
	        ugd->thumbs_d->album_uuid == NULL || gitem == NULL ||
	        gitem->item == NULL) {
		ge_dbgE("Error input!");
		return -1;
	}

	/* Check album is created or not */
	ge_sel_album_s *salbum = NULL;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(ugd->selected_elist, l, salbum) {
		if (salbum == NULL || salbum->uuid == NULL) {
			continue;
		}

		if (!g_strcmp0(salbum->uuid, ugd->thumbs_d->album_uuid)) {
			ge_dbg("Found[%s]", salbum->uuid);
			break;
		}

		salbum = NULL;
	}

	/* Add a new album */
	if (salbum == NULL) {
		ge_dbg("Add new select album");
		salbum = _ge_data_util_new_sel_album(ugd->thumbs_d->album_uuid);
		ugd->selected_elist = eina_list_append(ugd->selected_elist,
		                                       salbum);
		ge_dbg("Album count: %d", eina_list_count(ugd->selected_elist));
	}
	if (salbum == NULL) {
		ge_dbgE("Invalid select album");
		return -1;
	}

	ge_sel_item_s *sit = NULL;
	sit = _ge_data_util_new_sel_item(gitem);
	/* Append item */
	if (sit == NULL) {
		ge_dbgE("Invalid select item");
	} else {
		ge_sdbg("Path: %s", sit->file_url);
		salbum->elist = eina_list_append(salbum->elist, sit);
	}
	return 0;
}

/* Remove item */
int __ge_grid_remove_sel_item(ge_ugdata *ugd, ge_item *gitem)
{
	if (ugd == NULL || ugd->thumbs_d == NULL ||
	        ugd->thumbs_d->album_uuid == NULL || gitem == NULL ||
	        gitem->item == NULL) {
		ge_dbgE("Error input!");
		return -1;
	}

	/* Check album is appended or not */
	ge_sel_album_s *salbum = NULL;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(ugd->selected_elist, l, salbum) {
		if (salbum == NULL || salbum->uuid == NULL) {
			continue;
		}

		if (!g_strcmp0(salbum->uuid, ugd->thumbs_d->album_uuid)) {
			ge_dbg("Found[%s]", salbum->uuid);
			break;
		}

		salbum = NULL;
	}

	if (salbum == NULL) {
		ge_dbgE("None select album!");
		return -1;
	}

	/* Remove item */
	ge_sel_item_s *sit = NULL;
	EINA_LIST_FOREACH(salbum->elist, l, sit) {
		if (sit == NULL || sit->uuid == NULL) {
			continue;
		}
		if (g_strcmp0(sit->uuid, gitem->item->uuid)) {
			continue;
		}

		salbum->elist = eina_list_remove(salbum->elist, sit);
		_ge_data_util_free_sel_item(sit);
	}
	return 0;
}

/* Check item */
static int __ge_grid_check_sel_items(ge_ugdata *ugd)
{
	if (ugd == NULL || ugd->thumbs_d == NULL ||
	        ugd->thumbs_d->album_uuid == NULL) {
		ge_dbgE("Error input!");
		return -1;
	}

	/* Check album is appended or not */
	ge_sel_album_s *salbum = NULL;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(ugd->selected_elist, l, salbum) {
		if (salbum == NULL || salbum->uuid == NULL) {
			continue;
		}

		if (!g_strcmp0(salbum->uuid, ugd->thumbs_d->album_uuid)) {
			break;
		}

		salbum = NULL;
	}

	if (salbum == NULL) {
		ge_dbgE("None select album!");
		return -1;
	}

	/* Remove item */
	ge_sel_item_s *sit = NULL;
	EINA_LIST_FOREACH(salbum->elist, l, sit) {
		if (sit == NULL || sit->uuid == NULL) {
			continue;
		}
		if (sit->valid == 1) {
			sit->valid = 0;
			continue;
		} else {
			ge_dbgW("Remove invalid item!");
			salbum->elist = eina_list_remove(salbum->elist, sit);
			_ge_data_util_free_sel_item(sit);
		}
	}
	return 0;
}

/* Get count of selected items */
static int __ge_grid_get_sel_cnt(ge_ugdata *ugd)
{
	if (ugd == NULL || ugd->selected_elist == NULL ||
	        ugd->thumbs_d == NULL) {
		return 0;
	}

	if (ugd->thumbs_d->b_mainview) {
		return eina_list_count(ugd->selected_elist);
	}

	if (ugd->thumbs_d->album_uuid == NULL) {
		return 0;
	}
	/* Check album is created or not */
	ge_sel_album_s *salbum = NULL;
	int cnt = 0;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(ugd->selected_elist, l, salbum) {
		if (salbum == NULL || salbum->elist == NULL ||
		        salbum->uuid == NULL ||
		        g_strcmp0(salbum->uuid, ugd->thumbs_d->album_uuid)) {
			continue;
		}
		cnt = eina_list_count(salbum->elist);
		break;
	}
	ge_dbg("Count: %d", cnt);

	int cnt_all = _ge_data_get_sel_cnt(ugd);
	if (cnt_all > 0) {
		ge_dbg("Album count if part ");
		Evas_Object *btn = NULL;
		btn = elm_object_item_part_content_get(ugd->thumbs_d->nf_it, GE_NAVIFRAME_TITLE_RIGHT_BTN);
		if (btn == NULL) {
			ge_dbgE("Failed to get part information");
			return 0;
		}
		elm_object_disabled_set(btn, EINA_FALSE);
		evas_object_smart_callback_add(btn, "clicked", __ge_grid_done_cb, (void *)ugd);
	} else {
		ge_dbg("Album count else part ");
		Evas_Object *btn = NULL;
		btn = elm_object_item_part_content_get(ugd->thumbs_d->nf_it , GE_NAVIFRAME_TITLE_RIGHT_BTN);
		if (btn == NULL) {
			ge_dbgE("Failed to get part information");
			return 0;
		}
		elm_object_disabled_set(btn, EINA_TRUE);
		evas_object_smart_callback_add(btn, "clicked", NULL, (void *)ugd);
	}

	return cnt;
}

static void __ge_grid_realized(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("realized");
	GE_CHECK(ei);
	GE_CHECK(data); /* It's ad */
	Elm_Object_Item *it = (Elm_Object_Item *)ei;
	ge_item *gitem = elm_object_item_data_get(it);
	GE_CHECK(gitem);
	GE_CHECK(gitem->item);
	/* Checking for local files only */
	if (gitem->store_type == GE_MMC || gitem->store_type == GE_ALL ||
	        gitem->store_type == GE_PHONE) {
		/* Use default image */
		if (!GE_FILE_EXISTS(gitem->item->thumb_url) &&
		        GE_FILE_EXISTS(gitem->item->file_url)) {
			ge_sdbg("File[%s]", gitem->item->file_url);
			_ge_data_create_thumb(gitem->item,
			                      __ge_grid_thumb_created_cb,
			                      gitem);
		}
	}
}

static void __ge_grid_unrealized(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("unrealized");
	GE_CHECK(ei);
	GE_CHECK(data); /* It's ad */
	Elm_Object_Item *it = (Elm_Object_Item *)ei;
	ge_item *gitem = elm_object_item_data_get(it);
	GE_CHECK(gitem);
	GE_CHECK(gitem->item);
	/* Checking for local files only */
	if (gitem->store_type == GE_MMC ||
	        gitem->store_type == GE_ALL ||
	        gitem->store_type == GE_PHONE) {
		if (gitem->item->b_create_thumb) {
			_ge_data_cancel_thumb(gitem->item);
		}
	}
}

int _ge_grid_sel_one(ge_ugdata *ugd, char *file_url)
{
	GE_CHECK_VAL(file_url, -1);
	GE_CHECK_VAL(ugd, -1);
	ge_sdbg("Service add:%s", file_url);

	/* 2013.06.20
	  * single pick operation also needs array for return value.
	  * this array only has one element. */
	char **path_array = (char **)calloc(1, sizeof(char *));
	if (!path_array) {
		ge_dbgW("failed to allocate path");
		return 0;
	}
	path_array[0] = strdup(file_url);

	int ret = -1;
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
	return 0;
}

static int __ge_grid_change_check(void *data, Elm_Object_Item *it)
{
	GE_CHECK_VAL(it, -1);
	GE_CHECK_VAL(data, -1);
	Evas_Object *ck = NULL;
	ge_dbg("");

	ck = elm_object_item_part_content_get(it, GE_GRID_CHECKBOX);
	GE_CHECK_VAL(ck, -1);

	elm_check_state_set(ck, !elm_check_state_get(ck));
	__ge_grid_check_changed(data, ck, NULL);
	return 0;
}

static void __ge_grid_sel(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("");
	GE_CHECK(data);
	ge_item *gitem = (ge_item*)data;
	GE_CHECK(gitem->item);
	GE_CHECK(gitem->ugd);
	ge_ugdata *ugd = gitem->ugd;
	char *fileurl = NULL;

	elm_gengrid_item_selected_set(ei, EINA_FALSE);

	if (gitem->item->file_url == NULL || strlen(gitem->item->file_url) <= 0) {
		ge_dbgE("file_url is invalid!");
		return;
	} else {
		ge_sdbg("file_url: %s.", gitem->item->file_url);
	}
	/* Save scroller position before clearing gengrid */
	_ge_ui_save_scroller_pos(obj);

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SETAS) {
		/* imageviewer only supprot image type while setting wallpaper */
		if (gitem->item->type == MEDIA_CONTENT_TYPE_IMAGE) {
			ge_dbg("Loading UG-IMAGE(VIDEO)-VIEWER");
			char album_index_str[12] = {0,};
			eina_convert_itoa(gitem->sequence, album_index_str);
			ugd->file_select_setas_path = gitem->item->file_url;
			fileurl = strdup(GE_ALBUM_ALL_ID);
			_ge_ext_load_iv_ug(ugd, gitem->item->file_url, fileurl, gitem->sequence);
		} else if (gitem->item->type == MEDIA_CONTENT_TYPE_VIDEO &&
		           ugd->popup == NULL) {
			ge_ui_create_popup(ugd, GE_POPUP_ONEBUT,
			                   GE_POPUP_ONLY_SUPPORT_IMAGE);
		}

	}
#ifdef TODO
	else if (gitem->item->type == MEDIA_CONTENT_TYPE_IMAGE ||
	         gitem->item->type == MEDIA_CONTENT_TYPE_VIDEO) {
		ge_dbg("Loading UG-IMAGE(VIDEO)-VIEWER");
		fileurl = strdup(GE_ALBUM_ALL_ID);
		_ge_ext_load_iv_ug(ugd, gitem->item->file_url, fileurl, gitem->sequence);
	}
#endif
	GE_FREEIF(fileurl);

	if (ugd->file_select_mode != GE_FILE_SELECT_T_SETAS) {
		if (_ge_get_view_mode(ugd) != GE_VIEW_THUMBS) {
			ge_dbgW("Edit mode!");
			if (ugd->thumbs_d->b_multifile) {
				__ge_grid_change_check(data, (Elm_Object_Item *)ei);
			} else {
				_ge_grid_sel_one(ugd, gitem->item->file_url);
			}
			return;
		} else {
			ge_dbg("ViewMode");
			_ge_grid_sel_one(ugd, gitem->item->file_url);
			return;
		}
	}

}

static void __ge_grid_check_changed(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("");
	GE_CHECK(obj);
	GE_CHECK(data);
	ge_item *gitem = (ge_item *)data;
	GE_CHECK(gitem->item);
	GE_CHECK(gitem->item->file_url);
	GE_CHECK(gitem->ugd);
	ge_ugdata *ugd = gitem->ugd;
	GE_CHECK(ugd->service);
	GE_CHECK(ugd->thumbs_d);

	if (_ge_get_view_mode(ugd) != GE_VIEW_THUMBS_EDIT) {
		ge_dbgE("Wrong view mode!");
		return;
	}

	Eina_Bool checked = elm_check_state_get(obj);
	if (checked) {
		ge_sdbg("Append:%s", gitem->item->file_url);
		if (!ugd->thumbs_d->b_mainview) {
			__ge_grid_append_sel_item(ugd, gitem);
		} else
			ugd->selected_elist = eina_list_append(ugd->selected_elist,
			                                       gitem);
	} else {
		ge_sdbg("Remove:%s", gitem->item->file_url);
		if (!ugd->thumbs_d->b_mainview) {
			__ge_grid_remove_sel_item(ugd, gitem);
		} else
			ugd->selected_elist = eina_list_remove(ugd->selected_elist,
			                                       gitem);
	}
	ugd->thumbs_d->album->select = true;
	elm_gengrid_item_update(ugd->thumbs_d->album->griditem);

	gitem->checked = checked;
	int local_sel_cnt = __ge_grid_get_sel_cnt(ugd);
	int sel_cnt = _ge_data_get_sel_cnt(ugd);
	_ge_ui_update_label_text(ugd->thumbs_d->nf_it, sel_cnt,
	                         ugd->albums_view_title);
	if (local_sel_cnt == ugd->thumbs_d->medias_cnt) {
		ugd->ck_state = EINA_TRUE;
	} else {
		ugd->ck_state = EINA_FALSE;
	}
	Evas_Object *ck = elm_object_part_content_get(ugd->thumbs_d->layout, "select.all.area.check");
	elm_check_state_set(ck, ugd->ck_state);
	bool b_disabled = false;
	if (!ugd->thumbs_d->b_mainview) {
		sel_cnt = _ge_data_get_sel_cnt(ugd);
		if (ugd->max_count > 0 && sel_cnt > ugd->max_count) {
			char *noti = NULL;
			noti = g_strdup_printf(GE_STR_MAXIMUM_COUNT);
			_ge_ui_create_notiinfo(noti);
			GE_GFREEIF(noti);
			b_disabled = true;
		}
	}
	if (ugd->thumbs_d->done_it != NULL) {
		if (sel_cnt > 0 && !b_disabled) {
			_ge_ui_disable_item(ugd->thumbs_d->done_it, false);
		} else {
			_ge_ui_disable_item(ugd->thumbs_d->done_it, true);
		}
	} else {
		ge_dbgW("done item is NULL");
	}
}

static Evas_Object *__ge_grid_get_content(void *data, Evas_Object *obj, const char *part)
{
	GE_CHECK_NULL(data);
	GE_CHECK_NULL(part);
	GE_CHECK_NULL(obj);
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

	if (!g_strcmp0(part, "elm.swallow.icon")) {
		Evas_Object *layout = NULL;
		char *path = NULL;
		unsigned int orient = 0;

		/* Use default image */
		if (GE_FILE_EXISTS(gitem->item->thumb_url)) {
			path = gitem->item->thumb_url;
		} else {
			path = GE_ICON_NO_THUMBNAIL;
		}

		bool b_favor = false;
		int ret = -1;
		ret = media_info_get_favorite(gitem->item->media_h, &b_favor);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			ge_dbg("media_info_get_favorite error! ERROR[%d]", ret);
		}

		if (gitem->item->type == MEDIA_CONTENT_TYPE_VIDEO) {
			int v_dur = 0;
			_ge_local_data_get_duration(gitem->item->media_h, &v_dur);
			layout = _ge_thumb_show_part_icon_video(obj, path,
			                                        v_dur, 0, b_favor,
			                                        ugd->thumbs_d->icon_w,
			                                        ugd->thumbs_d->icon_h);

		} else {
			layout = _ge_thumb_show_part_icon_image(obj, path,
			                                        orient, b_favor,
			                                        ugd->thumbs_d->icon_w,
			                                        ugd->thumbs_d->icon_h);
		}
		return layout;
	} else if (!g_strcmp0(part, GE_GRID_CHECKBOX) &&
	           (_ge_get_view_mode(ugd) == GE_VIEW_THUMBS_EDIT) &&
	           ugd->thumbs_d->b_multifile) { /* Show checkbox for multiple files selection */
		Evas_Object* ck = NULL;
		ck = elm_check_add(obj);
		GE_CHECK_NULL(ck);

		evas_object_propagate_events_set(ck, EINA_FALSE);
		elm_check_state_set(ck, gitem->checked);
		evas_object_smart_callback_add(ck, "changed",
		                               __ge_grid_check_changed, data);
		evas_object_show(ck);

		return ck;
	} else if (!g_strcmp0(part, "elm.swallow.mode")) {
		return _ge_thumb_show_mode(obj, gitem->item->mode);
	}
	return NULL;
}

static bool __ge_grid_append_thumb(ge_ugdata *ugd, Evas_Object *parent, bool is_update)
{
	ge_dbg("");
	GE_CHECK_FALSE(parent);
	GE_CHECK_FALSE(ugd);
	GE_CHECK_FALSE(ugd->thumbs_d);
	int i = 0;
	ge_item* gitem = NULL;
	int item_cnt = 0;
	ge_dbg("List count : %d", ugd->thumbs_d->medias_cnt);

	if (elm_gengrid_items_count(parent) > 0) {
		/* Save scroller position before clearing gengrid */
		_ge_ui_save_scroller_pos(parent);
		elm_gengrid_clear(parent);
	}
	if (is_update) {
		_ge_grid_update_data(ugd);
	}
	if (ugd->th) {
		elm_object_theme_set(parent, ugd->th);
	}

	for (i = 0; i < ugd->thumbs_d->medias_cnt; i++) {
		gitem = eina_list_nth(ugd->thumbs_d->medias_elist, i);
		if (gitem == NULL || gitem->item == NULL ||
		        gitem->item->uuid == NULL) {
			ge_dbgE("Invalid gitem!");
			continue;
		}

		if (!gitem->item->file_url) {
			_ge_data_del_media_by_id(ugd, gitem->item->uuid);
			_ge_data_util_free_item(gitem);
			ugd->thumbs_d->medias_cnt--;
			--i;
			continue;
		}

		gitem->ugd = ugd;
		gitem->elm_item = elm_gengrid_item_append(parent,
		                  ugd->thumbs_d->gic,
		                  gitem, __ge_grid_sel,
		                  gitem);
		item_cnt++;
		gitem->sequence = item_cnt;
	}
	/* Restore previous position of scroller */
	_ge_ui_restore_scroller_pos(parent);

	if (item_cnt == 0) {
		return EINA_FALSE;
	} else {
		return EINA_TRUE;
	}
}

static Evas_Object* __ge_grid_create_ly(Evas_Object *parent)
{
	ge_dbg("");
	GE_CHECK_NULL(parent);
	Evas_Object* layout = NULL;

	layout = ge_ui_load_edj(parent, GE_EDJ_FILE, GE_GRP_GRIDVIEW);
	GE_CHECK_NULL(layout);
	evas_object_show(layout);
	return layout;
}

static int __ge_grid_del_cbs(Evas_Object *view)
{
	if (view) {
		ge_dbg("Delete callbacks");
		evas_object_smart_callback_del(view, "realized",
		                               __ge_grid_realized);
		evas_object_smart_callback_del(view, "unrealized",
		                               __ge_grid_unrealized);
	}
	return 0;
}

static int __ge_grid_rotate_view(ge_ugdata *ugd)
{
	if (ugd && ugd->thumbs_d && ugd->thumbs_d->view &&
	        ugd->thumbs_d->nocontents != ugd->thumbs_d->view) {
		_ge_thumb_set_size(ugd, ugd->thumbs_d->view, ugd->rotate_mode,
		                   &(ugd->thumbs_d->icon_w),
		                   &(ugd->thumbs_d->icon_h));
		return 0;
	} else {
		return -1;
	}
}

/* Free data after layout deleted */
static void __ge_grid_delete_layout_cb(void *data, Evas *e, Evas_Object *obj,
                                       void *ei)
{
	ge_dbg("Delete layout ---");
	/*	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
					       __ge_grid_delete_layout_cb);*/
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK(ugd->thumbs_d);

	if (ugd->key_down_handler) {
		ecore_event_handler_del(ugd->key_down_handler);
		ugd->key_down_handler = NULL;
	}

	GE_IF_DEL_IDLER(ugd->thumbs_d->append_idler)
	if (ugd->file_select_mode == GE_FILE_SELECT_T_ONE) {
		app_control_remove_extra_data(ugd->service,
		                              GE_FILE_SELECT_RETURN_PATH);
	}
	ugd->rotate_cbs = eina_list_remove(ugd->rotate_cbs,
	                                   __ge_grid_rotate_view);

	if (ugd->thumbs_d->nocontents != ugd->thumbs_d->view) {
		elm_gengrid_clear(ugd->thumbs_d->view);
		__ge_grid_del_cbs(ugd->thumbs_d->view);
		_ge_ui_del_scroller_pos(ugd->thumbs_d->view);
	}
	if (ugd->thumbs_d->gic) {
		elm_gengrid_item_class_free(ugd->thumbs_d->gic);
	}
	/* Clear view data */
	_ge_data_free_medias(&(ugd->thumbs_d->medias_elist));
	ugd->thumbs_d->medias_cnt = 0;
	GE_FREEIF(ugd->thumbs_d->album_uuid);
	/* Free thumbs data */
	GE_FREEIF(ugd->thumbs_d);
	ugd->thumbs_d = NULL;

	ge_dbg("Delete layout +++");
}

/*
* When select album, show first (GE_FIRST_VIEW_END_POS+1) medias.
* Then use idler to get other medias from DB and appened
* them to gridview.
* In order to reduce transit time of first show of thumbnails view.
* Cause most of time is spent for getting records from DB,
* so we get first (GE_FIRST_VIEW_END_POS+1) items and show thumbnails view,
* it will accelerate view show.
*/
static int __ge_grid_idler_append_thumbs(void *data)
{
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK_VAL(ugd->thumbs_d, -1);
	int i = 0;
	int item_cnt = 0;
	ge_item* gitem = NULL;
	int ret = -1;
	ge_dbg("First view medias count: %d;", ugd->thumbs_d->medias_cnt);

	if (ugd->thumbs_d->medias_cnt != (GE_FIRST_VIEW_END_POS + 1)) {
		ge_dbg("No any more items!");
		if (ugd->thumbs_d->b_multifile) {
			__ge_grid_check_sel_items(ugd);
		}
		return -1;
	}

	Eina_List *sel_ids = NULL;
	if (ugd->thumbs_d->b_multifile) {
		ge_dbg("Edit view for multiple selection");
		/* Get ID list of selected items */
		_ge_grid_get_selected_ids(ugd, &sel_ids);
	}
	/* Get other items from DB */
	ret = _ge_data_get_medias(ugd, ugd->thumbs_d->album_uuid,
	                          ugd->thumbs_d->store_type,
	                          (GE_FIRST_VIEW_END_POS + 1),
	                          GE_GET_UNTIL_LAST_RECORD,
	                          &(ugd->thumbs_d->medias_elist),
	                          _ge_data_restore_selected, sel_ids);
	if (ugd->thumbs_d->b_multifile) {
		__ge_grid_check_sel_items(ugd);
	}

	ge_dbgW("###Get items list over[%d]###", ret);
	if (ret < 0) {
		ge_dbg("Get items list failed!");
		return ret;
	}
	ugd->thumbs_d->medias_cnt = eina_list_count(ugd->thumbs_d->medias_elist);
	ge_dbg("Grid view all medias count: %d", ugd->thumbs_d->medias_cnt);


	/* From (GE_FIRST_VIEW_END_POS + 1)th item to last one */
	for (i = GE_FIRST_VIEW_END_POS + 1; i < ugd->thumbs_d->medias_cnt; i++) {
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
		gitem->elm_item = elm_gengrid_item_append(ugd->thumbs_d->view,
		                  ugd->thumbs_d->gic,
		                  gitem, __ge_grid_sel,
		                  gitem);
		item_cnt++;
		gitem->sequence = item_cnt + GE_FIRST_VIEW_END_POS + 1;
	}

	ge_dbgW("Use idler to append other medias--Done");
	if (item_cnt == 0) {
		return -1;
	} else {
		return 0;
	}
}

static Eina_Bool __ge_grid_append_thumb_idler_cb(void *data)
{
	GE_CHECK_CANCEL(data);
	ge_ugdata *ugd = (ge_ugdata*)data;
	GE_CHECK_CANCEL(ugd->thumbs_d);

	/* Try to get other medias from DB and append them to gridview*/
	int ret = __ge_grid_idler_append_thumbs(data);
	if (ret != 0) {
		ge_dbg("Failed to append grid items!");
	} else {
		ge_dbg("Successful to append grid items!");
	}

	/* Remove idler after all items appended */
	GE_IF_DEL_IDLER(ugd->thumbs_d->append_idler);
	return ECORE_CALLBACK_CANCEL;
}

#if 0
static Eina_Bool __ge_grid_back_cb(void *data, Elm_Object_Item *it)
{
	ge_dbgW("return key call back");
	GE_CHECK_FALSE(data);
	ge_ugdata *ugd = (ge_ugdata *)data;

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SLIDESHOW) {
		ug_send_result_full(ugd->ug, ugd->service,
		                    APP_CONTROL_RESULT_FAILED);
		//ug_destroy_me(ugd->ug);
		ge_dbg("ug_destroy_me");
		/*If return ture, ug will pop naviframe first.*/
		return EINA_FALSE;
	}

	else {
		//ug_destroy_me(ugd->ug);
		ge_dbgW("ug_destroy_me ug destroyed in the else part");
	}
	/* Reset view mode */
	ge_dbgW("To update albums view!");
	_ge_albums_update_view(data);
	//ug_destroy_me(ugd->ug);

	return EINA_TRUE;
}

static void __ge_grid_done_cb(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("");
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK(ugd->thumbs_d);

	elm_toolbar_item_selected_set((Elm_Object_Item *)ei, EINA_FALSE);
	elm_object_disabled_set(obj, EINA_TRUE);

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SLIDESHOW) {
		/*ge_sel_album_s *salbum = NULL;
		ge_sel_item_s *sit = NULL;
		salbum = (ge_sel_album_s *)eina_list_nth(ugd->selected_elist, 0);
		GE_CHECK(salbum);
		sit = (ge_sel_item_s *)eina_list_nth(salbum->elist, 0);*/
		ge_item *git = (ge_item *)eina_list_nth(ugd->selected_elist, 0);
		_ge_ext_load_iv_ug(ugd, git->item->file_url);
		return;
	}

	_ge_send_result(ugd);
	ug_destroy_me(ugd->ug);
}

static void __ge_grid_cancel_cb(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("");
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	elm_naviframe_item_pop(ugd->naviframe);
}
#endif

/* Select-all checkbox selected/deselected */
static void __ge_grid_selall_cb(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(data);
	ge_dbg("");
	int i = 0;
	ge_ugdata *ugd = (ge_ugdata *)data;
	ge_item *gitem = NULL;
	ugd->ck_state = !ugd->ck_state;
	ge_dbg("");
	Evas_Object *ck = elm_object_part_content_get(ugd->thumbs_d->layout, "select.all.area.check");
	ge_dbg("changing strate");
	elm_check_state_set(ck, ugd->ck_state);
	ge_dbg("state changed");
	for (; i < ugd->thumbs_d->medias_cnt; ++i) {
		gitem = eina_list_nth(ugd->thumbs_d->medias_elist, i);
		if (NULL == gitem || NULL == gitem->item ||
		        NULL == gitem->item->file_url || NULL == gitem->elm_item) {
			continue;
		}
		if (gitem->checked == ugd->ck_state) {
			continue;
		}
		gitem->checked = ugd->ck_state;
		if (ugd->ck_state == EINA_TRUE) {
			__ge_grid_append_sel_item(ugd, gitem);
		} else {
			__ge_grid_remove_sel_item(ugd, gitem);
		}
		ck = elm_object_item_part_content_get(gitem->elm_item, GE_GRID_CHECKBOX);
		if (ck == NULL) {
			ge_dbgE("Invalid checkbox!");
			continue;
		}
		if (elm_check_state_get(ck) != ugd->ck_state) {
			elm_check_state_set(ck, ugd->ck_state);
		}
	}
	int sel_cnt = __ge_grid_get_sel_cnt(ugd);
	sel_cnt = _ge_data_get_sel_cnt(ugd);
	_ge_ui_update_label_text(ugd->thumbs_d->nf_it, sel_cnt,
	                         ugd->albums_view_title);
	elm_gengrid_item_update(ugd->thumbs_d->album->griditem);
}
static void __ge_grid_done_cb(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	ugd->ck_state = !ugd->ck_state;
	int i = 0;
	ge_item *gitem = NULL;
	Evas_Object *ck = NULL;

	/* Check album is created or not */
	ge_sel_album_s *salbum = NULL;
	int cnt = 0;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(ugd->selected_elist, l, salbum) {
		if (salbum == NULL || salbum->elist == NULL ||
		        salbum->uuid == NULL ||
		        g_strcmp0(salbum->uuid, ugd->thumbs_d->album_uuid)) {
			continue;
		}
		cnt = eina_list_count(salbum->elist);
		break;
	}
	ge_dbg("Count: %d", cnt);

	if (cnt == 0) {
		return;
	}

	for (; i < ugd->thumbs_d->medias_cnt; ++i) {
		gitem = eina_list_nth(ugd->thumbs_d->medias_elist, i);
		if (NULL == gitem || NULL == gitem->item ||
		        NULL == gitem->item->file_url || NULL == gitem->elm_item) {
			continue;
		}

		if (gitem->checked == ugd->ck_state) {
			continue;
		}

		gitem->checked = ugd->ck_state;
		if (!ugd->thumbs_d->b_mainview) {
			if (ugd->ck_state == EINA_TRUE) {
				ge_dbgE("if part fun will be called ");
//				__ge_main_done_cb(ugd, NULL, NULL);
				_ge_send_result(ugd);
				if (!ugd->is_attach_panel) {
					ug_destroy_me(ugd->ug);
				}
				//__ge_grid_append_sel_item(ugd, gitem);
			} else {
				ge_dbgE("else part fun will be called ");
//				__ge_main_done_cb(ugd, NULL, NULL);
				_ge_send_result(ugd);
				if (!ugd->is_attach_panel) {
					ug_destroy_me(ugd->ug);
				}
				//__ge_grid_remove_sel_item(ugd, gitem);
			}
		} else {
			if (ugd->ck_state == EINA_TRUE)
				ugd->selected_elist = eina_list_append(ugd->selected_elist,
				                                       gitem);
			else
				ugd->selected_elist = eina_list_remove(ugd->selected_elist,
				                                       gitem);
		}
		ck = elm_object_item_part_content_get(gitem->elm_item, GE_GRID_CHECKBOX);
		if (ck == NULL) {
			ge_dbgE("Invalid checkbox!");
			continue;
		}
		if (elm_check_state_get(ck) != ugd->ck_state) {
			elm_check_state_set(ck, ugd->ck_state);
		}
	}

	int sel_cnt = __ge_grid_get_sel_cnt(ugd);
	_ge_ui_update_label_text(ugd->thumbs_d->nf_it, sel_cnt,
	                         ugd->albums_view_title);

	bool b_disabled = false;
	if (!ugd->thumbs_d->b_mainview) {
		sel_cnt = _ge_data_get_sel_cnt(ugd);
		if (ugd->max_count > 0 && sel_cnt > ugd->max_count) {
			char *noti = NULL;
			noti = g_strdup_printf(GE_STR_MAXIMUM_COUNT, ugd->max_count);
			_ge_ui_create_notiinfo(noti);
			GE_GFREEIF(noti);
			b_disabled = true;
		}
	}
	if (ugd->thumbs_d->done_it != NULL) {
		if (sel_cnt > 0 && !b_disabled) {
			_ge_ui_disable_item(ugd->thumbs_d->done_it, false);
		} else {
			_ge_ui_disable_item(ugd->thumbs_d->done_it, true);
		}
	} else {
		ge_dbgW("done item is NULL");
	}
}

static int __ge_grid_add_btns(ge_ugdata *ugd, Evas_Object *parent,
                              Elm_Object_Item *nf_it)
{
	ge_dbg("Multiple selection, show Done");
	/* Done */
	//Elm_Object_Item *tb_it = NULL;
	//Evas_Object *toolbar = _ge_ui_add_toolbar(parent);

	/*if (ugd->file_select_mode == GE_FILE_SELECT_T_SLIDESHOW) {
		_ge_ui_append_item(toolbar, NULL, GE_STR_ID_CANCEL,
				   __ge_grid_cancel_cb, ugd);
		tb_it = _ge_ui_append_item(toolbar, NULL, GE_STR_ID_START,
					   __ge_grid_done_cb, ugd);
		if (tb_it != NULL) {
			_ge_ui_disable_item(tb_it, true);
		}
	} else {
		_ge_ui_append_item(toolbar, NULL, GE_STR_ID_BACK,
				   __ge_grid_cancel_cb, ugd);
		tb_it = _ge_ui_append_item(toolbar, NULL, GE_STR_ID_DONE,
					   __ge_grid_done_cb, ugd);
		if (tb_it != NULL) {
			_ge_ui_disable_item(tb_it, true);
		}
	}

	ugd->thumbs_d->done_it = tb_it;*/
	//elm_object_item_part_content_set(nf_it, "toolbar", toolbar);
	return 0;
}

static Eina_Bool __ge_main_back_cb(void *data, Elm_Object_Item *it)
{
	ge_dbgE("all ug will be destroyed now");
	GE_CHECK_FALSE(data);
	ge_ugdata *ugd = (ge_ugdata *)data;

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

	app_control_add_extra_data(ugd->service, GE_FILE_SELECT_RETURN_COUNT, "0");
	app_control_add_extra_data(ugd->service, GE_FILE_SELECT_RETURN_PATH, NULL);
	app_control_add_extra_data(ugd->service, APP_CONTROL_DATA_SELECTED, NULL);
	app_control_add_extra_data_array(ugd->service, APP_CONTROL_DATA_PATH, NULL, 0);
	ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_FAILED);
	//elm_naviframe_item_pop(it);
	if (!ugd->is_attach_panel) {
		ug_destroy_me(ugd->ug);
		ge_dbg("ug_destroy_me");
	}
	/*If return ture, ug will pop naviframe first.*/
	return EINA_FALSE;
}

void _my_custom_keydown_cb(void* data, int type, void* event)
{
	Ecore_Event_Key* ev = (Ecore_Event_Key*)event;
	ge_ugdata *app_data = (ge_ugdata *)data;
	ge_dbgE("key down call back ");
	if (!strcmp(ev->keyname, "XF86Back")) {
		ge_dbg("back key event");
		if (app_data->is_attach_panel && (app_data->attach_panel_display_mode == ATTACH_PANEL_FULL_MODE)) {
			if (_ge_get_view_mode(app_data) != GE_VIEW_ALBUMS) {
				elm_scroller_movement_block_set(app_data->albums_view, ELM_SCROLLER_MOVEMENT_NO_BLOCK);
				int ret;
				app_control_h app_control = NULL;
				ret = app_control_create(&app_control);
				if (ret == APP_CONTROL_ERROR_NONE) {
					app_control_add_extra_data(app_control, ATTACH_PANEL_FLICK_MODE_KEY, ATTACH_PANEL_FLICK_MODE_ENABLE);
					ret = ug_send_result_full(app_data->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
				}
				app_control_destroy(app_control);
			}
		}
		_ge_tile_update_item_size(app_data, app_data->albums_view, app_data->rotate_mode,
		                          false);
	} else if (!strcmp(ev->keyname, "XF86Stop")) {
		char label[100];
		Elm_Object_Item *top = elm_naviframe_top_item_get(app_data->naviframe);

		snprintf(label, sizeof(label), "______________ the label is %s",  elm_object_item_part_text_get(top , NULL));
		ge_dbg("%s", label);

		back_cnt++;

		if (((a - b) == 1 || back_cnt <= 2)) {
			if (back_cnt == 1) {
				elm_naviframe_item_pop(app_data->naviframe);
				Eina_List *list = elm_naviframe_items_get(app_data->naviframe);
				A = eina_list_count(list);
				ge_dbg("middle   the value of A is %d", A);
			} else if (A != 3) {
				A = 0;
				ge_dbgE("back key is pressed so item will be poped from naviframe here");
				ge_dbgE("Size of the list before pop");
				Eina_List *list = elm_naviframe_items_get(app_data->naviframe);
				char str[100];
				snprintf(str, sizeof(str), "  --------> %d", eina_list_count(list));
				ge_dbg("%s", str);
				a = eina_list_count(list);
				ge_dbg(" value of a is %d", a);
				Elm_Object_Item *top = elm_naviframe_top_item_get(app_data->naviframe);

				elm_object_item_del(top);

				ge_dbgE("Size of the list after pop");
				list = elm_naviframe_items_get(app_data->naviframe);
				snprintf(str, sizeof(str), "  --------> %d", eina_list_count(list));
				ge_dbg("%s", str);
				b = eina_list_count(list);
				ge_dbg("value of b is %d", b);
				elm_naviframe_item_push(app_data->naviframe, GE_ALBUM_NAME, NULL, NULL, app_data->albums_view, NULL);

				list = elm_naviframe_items_get(app_data->naviframe);
				b = eina_list_count(list);
				ge_dbg("after value of b is %d", b);
			}
		} else {
			ge_dbgE("first finally calling the ug destroy function");
			__ge_main_back_cb(app_data, NULL);
		}

		if ((back_cnt == 2) & (A == 3)) {
			ge_dbgE("second finally calling the ug destroy function");
			__ge_main_back_cb(app_data, NULL);
		}
	}
}

static void __ge_albums_detail_cancel_cb(void *data, Evas_Object *obj, void *ei)
{
	ge_ugdata *app_data = (ge_ugdata *)data;
	ge_dbg("back key event");
	if (app_data->is_attach_panel && (app_data->attach_panel_display_mode == ATTACH_PANEL_FULL_MODE)) {
		if (_ge_get_view_mode(app_data) != GE_VIEW_ALBUMS) {
			elm_scroller_movement_block_set(app_data->albums_view, ELM_SCROLLER_MOVEMENT_NO_BLOCK);
			int ret;
			app_control_h app_control = NULL;
			ret = app_control_create(&app_control);
			if (ret == APP_CONTROL_ERROR_NONE) {
				app_control_add_extra_data(app_control, ATTACH_PANEL_FLICK_MODE_KEY, ATTACH_PANEL_FLICK_MODE_ENABLE);
				ret = ug_send_result_full(app_data->ug, app_control, APP_CONTROL_RESULT_SUCCEEDED);
			}
			app_control_destroy(app_control);
		}
	}
	_ge_tile_update_item_size(app_data, app_data->albums_view, app_data->rotate_mode,
	                          false);
	elm_naviframe_item_pop(app_data->naviframe);

}

static int __ge_grid_push_view(ge_ugdata *ugd, Evas_Object *parent,
                               Evas_Object *obj, char *title)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(parent, -1);
	GE_CHECK_VAL(obj, -1);
	GE_CHECK_VAL(title, -1);
	GE_CHECK_VAL(ugd->thumbs_d, -1);
	Elm_Object_Item *nf_it = NULL;

	if (ugd->file_select_mode == GE_FILE_SELECT_T_NONE) {
		ge_dbgE("GE_FILE_SELECT_T_NONE!");
		return -1;
	}
	ge_dbg("title: %s", title);
	/* Add prev button */
	/*	Evas_Object *prev_btn = NULL;
		Evas_Object *prev_item = NULL;
		prev_btn = _ge_but_create_but(obj, ugd->th, NULL, NULL,
					      GE_BTN_NAVI_PRE, NULL, NULL);
		GE_CHECK_VAL(prev_btn, -1);
	*/
	elm_naviframe_prev_btn_auto_pushed_set(parent, EINA_FALSE);
	nf_it = elm_naviframe_item_push(parent, title, NULL, NULL, obj,
	                                NULL);
	if (nf_it == NULL) {
		ge_dbgW("nf_it is NULL");
	} else {
		ge_dbgW("nf_it is NOT NULL");
	}


	Eina_List *list = elm_naviframe_items_get(ugd->naviframe);
	a = eina_list_count(list);

	ugd->thumbs_d->nf_it = nf_it;
	ugd->key_down_handler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
	                        (Ecore_Event_Handler_Cb)_my_custom_keydown_cb,
	                        ugd);

	Evas_Object *btn1 = elm_button_add(parent);

	/**
	 * Add  'Done' item.
	*/
	if (ugd->thumbs_d->b_multifile) {
		elm_object_style_set(btn1, "naviframe/title_left");
		elm_object_text_set(btn1, GE_STR_ID_CANCEL_CAP);
		elm_object_item_part_content_set(ugd->thumbs_d->nf_it, GE_NAVIFRAME_TITLE_LEFT_BTN, btn1);
		ge_dbg("Multiple selection, show Done");
		/* Done */
		ge_dbgE("all the buttons code removed from here");
		__ge_grid_add_btns(ugd, ugd->naviframe, nf_it);
		Evas_Object *btn2 = elm_button_add(parent);
		elm_object_style_set(btn2, "naviframe/title_right");
		elm_object_text_set(btn2, GE_STR_ID_DONE_CAP);
		elm_object_disabled_set(btn2, EINA_TRUE);
		elm_object_item_part_content_set(nf_it,
		                                 GE_NAVIFRAME_TITLE_RIGHT_BTN,
		                                 btn2);
		ugd->ck_state = EINA_FALSE;
	} else {
		elm_object_style_set(btn1, "naviframe/end_btn/default");
		elm_object_item_part_content_set(ugd->thumbs_d->nf_it, "prev_btn", btn1);
	}
	evas_object_smart_callback_add(btn1, "clicked", __ge_albums_detail_cancel_cb, ugd);
	return 0;
}

static void _ge_detail_view_grid_move_stop_cb(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("");
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	int x, y, w, h;
	int ret;
	elm_scroller_region_get(obj, &x, &y, &w, &h);

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

static Evas_Object *__ge_grid_add_view(ge_ugdata *ugd, Evas_Object *parent)
{
	GE_CHECK_NULL(parent);
	GE_CHECK_NULL(ugd);
	GE_CHECK_NULL(ugd->thumbs_d);
	Evas_Object *grid = elm_gengrid_add(parent);
	GE_CHECK_NULL(grid);

#ifdef _USE_CUSTOMIZED_GENGRID_STYLE
	elm_object_style_set(grid, GE_GENGRID_STYLE_GALLERY);
#endif
	ge_dbg("");

	/* Register delete callback */
	evas_object_event_callback_add(parent, EVAS_CALLBACK_DEL,
	                               __ge_grid_delete_layout_cb, ugd);
	/*	evas_object_smart_callback_add(grid, "unrealized",
					       __ge_grid_unrealized, ugd);
		evas_object_smart_callback_add(grid, "realized",
				__ge_grid_realized, ugd);
	*/

	if (ugd->is_attach_panel && (ugd->attach_panel_display_mode != ATTACH_PANEL_FULL_MODE)) {
		elm_scroller_movement_block_set(grid, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
	}
	if (ugd->is_attach_panel) {
		evas_object_smart_callback_add(grid, "scroll,anim,stop",
		                               _ge_detail_view_grid_move_stop_cb, ugd);
		evas_object_smart_callback_add(grid, "scroll,drag,stop",
		                               _ge_detail_view_grid_move_stop_cb, ugd);
	}

	_ge_ui_reset_scroller_pos(parent);

	if (!__ge_grid_append_thumb(ugd, grid, false)) {
		__ge_grid_del_cbs(grid);
		evas_object_del(grid);
		grid = NULL;

		ugd->thumbs_d->nocontents = ge_ui_create_nocontents(ugd);
		evas_object_show(ugd->thumbs_d->nocontents);
		return ugd->thumbs_d->nocontents;
	} else {
		evas_object_show(grid);
		ugd->thumbs_d->nocontents = NULL;
	}

#ifdef _USE_SCROL_HORIZONRAL
	elm_gengrid_align_set(grid, 0.5, 0.5);
	elm_gengrid_horizontal_set(grid, EINA_TRUE);
	elm_scroller_bounce_set(grid, EINA_TRUE, EINA_FALSE);
#else
	elm_gengrid_align_set(grid, 0.5, 0.0);
	elm_gengrid_horizontal_set(grid, EINA_FALSE);
	elm_scroller_bounce_set(grid, EINA_FALSE, EINA_TRUE);
#endif
	elm_scroller_policy_set(grid, ELM_SCROLLER_POLICY_OFF,
	                        ELM_SCROLLER_POLICY_AUTO);
	elm_gengrid_multi_select_set(grid, EINA_TRUE);
	evas_object_size_hint_weight_set(grid, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	_ge_thumb_set_size(ugd, grid, ugd->rotate_mode,
	                   &(ugd->thumbs_d->icon_w), &(ugd->thumbs_d->icon_h));
	return grid;
}

int _ge_grid_get_selected_ids(ge_ugdata *ugd, Eina_List **ids_list)
{
	GE_CHECK_VAL(ids_list, -1);
	if (ugd == NULL || ugd->selected_elist == NULL ||
	        ugd->thumbs_d == NULL || ugd->thumbs_d->album_uuid == NULL) {
		return -1;
	}

	/* Check album is created or not */
	ge_sel_album_s *salbum = NULL;
	Eina_List *l = NULL;
	EINA_LIST_FOREACH(ugd->selected_elist, l, salbum) {
		if (salbum == NULL || salbum->elist == NULL ||
		        salbum->uuid == NULL) {
			continue;
		}
		if (g_strcmp0(salbum->uuid, ugd->thumbs_d->album_uuid)) {
			continue;
		}

		*ids_list = salbum->elist;
		return 0;
	}
	return -1;
}

int _ge_grid_create_view(ge_ugdata *ugd, ge_thumbs_s *thumbs_d)
{
	GE_CHECK_VAL(thumbs_d, -1);

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SETAS) {
		ge_dbg("setas mode->thumbnail is view mode");
		_ge_set_view_mode(ugd, GE_VIEW_THUMBS);
	} else {
		ge_dbg("select mode->thumbnail is edit mode");
		_ge_set_view_mode(ugd, GE_VIEW_THUMBS_EDIT);
	}
	thumbs_d->split_view = __ge_add_albums_split_view(ugd, ugd->naviframe);
	if (!thumbs_d->split_view) {
		ge_dbg("creating split view failed");
		goto GE_GRID_FAILED;
	}

	thumbs_d->layout = __ge_grid_create_ly(ugd->naviframe);
	if (thumbs_d->layout == NULL) {
		goto GE_GRID_FAILED;
	}

	thumbs_d->gic = elm_gengrid_item_class_new();
	if (thumbs_d->gic == NULL) {
		goto GE_GRID_FAILED;
	}
	ugd->thumbs_d->gic->item_style = "gallery_efl/thumbnail_view";
	ugd->thumbs_d->gic->func.text_get = NULL;
	ugd->thumbs_d->gic->func.content_get = __ge_grid_get_content;

	thumbs_d->view = __ge_grid_add_view(ugd, thumbs_d->layout);
	if (thumbs_d->view == NULL) {
		goto GE_GRID_FAILED;
	}

	int w = -1, h = -1;
	evas_object_geometry_get(ugd->win, NULL, NULL, &w, &h);
	Edje_Message_Int *msg = (Edje_Message_Int *)malloc(sizeof(Edje_Message_Int) + sizeof(int));
	GE_CHECK_VAL(msg, -1);
	msg->val = ((w < h) ? (w / 3) : (h / 3));
	edje_object_message_send(elm_layout_edje_get(thumbs_d->layout), EDJE_MESSAGE_INT, 1, msg);
	free(msg);
	elm_object_signal_emit(thumbs_d->layout, "elm,splitview,state,visible", "elm");
	if (ugd->thumbs_d->b_multifile) {
		elm_object_signal_emit(thumbs_d->layout, "elm,selectall,state,visible,bg", "elm");
		elm_object_signal_emit(thumbs_d->layout, "elm,selectall,state,visible", "elm");
	}
	elm_object_part_text_set(thumbs_d->layout, "select.all.area.label", "Select all");

	Evas_Object *sel_all_ck = elm_check_add(thumbs_d->layout);
	elm_check_state_set(sel_all_ck, EINA_FALSE);
	elm_object_part_content_set(thumbs_d->layout, "select.all.area.check", sel_all_ck);
	Evas_Object *sel_all_btn = elm_button_add(thumbs_d->layout);
	elm_object_part_content_set(thumbs_d->layout, "select.all.area.check.fg", sel_all_btn);
	elm_object_style_set(sel_all_btn, "transparent");
	evas_object_smart_callback_add(sel_all_btn, "clicked", __ge_grid_selall_cb, ugd);
	if (w < h) {
		elm_gengrid_item_size_set(thumbs_d->split_view, (w / 3) - 40, (w / 3) - 20);
	} else {
		elm_gengrid_item_size_set(thumbs_d->split_view, (h / 3) - 20, (h / 3) - 40);
	}
	elm_object_part_content_set(thumbs_d->layout, "contents",
	                            thumbs_d->view);
	elm_object_part_content_set(thumbs_d->layout, "split.view",
	                            thumbs_d->split_view);
	__ge_grid_push_view(ugd, ugd->naviframe, thumbs_d->layout,
	                    ugd->albums_view_title);
	int sel_cnt = _ge_data_get_sel_cnt(ugd);
	if (ugd->thumbs_d->done_it != NULL) {
		if (sel_cnt > 0 && (ugd->max_count < 0 || sel_cnt <= ugd->max_count)) {
			_ge_ui_disable_item(ugd->thumbs_d->done_it, false);
		} else {
			_ge_ui_disable_item(ugd->thumbs_d->done_it, true);
		}
	} else {
		ge_dbgW("done item is NULL");
	}
	int grid_sel_cnt = __ge_grid_get_sel_cnt(ugd);
	if (grid_sel_cnt == ugd->thumbs_d->medias_cnt) {
		ugd->ck_state = true;
	} else {
		ugd->ck_state = false;
	}
	Evas_Object *ck = elm_object_part_content_get(ugd->thumbs_d->layout, "select.all.area.check");
	elm_check_state_set(ck, ugd->ck_state);
	_ge_ui_update_label_text(ugd->thumbs_d->nf_it, sel_cnt,
	                         ugd->albums_view_title);

	if (ugd->thumbs_d->medias_cnt == (GE_FIRST_VIEW_END_POS + 1)) {
		ge_dbgW("Use idler to append other medias--Start");
		GE_IF_DEL_IDLER(ugd->thumbs_d->append_idler)
		ugd->thumbs_d->append_idler = ecore_idler_add(__ge_grid_append_thumb_idler_cb,
		                              ugd);
	} else {
		if (ugd->thumbs_d->b_multifile) {
			__ge_grid_check_sel_items(ugd);
		}
	}
	ugd->rotate_cbs = eina_list_append(ugd->rotate_cbs,
	                                   __ge_grid_rotate_view);
	return 0;

GE_GRID_FAILED:

	if (thumbs_d->gic) {
		elm_gengrid_item_class_free(thumbs_d->gic);
		thumbs_d->gic = NULL;
	}
	return -1;
}

int _ge_grid_create_split_view(ge_ugdata *ugd, ge_thumbs_s *thumbs_d)
{
	GE_CHECK_VAL(thumbs_d, -1);
	ge_dbg(" ");
	thumbs_d->view = NULL;
	thumbs_d->view = __ge_grid_add_view(ugd, thumbs_d->layout);
	if (thumbs_d->view == NULL) {
		goto GE_GRID_FAILED;
	}
	elm_object_part_content_set(thumbs_d->layout, "contents",
	                            thumbs_d->view);
	int sel_cnt = _ge_data_get_sel_cnt(ugd);
	int grid_sel_cnt = __ge_grid_get_sel_cnt(ugd);
	if (grid_sel_cnt == ugd->thumbs_d->medias_cnt) {
		ugd->ck_state = true;
	} else {
		ugd->ck_state = false;
	}
	Evas_Object *ck = elm_object_part_content_get(ugd->thumbs_d->layout, "select.all.area.check");
	elm_object_part_text_set(ugd->thumbs_d->layout, "select.all.area.label", "Select all");
	if (ugd->thumbs_d->b_multifile) {
		elm_object_signal_emit(ugd->thumbs_d->layout, "elm,selectall,state,visible,bg", "elm");
		elm_object_signal_emit(ugd->thumbs_d->layout, "elm,selectall,state,visible", "elm");
	}
	elm_check_state_set(ck, ugd->ck_state);
	_ge_ui_update_label_text(ugd->thumbs_d->nf_it, sel_cnt,
	                         ugd->albums_view_title);

	if (ugd->thumbs_d->medias_cnt == (GE_FIRST_VIEW_END_POS + 1)) {
		ge_dbgW("Use idler to append other medias--Start");
		GE_IF_DEL_IDLER(ugd->thumbs_d->append_idler)
		ugd->thumbs_d->append_idler = ecore_idler_add(__ge_grid_append_thumb_idler_cb,
		                              ugd);
	} else {
		if (ugd->thumbs_d->b_multifile) {
			__ge_grid_check_sel_items(ugd);
		}
	}
	return 0;

GE_GRID_FAILED:

	if (thumbs_d->gic) {
		elm_gengrid_item_class_free(thumbs_d->gic);
		thumbs_d->gic = NULL;
	}
	return -1;
}

int _ge_grid_get_first_data(ge_ugdata *ugd, ge_cluster *album)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(ugd->thumbs_d, -1);
	Eina_List *sel_ids = NULL;
	if (ugd->b_multifile) {
		ge_dbg("Multiple selection");
		ugd->thumbs_d->b_multifile = ugd->b_multifile;
		/* Get ID list of selected items */
		_ge_grid_get_selected_ids(ugd, &sel_ids);
	}
	/* Get album contents from DB */
	_ge_data_get_medias(ugd, album->cluster->uuid, album->cluster->type,
	                    GE_FIRST_VIEW_START_POS, GE_FIRST_VIEW_END_POS,
	                    &(ugd->thumbs_d->medias_elist),
	                    _ge_data_restore_selected, sel_ids);

	ugd->thumbs_d->medias_cnt = eina_list_count(ugd->thumbs_d->medias_elist);
	return ugd->thumbs_d->medias_cnt;
}

int _ge_grid_create_split_view_thumb(ge_cluster *album)
{
	if (back_cnt == 1) {
		A = 0;
	}
	ge_dbgE("split thumb view starts here");
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(album->cluster->uuid, -1);
	GE_CHECK_VAL(album->ugd, -1);
	ge_ugdata *ugd = album->ugd;
	GE_CHECK_VAL(ugd->thumbs_d, -1);
	ge_thumbs_s *thumbs_d = ugd->thumbs_d;

	ugd->thumbs_d->album_uuid = strdup(album->cluster->uuid);
	ugd->thumbs_d->album = album;

	_ge_data_free_medias(&(ugd->thumbs_d->medias_elist));

	_ge_grid_get_first_data(ugd, album);
	ugd->thumbs_d->store_type = album->cluster->type;

	elm_object_item_signal_emit(album->griditem, "elm,state,focused", "elm");

	if (ugd->selected_griditem && album->griditem != ugd->selected_griditem) {
		elm_object_item_signal_emit(ugd->selected_griditem, "elm,state,unfocused", "elm");
	}
	ugd->selected_griditem = album->griditem;

	if (_ge_grid_create_split_view(ugd, thumbs_d) < 0) {
		goto GE_GRID_FAILED;
	}

	return 0;

GE_GRID_FAILED:

	GE_FREEIF(ugd->thumbs_d->album_uuid);
	GE_FREE(thumbs_d);
	ugd->thumbs_d = NULL;
	return -1;
}

int _ge_grid_create_thumb(ge_cluster *album)
{
	if (back_cnt == 1) {
		A = 0;
	}
	ge_dbgE("thumb view starts here");
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(album->cluster, -1);
	GE_CHECK_VAL(album->cluster->uuid, -1);
	GE_CHECK_VAL(album->ugd, -1);
	ge_ugdata *ugd = album->ugd;
	ge_thumbs_s *thumbs_d = NULL;

	thumbs_d = (ge_thumbs_s *)calloc(1, sizeof(ge_thumbs_s));
	GE_CHECK_VAL(thumbs_d, -1);
	ugd->thumbs_d = thumbs_d;
	ugd->thumbs_d->album_uuid = strdup(album->cluster->uuid);
	ugd->thumbs_d->album = album;

	_ge_grid_get_first_data(ugd, album);
	ugd->thumbs_d->store_type = album->cluster->type;

	elm_object_item_signal_emit(album->griditem, "elm,state,focused", "elm");

	if (_ge_grid_create_view(ugd, thumbs_d) < 0) {
		goto GE_GRID_FAILED;
	}

	return 0;

GE_GRID_FAILED:

	GE_FREEIF(ugd->thumbs_d->album_uuid);
	GE_FREE(thumbs_d);
	ugd->thumbs_d = NULL;
	return -1;
}

int _ge_grid_update_view(ge_ugdata *ugd)
{
	ge_dbg("");
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->thumbs_d, -1);

	if (!__ge_grid_append_thumb(ugd, ugd->thumbs_d->view, true)) {
		__ge_grid_del_cbs(ugd->thumbs_d->view);
		return -1;
	} else {
		_ge_thumb_set_size(ugd, ugd->thumbs_d->view, ugd->rotate_mode,
		                   &(ugd->thumbs_d->icon_w),
		                   &(ugd->thumbs_d->icon_h));

		int sel_cnt =  __ge_grid_get_sel_cnt(ugd);
		_ge_ui_update_label_text(ugd->thumbs_d->nf_it, sel_cnt,
		                         ugd->albums_view_title);
		if (!ugd->thumbs_d->b_mainview) {
			sel_cnt = _ge_data_get_sel_cnt(ugd);
			if (ugd->thumbs_d->done_it != NULL) {
				if (sel_cnt > 0 && (ugd->max_count < 0 || sel_cnt <= ugd->max_count)) {
					_ge_ui_disable_item(ugd->thumbs_d->done_it, false);
				} else {
					_ge_ui_disable_item(ugd->thumbs_d->done_it, true);
				}
			} else {
				ge_dbgW("done item is NULL");
			}
		}
	}

	return 0;
}

Evas_Object *_ge_grid_get_layout(ge_ugdata *ugd)
{
	if (ugd && ugd->thumbs_d) {
		return ugd->thumbs_d->layout;
	} else {
		return NULL;
	}
}

int _ge_grid_update_lang(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->thumbs_d, -1);
	ge_dbg("");

	Elm_Object_Item *nf_it = elm_naviframe_top_item_get(ugd->naviframe);
	if (nf_it == NULL) {
		ge_dbgE("Invalid naviframe item!");
		return -1;
	}
	int count = __ge_grid_get_sel_cnt(ugd);
	/* Update the label text */
	_ge_ui_update_label_text(nf_it, count, ugd->albums_view_title);
	return 0;
}

int _ge_grid_update_data(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->thumbs_d, -1);
	Eina_List *sel_ids = NULL;
	ge_dbg("current album: %s", ugd->thumbs_d->album_uuid);

	if (ugd->thumbs_d->b_multifile) {
		ge_dbg("Edit view for multiple selection");
		/* Get ID list of selected items */
		_ge_grid_get_selected_ids(ugd, &sel_ids);
	}

	if (ugd->thumbs_d->nocontents != ugd->thumbs_d->view) {
		_ge_ui_save_scroller_pos(ugd->thumbs_d->view);
		elm_gengrid_clear(ugd->thumbs_d->view);
	}
	/* Clear view data */
	_ge_data_free_medias(&(ugd->thumbs_d->medias_elist));
	ugd->thumbs_d->medias_cnt = 0;

	int ret = -1;
	if (ugd->tab_mode == GE_CTRL_TAB_ALBUMS) {
		ret = _ge_data_update_medias(ugd, ugd->thumbs_d->album_uuid,
		                             ugd->thumbs_d->store_type,
		                             &(ugd->thumbs_d->medias_elist),
		                             _ge_data_restore_selected,
		                             sel_ids);
	}

	if (ret < 0) {
		ugd->thumbs_d->medias_cnt = 0;
	} else {
		ugd->thumbs_d->medias_cnt = eina_list_count(ugd->thumbs_d->medias_elist);
	}

	if (ugd->thumbs_d->b_multifile) {
		__ge_grid_check_sel_items(ugd);
	}

	if (ugd->thumbs_d->medias_cnt <= 0) {
		ge_dbgW("Current album is empty!");
		return -1;
	}
	return 0;
}

int _ge_grid_get_store_type(ge_ugdata *ugd)
{
	if (ugd && ugd->thumbs_d) {
		return ugd->thumbs_d->store_type;
	} else {
		return -1;
	}
}

int _ge_grid_create_main_view(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	ge_thumbs_s *thumbs_d = NULL;

	thumbs_d = (ge_thumbs_s *)calloc(1, sizeof(ge_thumbs_s));
	GE_CHECK_VAL(thumbs_d, -1);
	ugd->thumbs_d = thumbs_d;
	ugd->thumbs_d->store_type = GE_ALL;
	ugd->thumbs_d->b_multifile = ugd->b_multifile;
	ugd->thumbs_d->b_mainview = true;

	switch (ugd->slideshow_viewby) {
	case GE_VIEW_BY_ALL:
	case GE_VIEW_BY_ALBUMS:
		if (!ugd->slideshow_album_id) {
			goto GE_GRID_FAILED;
		}
		ugd->thumbs_d->album_uuid = strdup(ugd->slideshow_album_id);
		/* Get album contents from DB */
		_ge_data_get_medias(ugd, ugd->slideshow_album_id, GE_ALL,
		                    GE_FIRST_VIEW_START_POS,
		                    GE_FIRST_VIEW_END_POS,
		                    &(ugd->thumbs_d->medias_elist), NULL, NULL);
		break;
	default:
		goto GE_GRID_FAILED;
	}

	ugd->thumbs_d->medias_cnt = eina_list_count(ugd->thumbs_d->medias_elist);

	if (_ge_grid_create_view(ugd, thumbs_d) < 0) {
		goto GE_GRID_FAILED;
	}
	return 0;

GE_GRID_FAILED:

	GE_FREEIF(ugd->thumbs_d->album_uuid);
	GE_FREE(thumbs_d);
	ugd->thumbs_d = NULL;
	return -1;
}

void _ge_grid_change_selection_mode(ge_ugdata *ugd, bool state)
{
	GE_CHECK(ugd);
	GE_CHECK(ugd->thumbs_d);
	ge_dbg("changing selection type and mode is %d", _ge_get_view_mode(ugd));
#if 0
	ugd->selected_elist = NULL;
	Evas_Object *grid = NULL;
	if (_ge_get_view_mode(ugd) == GE_VIEW_THUMBS_EDIT || _ge_get_view_mode(ugd) == GE_VIEW_THUMBS) {
		if (ugd->thumbs_d) {
			ugd->thumbs_d->b_multifile = state;
			_ge_grid_update_view(ugd);
			Evas_Object *btn1 = elm_button_add(ugd->naviframe);
			if (state) {
				elm_object_style_set(btn1, "naviframe/title_left");
				elm_object_text_set(btn1, GE_STR_ID_CANCEL_CAP);
				elm_object_item_part_content_set(ugd->thumbs_d->nf_it, GE_NAVIFRAME_TITLE_LEFT_BTN, btn1);
				Evas_Object *temp = elm_object_item_part_content_unset(ugd->thumbs_d->nf_it,
				                    "prev_btn");
				ge_dbg("Multiple selection, show Done");
				/* Done */
				__ge_grid_add_btns(ugd, ugd->naviframe, ugd->thumbs_d->nf_it);
				Evas_Object *btn2 = elm_button_add(ugd->naviframe);
				elm_object_style_set(btn2, "naviframe/title_right");
				elm_object_text_set(btn2, GE_STR_ID_DONE_CAP);
				elm_object_disabled_set(btn2, EINA_TRUE);
				elm_object_item_part_content_set(ugd->thumbs_d->nf_it,
				                                 GE_NAVIFRAME_TITLE_RIGHT_BTN,
				                                 btn2);
				ugd->ck_state = EINA_FALSE;
				elm_object_signal_emit(ugd->thumbs_d->layout, "elm,selectall,state,visible,bg", "elm");
				elm_object_signal_emit(ugd->thumbs_d->layout, "elm,selectall,state,visible", "elm");
				if (temp) {
					evas_object_hide(temp);
				}
			} else {
				Evas_Object *temp = elm_object_item_part_content_unset(ugd->thumbs_d->nf_it,
				                    GE_NAVIFRAME_TITLE_RIGHT_BTN);
				elm_object_style_set(btn1, "naviframe/end_btn/default");
				elm_object_item_part_content_set(ugd->thumbs_d->nf_it, "prev_btn", btn1);
				Evas_Object *temp1 = elm_object_item_part_content_unset(ugd->thumbs_d->nf_it,
				                     GE_NAVIFRAME_TITLE_LEFT_BTN);
				elm_object_signal_emit(ugd->thumbs_d->layout, "elm,selectall,state,default", "elm");
				if (temp) {
					evas_object_hide(temp);
				}
				if (temp1) {
					evas_object_hide(temp1);
				}

				//elm_gengrid_realized_items_update(ugd->thumbs_d->split_view);
				_ge_data_free_sel_albums(ugd);
				Elm_Object_Item *it = NULL;
				it = elm_gengrid_first_item_get(ugd->thumbs_d->split_view);
				while (it) {
					elm_gengrid_item_update(it);
					elm_object_item_signal_emit((Elm_Object_Item *)it,
					                            "elm,state,elm.text.badge,hidden",
					                            "elm");
					it = elm_gengrid_item_next_get(it);
				}

				Evas_Object *ck = elm_object_part_content_get(ugd->thumbs_d->layout, "select.all.area.check");
				ugd->ck_state = false;
				elm_check_state_set(ck, ugd->ck_state);
			}
			evas_object_smart_callback_add(btn1, "clicked", __ge_albums_detail_cancel_cb, ugd);

			grid = ugd->thumbs_d->view;
		} else {
			ge_dbg("thumb data is null ");
		}
	} else if (_ge_get_view_mode(ugd) == GE_VIEW_ALBUMS) {
		grid = ugd->albums_view;
	}
#endif

	if (ugd->thumbs_d->gengrid) {
		if (ugd->is_attach_panel && (ugd->attach_panel_display_mode != ATTACH_PANEL_FULL_MODE)) {
			elm_scroller_movement_block_set(ugd->thumbs_d->gengrid, ELM_SCROLLER_MOVEMENT_BLOCK_VERTICAL);
		} else {
			elm_scroller_movement_block_set(ugd->thumbs_d->gengrid, ELM_SCROLLER_MOVEMENT_NO_BLOCK);
		}
	}
}

bool _ge_is_grid_view(ge_ugdata *ugd)
{
	if (ugd->thumbs_d && ugd->thumbs_d->nf_it) {
		return true;
	}
	return false;
}
