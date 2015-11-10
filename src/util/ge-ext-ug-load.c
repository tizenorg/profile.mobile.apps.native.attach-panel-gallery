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

#include "ge-ext-ug-load.h"
#include "ge-debug.h"
#include "ge-ui-util.h"
#include "ge-util.h"
#include "ge-albums.h"
#include "ge-gridview.h"

#define GE_IV_UG_NAME "image-viewer-efl"
#define GE_IV_STR_LEN_MAX 32
#define GE_VIEW_MODE "View Mode"
#define GE_SETAS_TYPE "Setas type"
#define GE_VIEW_BY "View By"
#define GE_MEDIA_TYPE "Media type"
#define GE_PATH "Path"

static void _ge_ext_destroy_me(ge_ugdata *ugd)
{
	ge_dbg("");
	GE_CHECK(ugd);
	GE_CHECK(ugd->ug_called_by_me);
	GE_CHECK(ugd->ug);
	GE_CHECK(ugd->service);
	bool send_result = false;

	if (ugd->ug_called_by_me) {
		ge_dbg("Destroy ug_called_by_me");
		ug_destroy(ugd->ug_called_by_me);
		ugd->ug_called_by_me = NULL;
	} else {
		ge_dbg("ug_called_by_me does not exist!");
	}

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SLIDESHOW) {
		ugd->b_destroy_me = false;
	}

	if (!ugd->b_destroy_me) {
		ge_dbg("gallery ug is still alive");
		return;
	}
	if (ugd->file_select_mode == GE_FILE_SELECT_T_SETAS) {
		if (ugd->file_setas_image_path) {
			ge_dbg("GE_SETAS_IMAGE_PATH:%s", ugd->file_setas_image_path);
			app_control_add_extra_data(ugd->service,
			                           GE_SETAS_IMAGE_PATH,
			                           ugd->file_setas_image_path);

			GE_FREE(ugd->file_setas_image_path);
			send_result = true;
		}

		if (ugd->file_setas_crop_image_path &&
		        (ugd->file_select_setas_mode == GE_SETAS_T_CALLERID ||
		         ugd->file_select_setas_mode == GE_SETAS_T_CROP_WALLPAPER)) {
			ge_dbg("GE_SETAS_CALLERID_CROP_IMAGE_PATH:%s",
			       ugd->file_setas_crop_image_path);
			app_control_add_extra_data(ugd->service,
			                           APP_CONTROL_DATA_SELECTED,
			                           ugd->file_setas_crop_image_path);

			GE_FREE(ugd->file_setas_crop_image_path);
			send_result = true;
		}

		if (send_result) {
			ge_dbg("Call ug_send_result_full() to send result.");
			ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_SUCCEEDED);
		}
	}

	if (ugd->b_destroy_me) {
		ge_dbg("Setting is appllied, destroy gallery UG.");
		ugd->b_destroy_me = false;
		/* Destroy self */
		if (!ugd->is_attach_panel) {
			ug_destroy_me(ugd->ug);
		}
	} else {
		ge_dbg("Cancel button tapped, back to thumbnails view.");
	}
}

static void _ge_ext_iv_layout_cb(ui_gadget_h ug, enum ug_mode mode, void* priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	GE_CHECK(ug);

	Evas_Object *base = (Evas_Object *)ug_get_layout(ug);
	if (!base) {
		ge_dbgE("ug_get_layout failed!");
		ug_destroy(ug);
		return;
	}

	evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	/* Disable effect to avoid BS caused by ui-gadget to
	     unset ug layout after deleting it */
	ug_disable_effect(ug);
	evas_object_show(base);
}

static void _ge_ext_iv_result_cb(ui_gadget_h ug, app_control_h result, void *priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	GE_CHECK(result);
	ge_ugdata *ugd = (ge_ugdata *)priv;
	char *path = NULL;
	char *status = NULL;

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SETAS) {
		/*If set wallpaper success, homescreen_path should not be null.
		And if setting wallpaper was canceled in IV, gallery-efl doesn't exit immediately*/
		app_control_get_extra_data(result, GE_BUNDLE_HOMESCREEN_PATH,
		                           &path);
		if (NULL == path)
			app_control_get_extra_data(result, GE_BUNDLE_LOCKSCREEN_PATH,
			                           &path);
		ge_dbg("SETAS_IMAGE_PATH");
		app_control_get_extra_data(result, "Result", &status);
		ugd->file_select_setas_mode = 0;
		if (strcmp(status, "Cancel")) {
			ugd->file_select_setas_mode = 1;
		}

		if (path) {
			ge_dbg(":%s", path);
			ugd->b_destroy_me = true;
			ugd->file_setas_image_path = path;
		} else {
			ugd->b_destroy_me = false;
		}
		/*If has got homescreen_path, setats_mode should not be callerid and
		crop wallpaper*/
		if (path == NULL &&
		        (ugd->file_select_setas_mode == GE_SETAS_T_CALLERID ||
		         ugd->file_select_setas_mode == GE_SETAS_T_CROP_WALLPAPER)) {
			app_control_get_extra_data(result, APP_CONTROL_DATA_SELECTED,
			                           &path);
			ge_dbg("CALLERID_CROP_IMAGE_PATH");
			if (path) {
				ge_dbg(":%s", path);
				ugd->b_destroy_me = true;
				ugd->file_setas_crop_image_path = path;
			} else {
				ugd->b_destroy_me = false;
			}
		}
	}

	char *error_state = NULL;
	app_control_get_extra_data(result, GE_IMAGEVIEWER_RETURN_ERROR,
	                           &error_state);
	if (error_state) {
		ge_dbg("error string : %s", error_state);

		if (!g_strcmp0(error_state, "not_supported_file_type")) {
			ugd->b_destroy_me = false;
			if (ugd->ug_path == NULL) {
				ge_dbgE("current item is NULL");
				GE_FREE(error_state);
				return;
			}
			app_control_h service;
			app_control_create(&service);
			GE_CHECK(service);
			app_control_set_operation(service, APP_CONTROL_OPERATION_VIEW);
			app_control_set_uri(service, ugd->ug_path);
			app_control_send_launch_request(service, NULL, NULL);
			app_control_destroy(service);
		}
		GE_FREE(error_state);
	}
}

static void _ge_ext_iv_destroy_cb(ui_gadget_h ug, void *priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	_ge_ext_destroy_me((ge_ugdata *)priv);
}

static void __ge_ext_iv_end_cb(ui_gadget_h ug, void *priv)
{
	ge_dbg("");
	GE_CHECK(priv);
	ge_ugdata *ugd = (ge_ugdata *)priv;

	if (ugd->file_select_setas_mode == 1) {
		_ge_grid_sel_one(ugd, ugd->file_select_setas_path);
	}

	if (ugd->b_hide_indicator) {
		_ge_ui_hide_indicator((ge_ugdata *)priv);
	}
}

static char **__ge_ext_get_select_index(ge_ugdata *ugd, int *size)
{
	GE_CHECK_NULL(ugd);
	GE_CHECK_NULL(ugd->selected_elist);
	char *index = NULL;
	int i = 0;
	char **media_index = NULL;
	int pos = 0;
	int sel_cnt = 0;
	ge_item *git = NULL;
	Eina_List *l = NULL;
	ge_dbg("Media count: %d", eina_list_count(ugd->selected_elist));

	sel_cnt = eina_list_count(ugd->selected_elist);
	ge_dbg("Item count: %d", sel_cnt);
	media_index = (char **)calloc(sel_cnt, sizeof(char *));
	GE_CHECK_NULL(media_index);

	EINA_LIST_FOREACH(ugd->selected_elist, l, git) {
		index = (char *)calloc(1, GE_IV_STR_LEN_MAX);
		if (git == NULL || index == NULL) {
			for (pos = 0; pos < i; ++pos) {
				GE_FREEIF(media_index[pos]);
			}

			GE_FREEIF(index);
			GE_FREE(media_index);
			return NULL;
		}
		ge_dbg("Sequence: %d", git->sequence - 1);
		snprintf(index, GE_IV_STR_LEN_MAX, "%d", git->sequence - 1);
		media_index[i++] = index;
		index = NULL;
		git = NULL;
	}


	if (size) {
		*size = sel_cnt;
	}

	return media_index;
}

/* Slideshow selected images */
static int __ge_ext_slideshow_selected(ge_ugdata *ugd, app_control_h service)
{
	GE_CHECK_VAL(service, -1);
	GE_CHECK_VAL(ugd, -1);
	char **media_index = NULL;
	int media_size = 0;
#define GE_SELECTED_FILES "Selected index"
#define GE_INDEX "Index"
#define GE_INDEX_VALUE "1"

	media_index = __ge_ext_get_select_index(ugd, &media_size);
	if (media_index == NULL) {
		ge_dbgE("Invalid select index!");
		return -1;
	}
	ge_dbg("Set selected medias, media_index[%p], size[%d]", media_index,
	       media_size);
	app_control_add_extra_data_array(service, GE_SELECTED_FILES,
	                                 (const char **)media_index, media_size);
	/*free space of the medias index*/
	int i = 0;
	for (i = 0; i < media_size; ++i) {
		ge_dbg("Set selected medias, media_index[%s]", media_index[i]);
		GE_FREEIF(media_index[i]);
	}
	GE_FREE(media_index);
	media_index = NULL;

	app_control_add_extra_data(service, GE_INDEX, GE_INDEX_VALUE);
	return 0;
}

static int __ge_ext_set_slideshow_data(ge_ugdata *ugd, char *file_url,
                                       app_control_h service)
{
	GE_CHECK_VAL(service, -1);
	GE_CHECK_VAL(file_url, -1);
	GE_CHECK_VAL(ugd, -1);

	app_control_add_extra_data(service, GE_PATH, file_url);
	app_control_add_extra_data(service, GE_VIEW_MODE, "SLIDESHOW");
	app_control_add_extra_data(service, "Sort By", "DateDesc");
	if (ugd->file_type_mode == GE_FILE_T_IMAGE) {
		app_control_add_extra_data(service, GE_MEDIA_TYPE, "Image");
	} else if (ugd->file_type_mode == GE_FILE_T_VIDEO) {
		app_control_add_extra_data(service, GE_MEDIA_TYPE, "Video");
	} else {
		app_control_add_extra_data(service, GE_MEDIA_TYPE, "All");
	}
	if (__ge_ext_slideshow_selected(ugd, service) != 0) {
		ge_dbgE("Create UG failed!");
		return -1;
	}

	switch (ugd->slideshow_viewby) {
	case GE_VIEW_BY_ALL:
	case GE_VIEW_BY_ALBUMS:
		if (ugd->slideshow_album_id == NULL) {
			ge_dbgE("Create UG failed!");
			return -1;
		}
		app_control_add_extra_data(service, "Album index", ugd->slideshow_album_id);
		if (!g_strcmp0(ugd->slideshow_album_id, GE_ALBUM_ALL_ID)) {
			app_control_add_extra_data(service, GE_VIEW_BY, "All");
		} else {
			app_control_add_extra_data(service, GE_VIEW_BY, "By Folder");
		}
		break;
	default:
		return -1;
	}
	return 0;
}

static int __ge_ext_set_setas_data(ge_ugdata *ugd, char *file_url,
                                   app_control_h service)
{
	GE_CHECK_VAL(service, -1);
	GE_CHECK_VAL(file_url, -1);
	GE_CHECK_VAL(ugd, -1);

	if (file_url) {
		app_control_add_extra_data(service, GE_PATH, file_url);
		GE_FREEIF(ugd->ug_path);
		ugd->ug_path = strdup(file_url);
	}
	app_control_add_extra_data(service, GE_VIEW_MODE, "SETAS");

	if (ugd->file_select_setas_mode == GE_SETAS_T_WALLPAPER) {
		app_control_add_extra_data(service, GE_SETAS_TYPE, "Wallpaper");
	} else if (ugd->file_select_setas_mode == GE_SETAS_T_LOCKPAPER) {
		app_control_add_extra_data(service, GE_SETAS_TYPE, "Lockscreen");
	} else if (ugd->file_select_setas_mode == GE_SETAS_T_WALLPAPER_LOCKPAPER) {
		app_control_add_extra_data(service, GE_SETAS_TYPE, "Wallpaper & Lockscreen");
	} else if (ugd->file_select_setas_mode == GE_SETAS_T_CROP_WALLPAPER) {
		app_control_add_extra_data(service, GE_SETAS_TYPE, "Wallpaper Crop");
		app_control_add_extra_data(service, "Fixed ratio", "TRUE");

		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;
		elm_win_screen_size_get((Evas_Object *)ug_get_window(), &x, &y, &w, &h);
		char *reso_str = (char *)calloc(1, GE_IV_STR_LEN_MAX);
		if (reso_str == NULL) {
			ge_dbgE("Calloc failed!");
			return -1;
		}
		snprintf(reso_str, GE_IV_STR_LEN_MAX, "%dx%d", w, h);
		ge_dbgW("Window Resolution: %dx%d, %s", w, h, reso_str);
		app_control_add_extra_data(service, "Resolution", reso_str);
		GE_FREE(reso_str);
	} else if (ugd->file_select_setas_mode == GE_SETAS_T_CALLERID) {
		app_control_add_extra_data(service, GE_SETAS_TYPE, "CallerID");
	}
	return 0;
}

int _ge_ext_load_iv_ug(ge_ugdata *ugd, char *file_url, char *album_index, int image_index)
{
	GE_CHECK_VAL(file_url, -1);
	GE_CHECK_VAL(ugd, -1);
	struct ug_cbs cbs;
	ui_gadget_h ug = NULL;
	app_control_h service = NULL;

	if (ugd->ug_called_by_me) {
		ge_dbgW("Already exits some UG called by me!");
		return -1;
	}

	memset(&cbs, 0x00, sizeof(struct ug_cbs));
	cbs.layout_cb = _ge_ext_iv_layout_cb;
	cbs.result_cb = _ge_ext_iv_result_cb;
	cbs.destroy_cb = _ge_ext_iv_destroy_cb;
	cbs.end_cb = __ge_ext_iv_end_cb;
	cbs.priv = ugd;

	app_control_create(&service);
	GE_CHECK_VAL(service, -1);

	if (ugd->file_select_mode == GE_FILE_SELECT_T_SLIDESHOW) {
		if (__ge_ext_set_slideshow_data(ugd, file_url, service) < 0) {
			ge_dbgE("Set slideshow data failed!");
			app_control_destroy(service);
			return -1;
		}
	} else {
		if (__ge_ext_set_setas_data(ugd, file_url, service) < 0) {
			ge_dbgE("Set setas data failed!");
			app_control_destroy(service);
			return -1;
		}
	}

	evas_object_smart_callback_call(ugd->naviframe, "gallery,freeze,resize", (void *)1);
	app_control_add_extra_data(service, "View By", "All");
	app_control_add_extra_data(service, "Album index", album_index);
	app_control_add_extra_data(service, "Path", file_url);
	app_control_add_extra_data(service, "Sort By", "Name");
	char image_index_str[12];
	eina_convert_itoa(image_index, image_index_str);
	app_control_add_extra_data(service, "Index", image_index_str);

	app_control_set_app_id(service, GE_IV_UG_NAME);
	ug = ug_create(ugd->ug, GE_IV_UG_NAME, UG_MODE_FULLVIEW, service, &cbs);
	ugd->ug_called_by_me = ug;

	app_control_destroy(service);
	if (!ug) {
		ge_dbgE("Create UG failed!");
		return -1;
	} else {
		ge_dbg("Create UG successully");
		return 0;
	}

}

int _ge_ext_load_iv_ug_for_help(ge_ugdata *ugd, const char *uri)
{
	GE_CHECK_VAL(ugd, -1);
	struct ug_cbs cbs;
	ui_gadget_h ug = NULL;
	app_control_h service = NULL;

	if (ugd->ug_called_by_me) {
		ge_dbgW("Already exits some UG called by me!");
		return -1;
	}

	memset(&cbs, 0x00, sizeof(struct ug_cbs));
	cbs.layout_cb = _ge_ext_iv_layout_cb;
	cbs.result_cb = _ge_ext_iv_result_cb;
	cbs.destroy_cb = _ge_ext_iv_destroy_cb;
	cbs.end_cb = __ge_ext_iv_end_cb;
	cbs.priv = ugd;

	app_control_create(&service);
	GE_CHECK_VAL(service, -1);

	/* Set "HELP" to "View Mode" */
	app_control_add_extra_data(service, GE_VIEW_MODE, "HELP");

	/* Set help uri to file path */
	app_control_add_extra_data(service, GE_PATH, uri);

	ug = ug_create(ugd->ug, GE_IV_UG_NAME, UG_MODE_FULLVIEW, service, &cbs);
	ugd->ug_called_by_me = ug;
	app_control_destroy(service);
	if (ug != NULL) {
		ge_dbg("Create UG successully");
		return 0;
	} else {
		ge_dbgE("Create UG failed!");
		return -1;
	}
}
