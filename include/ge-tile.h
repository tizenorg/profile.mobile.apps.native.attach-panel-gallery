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

#ifndef _GE_TILE_H_
#define _GE_TILE_H_

#include "ge-data.h"

#define GE_TILE_ICON "elm.swallow.icon"
#define GE_TILE_LABEL "label_bg"
#define GE_TILE_TYPE_ICON "elm.swallow.type_icon"

/* type for showing different icon in typeicon part in edc */
typedef enum _ge_tile_type {
	GE_TILE_TYPE_NONE,
	GE_TILE_TYPE_FACEBOOK, /* Facebook album */
	GE_TILE_TYPE_CLOUD, /* Cloud album */
	GE_TILE_TYPE_CAMERA, /* Camera album */
	GE_TILE_TYPE_DOWNLOAD, /* Download album */
	GE_TILE_TYPE_FOLDER, /* Folder album */
	GE_TILE_TYPE_PEOPLE, /* People album */
} ge_tile_type;

typedef enum
{
	GE_ICON_NORMAL,
	GE_ICON_CORRUPTED_FILE
}ge_icon_type;

typedef ge_icon_type (*bg_file_set_cb)(Evas_Object *bg,  void *data);

Evas_Object *_ge_tile_show_part_icon(Evas_Object *obj, const char *part,
				     int length, bg_file_set_cb func,
				     void *data);
int _ge_tile_update_item_size(ge_ugdata *ugd, Evas_Object *grid,
			      int rotate_mode, bool b_update);
Evas_Object *_ge_tile_show_part_type_icon(Evas_Object *obj, int type);

#endif //_GE_TILE_H_

