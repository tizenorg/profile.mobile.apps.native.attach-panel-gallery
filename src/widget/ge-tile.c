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
#include "ge-tile.h"
#include "ge-button.h"
#include "ge-icon.h"
#include "ge-util.h"
#ifdef _USE_ROTATE_BG_GE
#include "ge-rotate-bg.h"
#endif

/* Width and height of album item (4+331, 4+331) */
#define GE_TILE_ITEM_WIDTH 360
#define GE_TILE_ITEM_HEIGHT_HIDE 354
/* Indicator show, minus 20 pixels */
#define GE_TILE_ITEM_HEIGHT (354-20)
#define GE_TILE_ITEM_L_H 426
#define GE_TILE_2_PER_ROW 2
#define GE_TILE_3_PER_ROW 3
#define GE_TILE_ITEM_CNT 6
#define GE_TILE_ITEM_L_CNT 3
/* Items count of each column */
#define GE_TILE_ITEM_C_CNT 3
/* Size of album icon */
#define GE_TILE_W (GE_TILE_ITEM_WIDTH-8)
#define GE_TILE_H (GE_TILE_ITEM_HEIGHT_HIDE-8)

Evas_Object *__ge_tile_add_icon(Evas_Object *obj, bg_file_set_cb func, void *data)
{
	GE_CHECK_NULL(func);
	GE_CHECK_NULL(obj);
	Evas_Object *bg = NULL;

#ifdef _USE_ROTATE_BG_GE
	bg = _ge_rotate_bg_add(obj, true);
#else
	bg = elm_bg_add(obj);
#endif
	GE_CHECK_NULL(bg);
	double scale = elm_config_scale_get();

	if (data) {
		int wid = 0;
		int hei = 0;

		wid = (int)(GE_TILE_W * scale);
		hei = (int)(GE_TILE_H * scale);

#ifdef _USE_ROTATE_BG_GE
		_ge_rotate_bg_add_image(bg, wid, hei);
#else
		elm_bg_load_size_set(bg, wid, hei);
#endif

		func(bg, data);
	}

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, 0.5, 0.5);
	evas_object_show(bg);

	return bg;
}

Evas_Object *_ge_tile_show_part_icon(Evas_Object *obj, const char *part,
				     int length, bg_file_set_cb func,
				     void *data)
{
	GE_CHECK_NULL(part);
	GE_CHECK_NULL(strlen(part));
	GE_CHECK_NULL(obj);
	ge_dbg("part is %s", part);

	if (!g_strcmp0(part, GE_TILE_ICON)) {
		Evas_Object *bg = NULL;
		bg = __ge_tile_add_icon(obj, func, data);
		return bg;
	}
	return NULL;
}

Evas_Object *_ge_tile_show_part_type_icon(Evas_Object *obj, int type)
{
	GE_CHECK_NULL(obj);
	Evas_Object *icon = elm_icon_add(obj);
	GE_CHECK_NULL(icon);

	switch (type) {
	case GE_TILE_TYPE_CAMERA:
		GE_ICON_SET_FILE(icon, GE_ALBUM_ICON_CAMERA);
		break;
	case GE_TILE_TYPE_DOWNLOAD:
		GE_ICON_SET_FILE(icon, GE_ALBUM_ICON_DOWNLOAD);
		break;
	case GE_TILE_TYPE_FOLDER:
		GE_ICON_SET_FILE(icon, GE_ALBUM_ICON_FOLDER);
		break;
	default:
		ge_dbgE("Unknow type[%d]!", type);
	}

	evas_object_show(icon);
	return icon;
}

#ifdef _USE_SCROL_HORIZONRAL
/* Change icon size and gengrid alignment */
int _ge_tile_update_item_size(ge_ugdata *ugd, Evas_Object *grid,
			      int rotate_mode, bool b_update)
{
	GE_CHECK_VAL(grid, -1);
	GE_CHECK_VAL(ugd, -1);
	int win_w = 0;
	int win_h = 0;
	double scale = _ge_get_win_factor(ugd->win, ugd->b_hide_indicator,
					  &win_w, &win_h);
	int w = 0;
	int h = 0;
	int c = 0;
	int w_l = 0;
	int h_l = 0;
	int c_l = 0;
	ge_dbg("rotate_mode: %d, b_update: %d", rotate_mode, b_update);

	ge_dbg("scale: %f", scale);
	w = (int)(win_w / GE_TILE_2_PER_ROW);
	if (ugd->b_hide_indicator)
		h = (int)(GE_TILE_ITEM_HEIGHT_HIDE * scale);
	else
		h = (int)(GE_TILE_ITEM_HEIGHT * scale);
	w_l= (int)(win_h / GE_TILE_3_PER_ROW);
	h_l = (int)(GE_TILE_ITEM_L_H * scale);
	c = GE_TILE_ITEM_CNT;
	c_l = GE_TILE_ITEM_L_CNT;
	ge_dbg("Size: %dx%d L %dx%d C %dx%d", w, h, w_l, h_l, c, c_l);

	double align_x = 0.0f;
	double align_y = 0.0f;
	unsigned int count = 0;
	count = elm_gengrid_items_count(grid);
	elm_gengrid_align_get(grid, &align_x, &align_y);
	ge_dbg("count: %d, align_x: %f, align_y: %f", count, align_x, align_y);

	elm_gengrid_align_set(grid, 0.0, 0.0);
	if ((rotate_mode == GE_ROTATE_LANDSCAPE) ||
	    (rotate_mode == GE_ROTATE_LANDSCAPE_UPSIDEDOWN))
		elm_gengrid_item_size_set(grid, w_l, h_l);
	else
		elm_gengrid_item_size_set(grid, w, h);

	if (b_update)
		elm_gengrid_realized_items_update(grid);
	return 0;
}
#else
/* Change icon size and gengrid alignment */
int _ge_tile_update_item_size(ge_ugdata *ugd, Evas_Object *grid,
			      int rotate_mode, bool b_update)
{
	GE_CHECK_VAL(grid, -1);
	GE_CHECK_VAL(ugd, -1);
	int win_w = 0;
	int win_h = 0;
	double scale = _ge_get_win_factor(ugd->win, ugd->b_hide_indicator,
					  &win_w, &win_h);
	int w = 0;
	int h = 0;
	int c = 0;
	int w_l = 0;
	int h_l = 0;
	int c_l = 0;
	ge_dbg("rotate_mode: %d, b_update: %d", rotate_mode, b_update);

	ge_dbg("scale: %f", scale);
	w = (int)(win_w / GE_TILE_2_PER_ROW);
	h = (int)(300 * scale);
	w_l= (int)(win_h / GE_TILE_3_PER_ROW);
	h_l = w_l;
	h = w;
	c = GE_TILE_ITEM_CNT;
	c_l = GE_TILE_ITEM_L_CNT;
	ge_dbg("P %dx%d L %dx%d C %dx%d", w, h, w_l, h_l, c, c_l);

	double align_x = 0.0f;
	double align_y = 0.0f;
	unsigned int count = 0;
	count = elm_gengrid_items_count(grid);
	elm_gengrid_align_get(grid, &align_x, &align_y);
	ge_dbg("count: %d, align_x: %f, align_y: %f", count, align_x, align_y);

	elm_gengrid_align_set(grid, 0.0, 0.0);
	if ((rotate_mode == GE_ROTATE_LANDSCAPE) ||
	    (rotate_mode == GE_ROTATE_LANDSCAPE_UPSIDEDOWN))
		elm_gengrid_item_size_set(grid, w_l, h_l);
	else
		elm_gengrid_item_size_set(grid, w, h);

	if (b_update)
		elm_gengrid_realized_items_update(grid);
	return 0;
}
#endif

