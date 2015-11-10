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

#ifndef _GE_DATA_UTIL_H_
#define _GE_DATA_UTIL_H_

#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <media_content.h>
#include "ge-debug.h"
#include "ge-strings.h"
#include "ge-data-type.h"
#include "ge-ugdata.h"

#ifdef _cplusplus
extern "C" {
#endif

/*MEDIA_TYPE 0-image, 1-video, 2-sound, 3-music*/
#define GE_CONDITION_IMAGE_VIDEO "(MEDIA_TYPE=0 OR MEDIA_TYPE=1)"
#define GE_CONDITION_IMAGE       "(MEDIA_TYPE=0)"
#define GE_CONDITION_VIDEO       "(MEDIA_TYPE=1)"
/**< 0-not favourite, 1-favourite*/
#define GE_CONDITION_FAV_IMAGE_VIDEO "((MEDIA_TYPE=0 OR MEDIA_TYPE=1) AND MEDIA_FAVORITE=1)"

#define GE_CONDITION_FAVOR " AND MEDIA_FAVORITE=1"

#define GE_CONDITION_ORDER "MEDIA_TIMELINE, MEDIA_DISPLAY_NAME"
#define CONDITION_LENGTH 512
#define KEYWORD_LENGTH 128

#define GE_CAMERA_PATH_PHONE "/opt/usr/media/DCIM/Camera"
#define GE_CAMERA_PATH_MMC "/opt/storage/sdcard/DCIM/Camera"
#define GE_DOWNLOADS_PATH "/opt/usr/media/Downloads"

#define GE_ROOT_PATH_PHONE 	"/opt/usr/media"
#define GE_ROOT_PATH_MMC 	"/opt/storage/sdcard"
#define GE_DCIM "DCIM"

#define GE_ALBUM_ALL_ID "GALLERY_ALBUM_ALL_ALBUMS_ID"
/* String length of mtye item id */
#define GE_MTYPE_ITEN_ID_LEN_MAX 37

#define GE_BASE_WIN_WIDTH	720
#define GE_BASE_WIN_HEIGHT	1280
/* Indicator + naviframe title + naviframe bottom toobar */
#define GE_FIXED_HEIGHT_HIDE (111 + 98)
#define GE_FIXED_HEIGHT (60 + 111 + 98)

#define GE_GRID_ITEM_SIZE_W 202
#define GE_GRID_ITEM_SIZE_H 202
#define GE_GRID_ITEM_ZOOM_LEVEL_02 2
#define GE_GRID_ITEM_ZOOM_LEVEL_02_CNT 8

#define GE_ALBUM_COVER_THUMB_NUM 1

#define GE_SELINFO_TEXT_LEN_MAX 64
#define GE_ALBUM_DATE_LEN_MAX 256
#define GE_VIDEO_DURATION_MAX 256
#define GE_NOCONTENTS_LABEL_LEN_MAX 512
#define GE_FILE_PATH_LEN_MAX (4095 * GE_MAX_BYTES_FOR_CHAR + 1)
#define GE_DIR_PATH_LEN_MAX GE_FILE_PATH_LEN_MAX

#define GE_TIME_SECOND_MILLISECOND 1000 // 1s=1000ms
#define GE_TIME_USEC_PER_SEC 1000000L
#define GE_TIME_MINUTE_SECOND 60 // 1min=60s
#define GE_TIME_HOUR_MINUTE 60 // 1h=60min
#define GE_TIME_HOUR_SECOND (GE_TIME_HOUR_MINUTE * GE_TIME_MINUTE_SECOND) // 1h=(60x60)s

#define GE_FIRST_VIEW_START_POS 0
#define GE_GET_ONE_RECORD 1
#define GE_FIRST_VIEW_END_POS 47
#define GE_GET_UNTIL_LAST_RECORD 65536	/* Fixme: Do not use 65536. */
#define GE_GET_ALL_RECORDS -1

typedef struct _ge_transfer_data_t ge_transfer_data_s;

struct _ge_transfer_data_t {
	void **userdata;
	char *album_id;
	bool with_meta;
	bool b_mtype;
	int list_type;
};

typedef struct _ge_filter_t ge_filter_s;

struct _ge_filter_t {
	char cond[CONDITION_LENGTH];             /*set media type or favorite type, or other query statement*/
	media_content_collation_e collate_type;  /*collate type*/
	media_content_order_e sort_type;         /*sort type*/
	char sort_keyword[KEYWORD_LENGTH];       /*sort keyword*/
	int offset;                              /*offset*/
	int count;                               /*count*/
	bool with_meta;                          /*whether get image or video info*/
	int list_type;
};

typedef enum {
	GE_ALBUM_LIST_NONE,	/**<  */
	GE_ALBUM_LIST_FIRST, 	/**< Get first several albums except Camera and Downloads */
	GE_ALBUM_LIST_PATH, 	/**< Get a album via path */
	GE_ALBUM_LIST_ALL, 	/**< Get all albums except Camera and Downloads */
} ge_album_list_e;

typedef enum {
	GE_ALBUM_SNS_NONE,
	GE_ALBUM_SNS_PICASA,
	GE_ALBUM_SNS_FACEBOOK,
} ge_album_sns_type;

ge_item* _ge_data_util_calloc_geitem(void);
int _ge_data_util_free_geitem(ge_item* gitem);
int _ge_data_util_free_mtype_items(Eina_List **elist);
int _ge_data_util_free_cluster(ge_cluster* gcluster);
ge_cluster_list *_ge_data_util_calloc_clusters_list(void);
ge_cluster *_ge_data_util_calloc_cluster(void);
ge_cluster *_ge_data_util_new_cluster_all(ge_ugdata *ugd, int count);
ge_item *_ge_data_util_new_item_mitem(ge_media_s *mitem);
int _ge_data_util_free_item(ge_item *gitem);
ge_sel_item_s *_ge_data_util_new_sel_item(ge_item *gitem);
int _ge_data_util_free_sel_item(ge_sel_item_s *item);
ge_sel_album_s *_ge_data_util_new_sel_album(char *album_uuid);
int _ge_data_util_create_filter2(char *cond, char *keyword, int offset,
				 int count, filter_h *filter);
int _ge_data_util_create_filter(ge_filter_s *condition, filter_h *filter);
int _ge_data_util_destroy_filter(filter_h filter);
bool _ge_data_util_clone_media(media_info_h media, ge_media_s **pitem,
			       bool b_meta);
int _ge_data_util_get_geo_cond(char *cond, double longmin, double longmax,
			       double latmin, double latmax);
int _ge_data_util_get_type_cond(char *cond, int file_t);

#ifdef _cplusplus
}
#endif

#endif //_GE_DATA_UTIL_H_

