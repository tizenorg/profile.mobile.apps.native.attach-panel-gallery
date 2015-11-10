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
#include "ge-button.h"
#include "ge-util.h"
#include "ge-ui-util.h"

Evas_Object *_ge_but_create_but(Evas_Object *parent, Elm_Theme *theme,
                                const char *icon, const char *text,
                                const char *style, But_Smart_Cb cb_func,
                                const void *data)
{
	Evas_Object *btn = NULL;
	GE_CHECK_NULL(parent);

	btn = elm_button_add(parent);
	GE_CHECK_NULL(btn);

	if (theme) {
		elm_object_theme_set(btn, theme);
	}

	if (style) {
		elm_object_style_set(btn, style);
	}
	evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);

	if (icon) {
		ge_sdbg("Button icon: %s", icon);
		Evas_Object *ic = NULL;
		ic = elm_icon_add(btn);
		GE_CHECK_NULL(ic);
		GE_ICON_SET_FILE(ic, icon);
		elm_image_aspect_fixed_set(ic, EINA_TRUE);
		elm_image_fill_outside_set(ic, EINA_TRUE);
		evas_object_size_hint_aspect_set(ic,
		                                 EVAS_ASPECT_CONTROL_VERTICAL,
		                                 1, 1);
		evas_object_size_hint_align_set(ic, EVAS_HINT_FILL,
		                                EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(ic, EVAS_HINT_EXPAND,
		                                 EVAS_HINT_EXPAND);
		elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
		elm_object_content_set(btn, ic);
	}

	if (text) {
		ge_sdbg("Button text: %s", text);
		_ge_ui_set_translate_str(btn, text);
	}

	if (cb_func) {
		evas_object_smart_callback_add(btn, "clicked", cb_func, data);
	}

	return btn;
}

Evas_Object *_ge_but_create_image_but(Evas_Object *parent, const char *icon,
                                      const char *text, const char *style,
                                      Evas_Smart_Cb cb_func, void *data,
                                      Eina_Bool flag_propagate)
{
	GE_CHECK_NULL(parent);
	Evas_Object *btn;
	btn = elm_button_add(parent);
	GE_CHECK_NULL(btn);
	ge_ugdata *ugd = (ge_ugdata *)data;

	if (style) {
		elm_object_style_set(btn, style);
	}
	if (text) {
		ge_sdbg("Button text: %s", text);
		_ge_ui_set_translate_str(btn, text);
	}

	if (icon) {
		Evas_Object *ic = elm_image_add(parent);
		GE_CHECK_NULL(ic);
		ugd->image_object = ic;
		GE_ICON_SET_FILE(ic, icon);
		elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
		evas_object_show(ic);
		elm_object_content_set(btn, ic);
	}

	evas_object_propagate_events_set(btn, flag_propagate);
	if (cb_func) {
		evas_object_smart_callback_add(btn, "clicked", cb_func, (void *)data);
	}
	return btn;
}


