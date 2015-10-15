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

#ifndef __GE_UGDATA_H__
#define __GE_UGDATA_H__

#include <glib.h>
#include <Elementary.h>
#include <stdio.h>
#include <string.h>
#include <ui-gadget-module.h>
#include <Eina.h>
#include <app.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GE_MAX_BYTES_FOR_CHAR 3
#define GE_ALBUM_NAME_LEN_MAX (255 * GE_MAX_BYTES_FOR_CHAR + 1)

#define HALF_MODE_PORTRAIT_HEIGHT 362
#define FULL_MODE_PORTRAIT_HEIGHT 1036
#define LANDSCAPE_HEIGHT 528


/*Image viewer UG service parameters-End*/

typedef enum {
	GE_ALBUM_SELECT_T_NONE,
	GE_ALBUM_SELECT_T_ONE,
} ge_album_select_e;

typedef enum {
	GE_FILE_SELECT_T_NONE,
	GE_FILE_SELECT_T_ONE,
	GE_FILE_SELECT_T_MULTIPLE,
	GE_FILE_SELECT_T_SETAS,
	GE_FILE_SELECT_T_IMFT,
	GE_FILE_SELECT_T_SLIDESHOW,
} ge_file_select_e;

typedef enum {
	GE_SETAS_T_NONE,
	GE_SETAS_T_WALLPAPER,
	GE_SETAS_T_LOCKPAPER,
	GE_SETAS_T_WALLPAPER_LOCKPAPER,
	GE_SETAS_T_CROP_WALLPAPER,
	GE_SETAS_T_CALLERID,
} ge_file_select_setas_e;

typedef enum {
	GE_FILE_T_NONE,
	GE_FILE_T_IMAGE,
	GE_FILE_T_VIDEO,
	GE_FILE_T_ALL,
} ge_file_type_e;

typedef enum view_by_mode_e {
	GE_VIEW_ALBUMS,
	GE_VIEW_PEOPLE,
	GE_VIEW_THUMBS,
	GE_VIEW_THUMBS_EDIT,
} ge_view_mode;

typedef enum {
	GE_ROTATE_NONE,
	GE_ROTATE_PORTRAIT,
	GE_ROTATE_PORTRAIT_UPSIDEDOWN,
	GE_ROTATE_LANDSCAPE,
	GE_ROTATE_LANDSCAPE_UPSIDEDOWN,
} ge_rotate_mode;

typedef enum {
	GE_UPDATE_NONE,
	GE_UPDATE_NORMAL,
	GE_UPDATE_MMC_REMOVED,
	GE_UPDATE_MMC_ADDED,
	GE_UPDATE_VIEW_INOTIFY
} ge_update_mode;

typedef enum {
	GE_VIEW_BY_NONE,
	GE_VIEW_BY_ALL,
	GE_VIEW_BY_ALBUMS,
	GE_VIEW_BY_MAX,
} ge_viewby_mode;

typedef struct _ge_ugdata ge_ugdata;
typedef struct _ge_item ge_item;
typedef struct _ge_sel_album_t ge_sel_album_s;
typedef struct _ge_sel_item_t ge_sel_item_s;
typedef struct _ge_cluster ge_cluster;
typedef struct _ge_thumbs_t ge_thumbs_s;
typedef struct ge_db_noti_t ge_db_noti_s;

typedef int (*ge_albums_append_cb)(ge_ugdata *ugd, ge_cluster *album);
typedef int (*ge_rotate_view_cb)(ge_ugdata *ugd);

typedef struct {
	Eina_List *clist;
	int data_type;
	int local_cnt;
	int web_cnt;
	ge_albums_append_cb append_cb;
	bool b_updated;
	int all_medias_cnt;
	time_t last_mtime;
}ge_cluster_list;

struct _ge_sel_album_t {
	union {
		char *uuid;    /*album's uuid*/
		int id;    /*people album's id*/
	};
	Eina_List *elist;
};

struct _ge_sel_item_t {
	char *uuid;
	char *file_url;
	int store_type;
	int valid;
	int sequence;
};

struct _ge_item {
	ge_media_s* item;
	ge_ugdata* ugd;
	Elm_Object_Item *elm_item;
	bool checked;
	ge_cluster *album;
	int store_type;
	int sequence;
};

struct _ge_cluster {
	ge_album_s *cluster;
	ge_ugdata *ugd;
	int index;

	Elm_Object_Item *griditem;
	ge_item *cover;
	int cover_thumbs_cnt;
	int sel_cnt; /* Selected medias count of current album */
	bool select;
};

typedef enum {
	ATTACH_PANEL_NONE,
	ATTACH_PANEL_FULL_MODE,
	ATTACH_PANEL_HALF_MODE,
}_ge_attach_panel_display_mode;

struct _ge_ugdata {
	Evas_Object *win;
	Evas_Object *ly_parent;
	Evas_Object *ly_main;
	Evas_Object *bg;
	Elm_Theme *th;
	Evas_Object *naviframe;
	Elm_Object_Item *nf_it;
	Evas_Object *conform;
	Evas_Object *image_object;
//#ifdef _USE_HIDE_INDICATOR
	Elm_Win_Indicator_Opacity_Mode indi_o_mode;
	Elm_Win_Indicator_Mode indi_mode;
	bool overlap_mode;
//#endif
#ifdef _USE_ADD_ACCOUNT
	WebMediaHandle *db_handle;
#endif
	ge_view_mode view_mode;
	int rotate_mode;
	int tab_mode;
	Eina_List *rotate_cbs;

	Evas_Object *main_view_ly;
	Elm_Gengrid_Item_Class *album_gic;
	Evas_Object *albums_view;
	Evas_Object *albums_view_ly;
	Evas_Object *nocontents;
	Evas_Object *popup;
	Elm_Object_Item *done_it;
	Ecore_Timer *del_timer;
	int popup_mode;

	ge_cluster_list* cluster_list;
	ui_gadget_h ug_called_by_me;
	ui_gadget_h ug;
	char *ug_path;
	app_control_h service;		/*added for "ug_send_result"*/
	bool b_destroy_me;
	Ecore_Idler *album_idler;
	Ecore_Idler *sel_album_idler;
	Eina_List *selected_elist;
	Eina_Bool ck_state;
	int album_select_mode;
	int file_select_mode;
	int file_type_mode;
	int file_select_contact_id;
	int file_select_setas_mode;
	int file_select_setas_status;
	char *file_select_setas_path;
	char *slideshow_album_id;
	int slideshow_viewby;
	int max_count;
	char* file_setas_image_path;
	char* file_setas_crop_image_path;
	char albums_view_title[GE_ALBUM_NAME_LEN_MAX];

	Evas_Object *selinfo_ly;
	bool b_multifile; /* Multipe files selection */
	bool b_hide_indicator;
	ge_thumbs_s *thumbs_d;
	ge_db_noti_s *db_noti_d;
	_ge_attach_panel_display_mode attach_panel_display_mode;
	bool is_attach_panel;
	ge_cluster *album_item;
	Elm_Object_Item *selected_griditem;
	Ecore_Event_Handler *key_down_handler;
	long long int selsize;
	long long int limitsize;
};

struct _ge_thumbs_t {
	ge_ugdata *ugd;
	Evas_Object *view;
	Evas_Object *check;
	Evas_Object *nocontents;
	Evas_Object *layout;
	Evas_Object *split_view;
	Elm_Gengrid_Item_Class *gic;
	Elm_Object_Item *nf_it;
	Elm_Object_Item *done_it;

	Ecore_Idler *append_idler;
	/* It's thumbnails icon size, not grid item size */
	int icon_w;
	int icon_h;

	/* Data from parent album */
	char *album_uuid;
	int store_type;
	int tot_selected;
	/* Data from DB */
	int medias_cnt;
	Eina_List *medias_elist;
	bool b_multifile; /* Multiple file selection */
	bool b_mainview; /* First level view */
	ge_cluster *album;
	bool b_editmode;
	Evas_Object *gengrid;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GE_UGDATA_H__ */

