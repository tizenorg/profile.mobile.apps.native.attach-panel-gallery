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

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#include <Evas.h>
#include <app.h>
#include "gallery-efl.h"
#include "ge-albums.h"
#include "ge-gridview.h"
#include "ge-data.h"
#include "ge-ui-util.h"
#include "ge-util.h"
#include "ge-db-update.h"
#include "ge-ext-ug-load.h"
#include "ge-lang.h"
#include "ge-main-view.h"

#define GE_BG_COLOR_DEFAULT 0
#define GE_BG_COLOR_A 255

static int __ge_create_main_view(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->naviframe, -1);

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SLIDESHOW) {
		ge_dbgE("file select mode is slideshow>>>>>>> grid view");
		_ge_grid_create_main_view(ugd);
	} else {

		ge_dbgE("no slide show mode>>>>>>>>>");
		_ge_main_create_view(ugd);
	}
	return 0;
}

static Evas_Object *_ge_create_bg(Evas_Object *parent)
{
	ge_dbg("");
	GE_CHECK_NULL(parent);
	Evas_Object *bg = NULL;

	/* Show special color of background */
	bg = evas_object_rectangle_add(evas_object_evas_get(parent));
	evas_object_color_set(bg, GE_BG_COLOR_DEFAULT, GE_BG_COLOR_DEFAULT,
	                      GE_BG_COLOR_DEFAULT, GE_BG_COLOR_A);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(bg);

	return bg;
}

/**
 * Add windown rotation for PLM bug: P130605-7106.
 * Before we got rotation event from caller,
 * but it took much time, so view updating/rotation is visible for user
 * according to bug description.
 *
 * After we add window rotation callback to get rotation event as fast as possible,
 * so view updating/rotation is invisible for user.
*/
static void __ge_win_rot_changed_cb(void *data, Evas_Object *obj,
					 void *event)
{
	GE_CHECK(data);
	GE_CHECK(obj);
	ge_ugdata *ugd = (ge_ugdata *)data;

	/* changed_ang value is 0 90 180 270 */
	int changed_ang = elm_win_rotation_get(obj);
	/* Send event to UG */
	enum ug_event evt = UG_EVENT_NONE;
	switch (changed_ang) {
	case APP_DEVICE_ORIENTATION_0:
		evt = GE_ROTATE_PORTRAIT;
		break;
	case APP_DEVICE_ORIENTATION_90:
		evt = GE_ROTATE_LANDSCAPE_UPSIDEDOWN;
		break;
	case APP_DEVICE_ORIENTATION_180:
		evt = GE_ROTATE_PORTRAIT_UPSIDEDOWN;
		break;
	case APP_DEVICE_ORIENTATION_270:
		evt = GE_ROTATE_LANDSCAPE;
		break;
	}

	ge_dbg("New angle: %d, old angle: %d", evt, ugd->rotate_mode);
	if (evt == ugd->rotate_mode) {
		return;
	} else {
		ugd->rotate_mode = evt;
	}

	/* Update rotate mode and view */
	int len = eina_list_count(ugd->rotate_cbs);
	ge_rotate_view_cb rotate_cb = eina_list_nth(ugd->rotate_cbs, len - 1);
	if (rotate_cb) {
		ge_dbg("To rotate view");
		rotate_cb(ugd);
	} else {
		ge_dbgE("None callbacks exists[%d]!", len);
	}
}

static int _ge_init_view(ge_ugdata *ugd)
{
	ge_dbg("");
	GE_CHECK_VAL(ugd, -1);

	/* Base Layout */
	ugd->ly_main = ge_ui_create_main_ly(ugd->ly_parent);
	GE_CHECK_VAL(ugd->ly_main, -1);

	/* Background */
	ugd->bg = _ge_create_bg(ugd->ly_main);
	GE_CHECK_VAL(ugd->bg, -1);
	/* Pack bg to swallow */
	elm_object_part_content_set(ugd->ly_main, "elm.swallow.bg", ugd->bg);

	/* Navigation Bar */
	ugd->naviframe = ge_ui_create_naviframe(ugd, ugd->ly_main);
	GE_CHECK_VAL(ugd->naviframe, -1);

	ge_dbgE("base view is pushed in the naviframe instead of content set");



	ugd->nf_it = elm_naviframe_item_push(ugd->naviframe, "first naviframe bg", NULL, NULL, ugd->bg, NULL);



	if (ugd->nf_it != NULL) {
		ge_dbgE("base view is push successfully in the nf");
	} else {
		ge_dbgE("base view failed to push in nf ");
	}


	/* Albums view and its layout */
	if (__ge_create_main_view(ugd) != 0) {
		ge_dbgE("Create albums view and layout failed!");
		return -1;
	}

	return 0;
}

static int _ge_create_view(ge_ugdata *ugd)
{
	ge_dbg("");
	GE_CHECK_VAL(ugd, -1);
	/* Add new theme */
	ugd->th = elm_theme_new();
	GE_CHECK_VAL(ugd->th, -1);
	elm_theme_ref_set(ugd->th, NULL);
	elm_theme_set(ugd->th, "tizen-HD-dark");
	elm_theme_extension_add(ugd->th, GE_EDJ_FILE);

	if (_ge_init_view(ugd) != 0) {
		ge_dbgE("Initialize view failed!");
		return -1;
	} else {
		ge_dbgE("view is intialized !!!!!!!!!!!!!");
	}

	/* Register MMC changed callback */
	if (ge_reg_db_update_noti(ugd) != 0) {
		ge_dbgE("reg_db_update_noti failed!");
		return -1;
	} else {
		ge_dbgE("reg_db is updated storage state change");
	}

	return 0;
}

static int _ge_close_view(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);

	/************* Remove Callbacks **************/
	GE_IF_DEL_TIMER(ugd->del_timer)
	/* Remove win rotation callback */
	evas_object_smart_callback_del(ugd->win, "wm,rotation,changed",
	                               __ge_win_rot_changed_cb);
	ge_dereg_db_update_noti();

	/************* Destroy UI objects **************/
	/* Destroy UG called by me */
	if (ugd->ug_called_by_me) {
		ge_dbg("Destroy ug_called_by_me");
		ug_destroy(ugd->ug_called_by_me);
		ugd->ug_called_by_me = NULL;
	}
	GE_IF_DEL_OBJ(ugd->popup)
	GE_IF_DEL_OBJ(ugd->ly_main)
	if (ugd->th) {
		elm_theme_extension_del(ugd->th, GE_EDJ_FILE);
		elm_theme_free(ugd->th);
		ugd->th = NULL;
	}
#ifdef _USE_HIDE_INDICATOR
	_ge_ui_reset_indicator(ugd);
#endif

	/************* Free data **************/
	if (ugd->service) {
		app_control_destroy(ugd->service);
		ugd->service = NULL;
	}
	GE_FREEIF(ugd->file_setas_image_path);
	GE_FREEIF(ugd->file_setas_crop_image_path);
	GE_FREEIF(ugd->slideshow_album_id);
	GE_FREEIF(ugd->ug_path);
	/*Finalize db update callback*/
	_ge_db_update_finalize(ugd);
	/* Disconnet with libmedia-info */
	_ge_data_finalize(ugd);
	return 0;
}

static int __ge_parse_param_setas(ge_ugdata *ugd, app_control_h service)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(service, -1);
	char *setas_type = NULL;

	app_control_get_extra_data(service, GE_BUNDLE_SETAS_TYPE, &setas_type);
	if (setas_type == NULL) {
		ge_dbgW("setas-type NULL");
		return -1;
	}

	if (!strcasecmp(setas_type, GE_SETAS_WALLPAPER)) {
		ugd->file_select_setas_mode = GE_SETAS_T_WALLPAPER;
	} else if (!strcasecmp(setas_type, GE_SETAS_LOCKSCREEN)) {
		ugd->file_select_setas_mode = GE_SETAS_T_LOCKPAPER;
	} else if (!strcasecmp(setas_type, GE_SETAS_WALLPAPER_AND_LOCKSCREEN)) {
		ugd->file_select_setas_mode = GE_SETAS_T_WALLPAPER_LOCKPAPER;
	} else if (!strcasecmp(setas_type, GE_SETAS_CROP_WALLPAPER)) {
		ugd->file_select_setas_mode = GE_SETAS_T_CROP_WALLPAPER;
	} else if (!strcasecmp(setas_type, GE_SETAS_CALLERID)) {
		ugd->file_select_setas_mode = GE_SETAS_T_CALLERID;
	} else if (!strcasecmp(setas_type, "crop")) {
		ugd->file_select_setas_mode = GE_SETAS_T_CROP_WALLPAPER;
	}
	GE_FREE(setas_type);
	return 0;
}

static int __ge_parse_param_file(ge_ugdata *ugd, app_control_h service)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(service, -1);
	char *file_type = NULL;
	char *mime = NULL;

	app_control_get_mime(service, &mime);
	if (mime) {
		if (!g_strcmp0(mime, GE_MIME_IMAGE_ALL)) {
			ugd->file_type_mode = GE_FILE_T_IMAGE;
		} else if (!g_strcmp0(mime, GE_MIME_VIDEO_ALL)) {
			ugd->file_type_mode = GE_FILE_T_VIDEO;
		} else {
			ugd->file_type_mode = GE_FILE_T_ALL;
		}

		return 0;
	}

	/* file-type */
	app_control_get_extra_data(service, GE_BUNDLE_FILE_TYPE, &file_type);
	if (file_type == NULL) {
		ge_dbgW("file_type is empty!");
		goto GE_DEFAULT_FILE_TYPE;
	}
	ge_dbg("file_type: %s", file_type);

	if (!strcasecmp(file_type, GE_FILE_TYPE_IMAGE)) {
		ugd->file_type_mode = GE_FILE_T_IMAGE;
		GE_FREE(file_type);
		return 0;
	} else if (!strcasecmp(file_type, GE_FILE_TYPE_VIDEO)) {
		ugd->file_type_mode = GE_FILE_T_VIDEO;
		GE_FREE(file_type);
		return 0;
	}


GE_DEFAULT_FILE_TYPE:

	ge_dbg("file type is default, set default type(ALL).");
	ugd->file_type_mode = GE_FILE_T_ALL;
	return 0;
}

static int __ge_parse_param_slideshow(ge_ugdata *ugd, app_control_h service)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(service, -1);
	ugd->file_select_mode = GE_FILE_SELECT_T_SLIDESHOW;
	ugd->b_multifile = true;
	char *viewby = NULL;
	char *tmp = NULL;

	app_control_get_extra_data(service, "view-by", &viewby);
	if (viewby) {
		if (!strcasecmp(viewby, "all")) {
			ugd->slideshow_viewby = GE_VIEW_BY_ALL;
			ugd->slideshow_album_id = g_strdup(GE_ALBUM_ALL_ID);
			ge_sdbg("album id: %s", ugd->slideshow_album_id);
		} else if (!strcasecmp(viewby, "albums")) {
			ugd->slideshow_viewby = GE_VIEW_BY_ALBUMS;
			app_control_get_extra_data(service, "album-id", &tmp);
			if (tmp) {
				ugd->slideshow_album_id = g_strdup(tmp);
				GE_FREE(tmp);
			} else {
				ugd->slideshow_album_id = g_strdup(GE_ALBUM_ALL_ID);
			}
			ge_sdbg("album id: %s", ugd->slideshow_album_id);
		}
		GE_FREE(viewby);
	} else {
		ge_dbgW("vieweby does not exist! set viewby all as default");
		ugd->slideshow_viewby = GE_VIEW_BY_ALL;
		ugd->slideshow_album_id = g_strdup(GE_ALBUM_ALL_ID);
	}

	ge_sdbg("viewby: %d", ugd->slideshow_viewby);
	return 0;
}

/* analysis parameters */
static int _ge_parse_param(ge_ugdata *ugd, app_control_h service)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(service, -1);
	char *launch_type = NULL;
	char *select_mode = NULL;
	char *operation = NULL;

	app_control_get_operation(service, &operation);
	ge_sdbg("operation [%s]", operation);

	app_control_get_extra_data(service, APP_CONTROL_DATA_SELECTION_MODE,
	                           &select_mode);
	if (select_mode) {
		if (!strcasecmp(select_mode, GE_BUNDLE_SELECTION_MODE_SINGLE)) {
			launch_type = strdup(GE_LAUNCH_SELECT_ONE);
		} else if (!strcasecmp(select_mode, GE_BUNDLE_SELECTION_MODE_MULTI)) {
			launch_type = strdup(GE_LAUNCH_SELECT_MULTIPLE);
		}
		GE_FREE(select_mode);
	}
	if (launch_type == NULL)
		app_control_get_extra_data(service, GE_BUNDLE_LAUNCH_TYPE,
		                           &launch_type);
	if (launch_type == NULL) {
		if (operation &&
		        !strcasecmp(operation, APP_CONTROL_OPERATION_PICK)) {
			launch_type = strdup(GE_LAUNCH_SELECT_ONE);
			ge_dbg("Set launch-type to default");
		} else {
			GE_FREEIF(operation);
			ge_dbg("launch-type = NULL, galery ug exit");
			return -1;
		}
	}
	if (launch_type) {
		ge_dbg("launch-type [%s]", launch_type);
		if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_ALBUM)) {
			ugd->album_select_mode = GE_ALBUM_SELECT_T_ONE;
		} else if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_SETAS)) {
			ugd->file_select_mode = GE_FILE_SELECT_T_SETAS;
			__ge_parse_param_setas(ugd, service);
			/* Checkme: Only image type is offered in SETAS  case */
			ugd->file_type_mode = GE_FILE_T_IMAGE;
		} else {
			if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_ONE)) {
				ugd->file_select_mode = GE_FILE_SELECT_T_ONE;
			} else if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_MULTIPLE)) {
				ugd->file_select_mode = GE_FILE_SELECT_T_MULTIPLE;
				ugd->b_multifile = true;
			} else if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_IMFT)) {
				ugd->file_select_mode = GE_FILE_SELECT_T_IMFT;
				ugd->b_multifile = true;

				char *contact_id = NULL;
				app_control_get_extra_data(service, "contact", &contact_id);
				if (contact_id) {
					ugd->file_select_contact_id = atoi(contact_id);
					GE_FREE(contact_id);
				} else {
					ugd->file_select_mode = GE_FILE_SELECT_T_NONE;
					ugd->file_select_contact_id = -1;
					ge_dbg("Invalid args");
					GE_FREEIF(operation);
					GE_FREEIF(launch_type);
					return -1;
				}

				ge_sdbg("contact_id: %d", ugd->file_select_contact_id);
			} else if (!strcasecmp(launch_type, GE_LAUNCH_SELECT_SLIDESHOW)) {
				__ge_parse_param_slideshow(ugd, service);
			} else {
				ge_dbgE("Wrong launch type!");
			}

			__ge_parse_param_file(ugd, service);
		}
	} else {
		//using default values
		ge_dbg("Set launch-type to default");
		ugd->album_select_mode = GE_ALBUM_SELECT_T_ONE;
		ugd->b_multifile = false;
	}

	ugd->max_count = -1;

	char *max_cnt = NULL;
	app_control_get_extra_data(service, APP_CONTROL_DATA_TOTAL_COUNT, &max_cnt);
	if (max_cnt) {
		ugd->max_count = _ge_atoi(max_cnt);
		GE_FREE(max_cnt);
	}
	ge_sdbg("max count: %d", ugd->max_count);

	if (!ugd->overlap_mode) {
		char *indicator = NULL;
		app_control_get_extra_data(service, "indicator-state", &indicator);
		if (indicator) {
			ge_dbg("indicator: %s", indicator);
			if (!strcasecmp(indicator, "hide")) {
				ugd->b_hide_indicator = true;
			}
			GE_FREE(indicator);
		}
	}

	GE_FREEIF(operation);
	GE_FREEIF(launch_type);
	return 0;
}

static int __ge_get_rotate_value(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->win, -1);
	/* changed_ang value is 0 90 180 270 */
	int changed_ang = elm_win_rotation_get(ugd->win);
	/* Send event to UG */
	switch (changed_ang) {
	case APP_DEVICE_ORIENTATION_0:
		ugd->rotate_mode = GE_ROTATE_PORTRAIT;
		break;
	case APP_DEVICE_ORIENTATION_90:
		ugd->rotate_mode = GE_ROTATE_LANDSCAPE_UPSIDEDOWN;
		break;
	case APP_DEVICE_ORIENTATION_180:
		ugd->rotate_mode = GE_ROTATE_PORTRAIT_UPSIDEDOWN;
		break;
	case APP_DEVICE_ORIENTATION_270:
		ugd->rotate_mode = GE_ROTATE_LANDSCAPE;
		break;
	}
	return ugd->rotate_mode;
}

/**
* @brief
*
* @param ug
* @param mode
* @param data
* @param priv
*
* @return
*/
static void * _ge_create(ui_gadget_h ug, enum ug_mode mode, app_control_h service, void *priv)
{
	ge_dbg("Enter...");
	ge_ugdata *ugd = NULL;
	GE_CHECK_NULL(priv);
	GE_CHECK_NULL(service);
	GE_CHECK_NULL(ug);
	ge_dbgW("Gallery UG start...");

	ugd = (ge_ugdata *)priv;
	ugd->ug = ug;
	ugd->attach_panel_display_mode = ATTACH_PANEL_NONE;
	/* Get window */
	ugd->win = (Evas_Object *)ug_get_window();
	GE_CHECK_NULL(ugd->win);
	/* Get conformant */
	ugd->conform = ug_get_conformant();
	GE_CHECK_NULL(ugd->conform);
	/* Get caller layout */
	ugd->ly_parent = (Evas_Object *)ug_get_parent_layout(ug);
	GE_CHECK_NULL(ugd->ly_parent);
	/* Bind text domain for internalization */
	bindtextdomain("attach-panel-gallery" , "/usr/ug/res/locale");
	/* Reset inited flag, it would be set as TRUE if albums view created */
	_ge_ui_get_indicator_state(ugd);
#ifdef _USE_HIDE_INDICATOR
	_ge_ui_hide_indicator(ugd);
#endif
	/* Add window rotation callback to rotate view as fast as possible */
	evas_object_smart_callback_add(ugd->win, "wm,rotation,changed",
	                               __ge_win_rot_changed_cb, (void *)ugd);
	__ge_get_rotate_value(ugd);

	app_control_clone(&(ugd->service), service);
	/* Connect DB first */
	if (_ge_data_init(ugd) != 0) {
		ge_dbgE("_ge_data_init failed!");
	}
	/*Register db udpate callback*/
	_ge_db_update_reg_cb(ugd);
	/* Parse parameters passed from parent */
	if (_ge_parse_param(ugd, service) != 0) {
		ge_dbgE("Failed to parse parameters!");
		ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_FAILED);
		return NULL;
	}
	char *contact_id = NULL;
	app_control_get_extra_data(service, "__CALLER_PANEL__", &contact_id);
	if (contact_id && !strcmp(contact_id, "attach-panel")) {
		if (ugd) {
			ugd->is_attach_panel = true;
		}
	} else {
		ugd->is_attach_panel = false;
	}

	if (ugd->overlap_mode) {
		ugd->b_hide_indicator = true;
	} else if (ugd->b_hide_indicator) {
		_ge_ui_hide_indicator(ugd);
	} else {
		ge_dbg("Normal mode");
	}
	if ((ugd->file_select_mode != GE_FILE_SELECT_T_NONE) ||
	        (ugd->album_select_mode != GE_ALBUM_SELECT_T_NONE)) {
		/* create gallery UG */
		if (_ge_create_view(ugd) != 0) {
			ge_dbgE("Failed to create Gallery UG view!");
			return NULL;
		}
	} else {
		ge_dbgE("Wrong file_select_mode[%d] or album_select_mode[%d]",
		        ugd->file_select_mode, ugd->album_select_mode);
	}

	return ugd->ly_main;
}

/**
* @brief
*
* @param ug
* @param data
* @param priv
*/
static void _ge_start(ui_gadget_h ug, app_control_h service, void *priv)
{
}

/**
* @brief
*
* @param ug
* @param data
* @param priv
*/
static void _ge_pause(ui_gadget_h ug, app_control_h service, void *priv)
{
	ge_dbg("");
}

/**
* @brief
*
* @param ug
* @param data
* @param priv
*/
static void _ge_resume(ui_gadget_h ug, app_control_h service, void *priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	ge_ugdata *ugd = (ge_ugdata *)priv;
	/*update*/

	if (ugd->ug_called_by_me == NULL &&
	        ugd->file_select_mode != GE_FILE_SELECT_T_SLIDESHOW) {
		ge_update_view(ugd, GE_UPDATE_NONE);
	}
}

/**
* @brief
*
* @param ug
* @param data
* @param priv
*/
static void _ge_destroy(ui_gadget_h ug, app_control_h service, void *priv)
{
	ge_dbgW("");
	GE_CHECK(ug);
	GE_CHECK(priv);
	_ge_close_view((ge_ugdata *)priv);
	ge_dbgW("Destroy gallery UG done!");
}

/**
* @brief
*
* @param ug
* @param msg
* @param data
* @param priv
*/
static void _ge_message(ui_gadget_h ug, app_control_h msg, app_control_h service, void *priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	ge_ugdata *ugd = (ge_ugdata *)priv;
	char *display_mode = NULL;
	char *initialized_mode = NULL;
	char *pd_selected = GE_STR_PD_SELECTED;
	char *text = NULL;
	Evas_Object *btn = NULL;
	int win_x = 0;
	int win_y = 0;
	int win_w = 0;
	int win_h = 0;

	elm_win_screen_size_get(ugd->win, &win_x, &win_y, &win_w, &win_h);
	if (ugd->is_attach_panel) {
		ge_dbg("called by attach panel ");
		app_control_get_extra_data(msg, APP_CONTROL_DATA_SELECTION_MODE, &display_mode);
		app_control_get_extra_data(msg, "__ATTACH_PANEL_INITIALIZE__", &initialized_mode);
#ifdef FEATURE_SIZE_CHECK
		int ret = app_control_get_extra_data(msg, "http://tizen.org/appcontrol/data/total_size", &(ugd->limitsize));
		if (ret != APP_CONTROL_ERROR_NONE) {
			//Test data
			//ugd->limitsize = 2 * 1024 * 1024;
		}
#endif
		if (display_mode) {
			if (!strcmp(display_mode, "single")) {
				//change to single selection
				ge_dbg("single_selection ");
				if (ugd->nocontents) {
					elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, win_w, win_h / 3);
				}
				ugd->attach_panel_display_mode = ATTACH_PANEL_HALF_MODE;
				ugd->file_select_mode = GE_FILE_SELECT_T_ONE;
				ugd->b_multifile = false;
				_ge_grid_change_selection_mode(ugd, false);
				ugd->thumbs_d->b_editmode = false;

				elm_gengrid_realized_items_update(ugd->thumbs_d->gengrid);

				Eina_List *l = NULL;
				ge_item *gitem = NULL;

				EINA_LIST_FOREACH(ugd->thumbs_d->medias_elist, l, gitem) {
					if (gitem) {
						gitem->checked = false;
					}
				}

				if (ugd->selected_elist) {
					ugd->selected_elist = eina_list_free(ugd->selected_elist);
				}

				ugd->thumbs_d->tot_selected = 0;
				elm_naviframe_item_title_enabled_set(ugd->nf_it, EINA_FALSE, EINA_FALSE);
			} else if (display_mode && !strcmp(display_mode, "multiple")) {
				//change to multi selection
				ge_dbg("multiple_selection");
				if (ugd->nocontents) {
					if (ugd->rotate_mode == GE_ROTATE_LANDSCAPE_UPSIDEDOWN || ugd->rotate_mode == GE_ROTATE_LANDSCAPE) {
						elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, win_h, LANDSCAPE_HEIGHT);
					} else {
						elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, win_w, FULL_MODE_PORTRAIT_HEIGHT);
					}
				}
				ugd->attach_panel_display_mode = ATTACH_PANEL_FULL_MODE;
				ugd->file_select_mode = GE_FILE_SELECT_T_MULTIPLE;
				ugd->b_multifile = true;
				_ge_grid_change_selection_mode(ugd, true);
				ugd->thumbs_d->b_editmode = true;

				elm_gengrid_realized_items_update(ugd->thumbs_d->gengrid);

				ugd->thumbs_d->tot_selected = 0;
				/*Update Title*/
				elm_naviframe_item_title_enabled_set(ugd->nf_it, EINA_TRUE, EINA_FALSE);
				text = g_strdup_printf(pd_selected, ugd->thumbs_d->tot_selected);
				elm_object_item_text_set(ugd->nf_it, text);

				/*Disable DONE Button if count is zero */
				if (!ugd->thumbs_d->tot_selected) {
					btn = elm_object_item_part_content_get(ugd->nf_it , GE_NAVIFRAME_TITLE_RIGHT_BTN);
					if (btn == NULL) {
						ge_dbgE("Failed to get part information");
					}
					elm_object_disabled_set(btn, EINA_TRUE);
				}
			} else {
				ge_dbg("invalid mode: %s", display_mode);
			}
		} else if (initialized_mode) {
			if (!strcmp(initialized_mode, "enable")) {
				if (_ge_is_grid_view(ugd)) {
					elm_naviframe_item_pop(ugd->naviframe);
				} else {
					ge_dbg("Album view");
				}
				elm_naviframe_item_pop(ugd->naviframe);
				elm_gengrid_realized_items_update(ugd->albums_view);
			}
		}
	}
}

/**
* @brief
*
* @param ug
* @param event
* @param data
* @param priv
*/
static void _ge_event(ui_gadget_h ug, enum ug_event event, app_control_h service, void *priv)
{
	GE_CHECK(priv);
	ge_ugdata *ugd = (ge_ugdata *)priv;

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		ge_dbg("UG_EVENT_LOW_MEMORY");
		break;

	case UG_EVENT_LOW_BATTERY:
		ge_dbg("UG_EVENT_LOW_BATTERY");
		break;

	case UG_EVENT_LANG_CHANGE:
		ge_dbg("UG_EVENT_LANG_CHANGE");
		_ge_lang_update(ugd);
		break;

	case UG_EVENT_ROTATE_PORTRAIT:
		ge_dbg("UG_EVENT_ROTATE_PORTRAIT");
		goto UG_ROTATE_EVENT;
		break;

	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		ge_dbg("UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN");
		goto UG_ROTATE_EVENT;
		break;

	case UG_EVENT_ROTATE_LANDSCAPE:
		ge_dbg("UG_EVENT_ROTATE_LANDSCAPE");
		goto UG_ROTATE_EVENT;
		break;

	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		ge_dbg("UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN");
		goto UG_ROTATE_EVENT;
		break;

	default:
		break;
	}

	return;

UG_ROTATE_EVENT:

	/* Update rotate mode and view */
	ge_dbg("rotate_mode: %d", ugd->rotate_mode);

	int win_x = 0;
	int win_y = 0;
	int win_w = 0;
	int win_h = 0;

	elm_win_screen_size_get(ugd->win, &win_x, &win_y, &win_w, &win_h);
	int size = (win_w / 4);
	int landscape_size = (win_h / 7);

	if (ugd->rotate_mode == GE_ROTATE_LANDSCAPE_UPSIDEDOWN || ugd->rotate_mode == GE_ROTATE_LANDSCAPE) {
		if (ugd->nocontents) {
			elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, win_h, LANDSCAPE_HEIGHT);
		} else {
			elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, landscape_size, landscape_size);
		}
	} else {
		if (ugd->nocontents) {
			if (ugd->attach_panel_display_mode == ATTACH_PANEL_FULL_MODE) {
				elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, win_w, FULL_MODE_PORTRAIT_HEIGHT);
			} else {
				elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, win_w, HALF_MODE_PORTRAIT_HEIGHT);
			}
		} else {
			elm_gengrid_item_size_set(ugd->thumbs_d->gengrid, size, size);
		}
	}
}

static void _ge_key_event(ui_gadget_h ug, enum ug_key_event event, app_control_h service, void *priv)
{
	ge_dbg("");
	GE_CHECK(ug);
	GE_CHECK(priv);
	ge_ugdata *ugd = (ge_ugdata *)priv;
	switch (event) {
	case UG_KEY_EVENT_END:
		ge_dbg("Receive key end event");
		if (!ugd->is_attach_panel) {
			ug_destroy_me(ugd->ug);
		}
		break;

	default:
		break;
	}
}

/**
* @brief
*
* @param ops
*
* @return
*/
UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	ge_dbgW("UG_MODULE_INIT");
	ge_ugdata *ugd = NULL;
	GE_CHECK_VAL(ops, -1);

	ugd = calloc(1, sizeof(ge_ugdata));
	GE_CHECK_VAL(ugd, -1);

	ops->create = _ge_create;
	ops->start = _ge_start;
	ops->pause = _ge_pause;
	ops->resume = _ge_resume;
	ops->destroy = _ge_destroy;
	ops->message = _ge_message;
	ops->event = _ge_event;
	ops->key_event = _ge_key_event;
	ops->priv = ugd;
	ops->opt = UG_OPT_INDICATOR_ENABLE;

	return 0;
}

/**
* @brief
*
* @param ops
*
* @return
*/
UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	GE_CHECK(ops);
	GE_FREEIF(ops->priv);
	ge_dbgW("UG_MODULE_EXIT");
}

