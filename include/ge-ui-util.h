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

#ifndef _GE_UI_UTIL_H_
#define _GE_UI_UTIL_H_

#include "gallery-efl.h"
#include "ge-icon.h"
#include "ge-file-util.h"

#ifdef _cplusplus
extern "C" {
#endif


#define GE_BTN_NAVI_PRE "naviframe/back_btn/default"
#define GE_BTN_NAVI_TOOLBAR "naviframe/toolbar/default"
#define GE_NAVIFRAME_BTN1 "toolbar_button1"
#define GE_NAVIFRAME_TITLE_RIGHT_BTN "title_right_btn"
#define GE_NAVIFRAME_TITLE_LEFT_BTN "title_left_btn"
#define GE_BTN_POPUP "popup"
#define GE_NAVIFRAME_PREB_BTN "prev_btn"
#define GE_BUTTON_STYLE_NAVI_TITLE_ICON "naviframe/title_icon"

#define _EDJ(o)         elm_layout_edje_get(o)

#define _GE_GET_ICON(_path) \
	((1 == ge_file_exists(_path) && (ge_file_size(_path) > 0)) ? _path : GE_ICON_NO_THUMBNAIL)
#define _GE_GET_APP_ICON(_path) \
	((1 == ge_file_exists(_path) && (ge_file_size(_path) > 0)) ? _path : GE_DEFAULT_APP_ICON)

typedef enum {
	GE_POPUP_NOBUT,
	GE_POPUP_ONEBUT,
	GE_POPUP_TWOBUT,
	GE_POPUP_EXIT,
} ge_popup_mode_e;

Evas_Object* ge_ui_create_naviframe(ge_ugdata *ugd, Evas_Object *parent);
Evas_Object *ge_ui_create_main_ly(Evas_Object *parent);
Evas_Object* ge_ui_load_edj(Evas_Object *parent, const char *file, const char *group);
Evas_Object* ge_ui_create_nocontents(ge_ugdata* ugd);
int ge_ui_lock_albums(ge_cluster* album_item);
Evas_Object* ge_ui_create_popup(ge_ugdata* ugd, ge_popup_mode_e mode,
				char* desc);
int _ge_ui_create_notiinfo(const char *text);
char* _ge_ui_get_i18n_album_name(ge_album_s *cluster);
int _ge_ui_reset_scroller_pos(Evas_Object *obj);
int _ge_ui_del_scroller_pos(Evas_Object *obj);
int _ge_ui_save_scroller_pos(Evas_Object *obj);
int _ge_ui_restore_scroller_pos(Evas_Object *obj);
int _ge_ui_set_translate_str(Evas_Object *obj, const char *str);
int _ge_ui_set_translate_part_str(Evas_Object *obj, const char *part,
				  const char *str);
int _ge_ui_set_translatable_item(Elm_Object_Item *nf_it, const char *str);
int _ge_ui_update_label_text(Elm_Object_Item *nf_it, int sel_cnt,
			     const char *text);
int _ge_ui_get_indicator_state(ge_ugdata *ugd);
int _ge_ui_hide_indicator(ge_ugdata *ugd);
#ifdef _USE_HIDE_INDICATOR
int _ge_ui_reset_indicator(ge_ugdata *ugd);
#endif
Evas_Object *_ge_ui_add_toolbar(Evas_Object *parent);
Elm_Object_Item *_ge_ui_append_item(Evas_Object *obj, const char *icon,
				    const char *label, Evas_Smart_Cb func,
				    const void *data);
int _ge_ui_disable_item(Elm_Object_Item *it, Eina_Bool b_disabled);

#ifdef _cplusplus
}
#endif

#endif //_GE_UI_UTIL_H_
