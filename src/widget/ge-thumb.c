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

#include "ge-debug.h"
#include "ge-util.h"
#include "ge-strings.h"
#include "ge-icon.h"
#include "ge-thumb.h"
#include "ge-util.h"
#ifdef _USE_ROTATE_BG_GE
#include "ge-rotate-bg.h"
#include "ge-exif.h"
#endif

#define GE_PHOTOFRAME_STYLE_VIDEO "default_layout_video"
#define GE_PHOTOFRAME_STYLE_DEFAULT "default_layout"

#define GE_CLASS_GENGRID "gengrid"
#define GE_GRP_PHOTOFRAME "photoframe"
#define GE_GRID_TEXT "elm.text"

Evas_Object *_ge_thumb_show_part_icon_image(Evas_Object *obj, char *path,
                unsigned int orient, bool b_favor,
                int item_w, int item_h)
{
	GE_CHECK_NULL(obj);

	Evas_Object *layout = elm_layout_add(obj);
	GE_CHECK_NULL(layout);

#ifdef _USE_ROTATE_BG_GE
	Evas_Object *bg = _ge_rotate_bg_add(layout, true);
#else
	Evas_Object *bg = elm_bg_add(layout);
#endif
	if (bg == NULL) {
		evas_object_del(layout);
		return NULL;
	}

#ifdef _USE_ROTATE_BG_GE
	_ge_rotate_bg_set_file(bg, path, item_w, item_h);
	_ge_rotate_bg_rotate_image(bg, orient);
#else
	elm_bg_file_set(bg, path, NULL);
	elm_bg_load_size_set(bg, item_w, item_h);
	evas_object_size_hint_max_set(bg, item_w, item_h);
	evas_object_size_hint_aspect_set(bg, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
#endif

	elm_layout_theme_set(layout, GE_CLASS_GENGRID, GE_GRP_PHOTOFRAME,
	                     GE_PHOTOFRAME_STYLE_DEFAULT);
	elm_object_part_content_set(layout, "elm.swallow.icon", bg);

	evas_object_show(layout);
	return layout;
}

Evas_Object *_ge_thumb_show_part_icon_video(Evas_Object *obj, char *path,
                unsigned int v_dur, int bk_len,
                bool b_favor, int item_w, int item_h)
{
	GE_CHECK_NULL(obj);

	Evas_Object *layout = elm_layout_add(obj);
	GE_CHECK_NULL(layout);

#ifdef _USE_ROTATE_BG_GE
	Evas_Object *bg = _ge_rotate_bg_add(layout, true);
#else
	Evas_Object *bg = elm_bg_add(layout);
#endif
	if (bg == NULL) {
		evas_object_del(layout);
		return NULL;
	}

#ifdef _USE_ROTATE_BG_GE
	_ge_rotate_bg_set_file(bg, path, item_w, item_h);
	_ge_rotate_bg_rotate_image(bg, GE_ORIENTATION_ROT_0);
#else
	elm_bg_file_set(bg, path, NULL);
	elm_bg_load_size_set(bg, item_w, item_h);
	evas_object_size_hint_max_set(bg, item_w, item_h);
	evas_object_size_hint_aspect_set(bg, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
#endif

	elm_layout_theme_set(layout, GE_CLASS_GENGRID, GE_GRP_PHOTOFRAME,
	                     GE_PHOTOFRAME_STYLE_VIDEO);
	elm_object_part_content_set(layout, "elm.swallow.icon", bg);

	if (bk_len) {
		Evas_Object *icon = elm_icon_add(layout);
		GE_ICON_SET_FILE(icon, GE_THUMB_BOOKMARK);
		evas_object_show(icon);
		elm_object_part_content_set(layout, "elm.swallow.bookmarkicon",
		                            icon);
	}

	char *dur_str = _ge_get_duration_string(v_dur);
	GE_CHECK_NULL(dur_str);
	elm_object_part_text_set(layout, GE_GRID_TEXT, dur_str);
	GE_FREE(dur_str);

	evas_object_show(layout);
	return layout;
}

/* Size of thumbnails in gridview */
#define GE_GRID_ITEM_SIZE_W_ZOOM_03 240
#define GE_GRID_ITEM_SIZE_H_ZOOM_03_HIDE 178
/* Indicator show, only show 5 items per column, plus 23 pixels */
#define GE_GRID_ITEM_SIZE_H_ZOOM_03 (178+23)

#define GE_GRID_ITEM_SIZE_W_ZOOM_02 360
#define GE_GRID_ITEM_SIZE_H_ZOOM_02_HIDE 267
/* Indicator show, minus 15 pixels */
#define GE_GRID_ITEM_SIZE_H_ZOOM_02 (267-15)

#define GE_GRID_ITEM_SIZE_L_W GE_GRID_ITEM_SIZE_W_ZOOM_02
#define GE_GRID_ITEM_SIZE_L_H_HIDE GE_GRID_ITEM_SIZE_H_ZOOM_02_HIDE
#define GE_GRID_ITEM_SIZE_L_H GE_GRID_ITEM_SIZE_H_ZOOM_02

#define GE_GRID_4_PER_ROW 4
#define GE_GRID_3_PER_ROW 3
#define GE_GRID_2_PER_ROW 2

#define GE_GRID_ITEM_L_CNT 6
#define GE_GRID_ITEM_S_CNT 5
#define GE_GRID_ITEM_ZOOM_02_CNT 8
#define GE_GRID_ITEM_ZOOM_03_CNT_HIDE 18
#define GE_GRID_ITEM_ZOOM_03_CNT 15

#ifdef _USE_SCROL_HORIZONRAL
/* Change icon size and gengrid alignment */
int _ge_thumb_set_size(ge_ugdata *ugd, Evas_Object *view, int rotate_mode,
                       int *size_w, int *size_h)
{
	GE_CHECK_VAL(view, -1);
	GE_CHECK_VAL(ugd, -1);
	int win_w = 0;
	int win_h = 0;
	double scale = _ge_get_win_factor(ugd->win, ugd->b_hide_indicator,
	                                  &win_w, &win_h);
	int _w = 0;
	int _h = 0;
	int _w_l = 0;
	int _h_l = 0;
	int align_c = 0;
	double align_x = 0.0f;
	double align_y = 0.0f;
	int count = elm_gengrid_items_count(view);
	elm_gengrid_align_get(view, &align_x, &align_y);
	ge_dbg("rotate_mode: %d, count: %d", rotate_mode, count);

	_w_l = (int)(GE_GRID_ITEM_SIZE_L_W * scale);
	if (ugd->b_hide_indicator) {
		_h_l = (int)(GE_GRID_ITEM_SIZE_L_H_HIDE * scale);
	} else {
		_h_l = (int)(GE_GRID_ITEM_SIZE_L_H * scale);
	}

	if (count <= GE_GRID_ITEM_ZOOM_LEVEL_02_CNT) {
		_w = (int)(win_w / GE_GRID_2_PER_ROW);
		if (ugd->b_hide_indicator) {
			_h = (int)(GE_GRID_ITEM_SIZE_H_ZOOM_02_HIDE * scale);
		} else {
			_h = (int)(GE_GRID_ITEM_SIZE_H_ZOOM_02 * scale);
		}
		align_c = GE_GRID_ITEM_ZOOM_02_CNT;
	} else {
		_w = (int)(win_w / GE_GRID_3_PER_ROW);
		if (ugd->b_hide_indicator) {
			_h = (int)(GE_GRID_ITEM_SIZE_H_ZOOM_03_HIDE * scale);
			align_c = GE_GRID_ITEM_ZOOM_03_CNT_HIDE;
		} else {
			_h = (int)(GE_GRID_ITEM_SIZE_H_ZOOM_03 * scale);
			align_c = GE_GRID_ITEM_ZOOM_03_CNT;
		}
	}

	if ((rotate_mode == GE_ROTATE_LANDSCAPE) ||
	        (rotate_mode == GE_ROTATE_LANDSCAPE_UPSIDEDOWN)) {
		align_c = GE_GRID_ITEM_L_CNT;
		elm_gengrid_item_size_set(view, _w_l, _h_l);
		if (size_w) {
			*size_w = _w_l - 4;
		}
		if (size_h) {
			*size_h = _h_l - 4;
		}
	} else {
		elm_gengrid_item_size_set(view, _w, _h);
		if (size_w) {
			*size_w = _w - 4;
		}
		if (size_h) {
			*size_h = _h - 4;
		}
	}
	elm_gengrid_align_set(view, 0.0, 0.0);
	if (size_w && size_h)
		ge_dbg("P: %dx%d, size_w: %d,size_h: %d", _w, _h, *size_w,
		       *size_h);
	return 0;
}
#else
/* Change icon size and gengrid alignment */
int _ge_thumb_set_size(ge_ugdata *ugd, Evas_Object *view, int rotate_mode,
                       int *size_w, int *size_h)
{
	GE_CHECK_VAL(view, -1);
	GE_CHECK_VAL(ugd, -1);
	int win_w = 0;
	int win_h = 0;
	double scale = _ge_get_win_factor(ugd->win, ugd->b_hide_indicator,
	                                  &win_w, &win_h);
	int _w = 0;
	int _h = 0;
	int _w_l = 0;
	int _h_l = 0;
	double align_x = 0.0f;
	double align_y = 0.0f;
	int count = elm_gengrid_items_count(view);
	elm_gengrid_align_get(view, &align_x, &align_y);
	ge_dbg("rotate_mode: %d, count: %d", rotate_mode, count);

	_w = (int)(win_w / GE_GRID_3_PER_ROW);
	_h = (int)(GE_GRID_ITEM_SIZE_H_ZOOM_02 * scale);
	_w_l = (int)(win_h / GE_GRID_ITEM_S_CNT);
	_h_l = (int)(356 * scale);

	if ((rotate_mode == GE_ROTATE_LANDSCAPE) ||
	        (rotate_mode == GE_ROTATE_LANDSCAPE_UPSIDEDOWN)) {
		_h_l = _w_l;
		elm_gengrid_item_size_set(view, _w_l, _h_l);
		if (size_w) {
			*size_w = _w_l - 4;
		}
		if (size_h) {
			*size_h = _h_l - 4;
		}
	} else {
		_h = _w;
		elm_gengrid_item_size_set(view, _w, _h);
		if (size_w) {
			*size_w = _w - 4;
		}
		if (size_h) {
			*size_h = _h - 4;
		}
	}
	/*if (count <= align_c)
		elm_gengrid_align_set(view, 0.5, 0.5);
	else*/
		elm_gengrid_align_set(view, 0.0, 0.0);
	if (size_w && size_h)
		ge_dbg("P: %dx%d, size_w: %d,size_h: %d", _w, _h, *size_w,
		       *size_h);
	return 0;
}
#endif
Evas_Object *_ge_thumb_show_mode(Evas_Object *obj, ge_cm_mode_e mode)
{
	Evas_Object *icon = NULL;
	const char *file = NULL;

	switch (mode) {
	case GE_CM_MODE_PANORAMA:
		file = GE_ICON_PANORAMA;
		break;
	case GE_CM_MODE_SOUND:
		file = GE_ICON_SOUND_SHOT;
		break;
	case GE_CM_MODE_ANIMATE:
		file = GE_ALBUM_ICON_FOLDER;
		break;
	case GE_CM_MODE_BEST:
		file = GE_ALBUM_ICON_DOWNLOAD;
		break;
	default:
		break;
	}

	if (file) {
		GE_CHECK_NULL(obj);
		icon = elm_icon_add(obj);
		GE_CHECK_NULL(icon);
		GE_ICON_SET_FILE(icon, file);
		evas_object_show(icon);
	}
	return icon;
}

