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

#ifndef __GALLERY_EFL_H__
#define __GALLERY_EFL_H__

#include "ge-data-util.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GALLERY_APP_ICON_DIR "/usr/share/icons/default/small/"
#define GE_UG_RES_DIR PREFIX"/res/"
#define GE_IMAGES_DIR GE_UG_RES_DIR"images/"PACKAGE"/images/"
#define GE_EDJ_DIR GE_UG_RES_DIR"edje/"PACKAGE"/"
#define GE_EDJ_FILE GE_EDJ_DIR"attach-panel-gallery.edj"
#ifdef _USE_CUSTOMIZED_GENGRID_STYLE
#define GE_GENGRID_STYLE_GALLERY "gallery_efl"
#endif
#define GE_GRP_GRIDVIEW "gallery_efl/gridview"
#define GE_GRP_ALBUMVIEW "gallery_efl/albumview"

#define GE_CBAR_STYLE_DEFAULT "gallery_efl/default"
#define GE_NAVIFRAME_STYLE_DEFAULT "gallery_efl/default"
#define GE_CHECKBOX_STYLE_GRID "gallery_efl/grid"

#define GE_BUNDLE_LAUNCH_TYPE 		"launch-type"
#define GE_BUNDLE_SELECTION_MODE_SINGLE "single"
#define GE_BUNDLE_SELECTION_MODE_MULTI	"multiple"

#define GE_BUNDLE_FILE_TYPE 		"file-type"
#define GE_BUNDLE_SETAS_TYPE 		"setas-type"

#define GE_LAUNCH_SELECT_ALBUM		"select-album"

#define GE_ALBUM_SELECT_RETURN_ID 	"album-id"
#define GE_ALBUM_SELECT_RETURN_PATH 	"folder-path"

#define GE_LAUNCH_SELECT_SLIDESHOW "select-slideshow"

#define GE_LAUNCH_SELECT_ONE 		"select-one"
#define GE_LAUNCH_SELECT_MULTIPLE 	"select-multiple"
#define GE_LAUNCH_SELECT_SETAS 		"select-setas"
#define GE_LAUNCH_SELECT_IMFT 		"select-imft"

#define GE_FILE_MAX_COUNT  "max-count"

#define GE_FILE_TYPE_IMAGE 		"image"
#define GE_FILE_TYPE_VIDEO 		"video"
#define GE_FILE_TYPE_ALL 		"all"

#define GE_SETAS_WALLPAPER 		"wallpaper"
#define GE_SETAS_LOCKSCREEN 		"lockscreen"
#define GE_SETAS_WALLPAPER_AND_LOCKSCREEN "wallpaper and lockscreen"
#define GE_SETAS_CROP_WALLPAPER 	"crop-wallpaper"
#define GE_SETAS_CALLERID 		"callerid"

#define GE_FILE_SELECT_RETURN_COUNT 	"count"
#define GE_FILE_SELECT_RETURN_PATH 	"path"

#define GE_SETAS_IMAGE_PATH "image-path"
#define GE_BUNDLE_HOMESCREEN_PATH  "homescreen_path"
#define GE_BUNDLE_LOCKSCREEN_PATH  "lockscreen_path"
#define GE_IMAGEVIEWER_RETURN_ERROR "Error"

/* MIME type */
#define GE_MIME_IMAGE_ALL	"image/*"
#define GE_MIME_VIDEO_ALL	"video/*"

#define GE_NAVIFRAME_POP_CB_KEY "ge_naviframe_pop_cb_key"

#define GE_IF_DEL_TIMER(timer) if (timer) { ecore_timer_del(timer); timer = NULL; }
#define GE_IF_DEL_IDLER(idler) if (idler) { ecore_idler_del(idler); idler = NULL; }
#define GE_IF_DEL_OBJ(obj) if (obj) { evas_object_del(obj); obj = NULL; }
#define GE_IF_DEL_JOB(job) if (job) { ecore_job_del(job); job = NULL; }

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GALLERY_EFL_H__ */

