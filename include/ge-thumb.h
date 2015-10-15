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

#ifndef _GE_THUMB_H_
#define _GE_THUMB_H_

typedef enum _ge_cm_mode_t {
	GE_CM_MODE_NORMAL		= 0,
	GE_CM_MODE_PANORAMA		= 1,
	GE_CM_MODE_SOUND		= 2,
	GE_CM_MODE_ANIMATE		= 3,
	GE_CM_MODE_BEST			= 4,
	GE_CM_MODE_FB			= 5,
	GE_CM_MODE_VOICE_REC	= 10,
	GE_CM_MODE_MAX,
} ge_cm_mode_e;

Evas_Object *_ge_thumb_show_part_icon_image(Evas_Object *obj, char *path,
					    unsigned int orient, bool b_favor,
					    int item_w, int item_h);

Evas_Object *_ge_thumb_show_part_icon_video(Evas_Object *obj, char *path,
					    unsigned int v_dur, int bk_len,
					    bool b_favor, int item_w, int item_h);
int _ge_thumb_set_size(ge_ugdata *ugd, Evas_Object *view, int rotate_mode,
		       int *size_w, int *size_h);
Evas_Object *_ge_thumb_show_mode(Evas_Object *obj, ge_cm_mode_e mode);

#endif //_GE_THUMB_H_

