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

#include <notification.h>
#include "ge-ui-util.h"
#include "ge-util.h"
#include "ge-button.h"
#include "ge-gridview.h"
#include "ge-albums.h"
#include "ge-data.h"
#include "ge-icon.h"
#include <efl_extension.h>

#define GE_NOBUT_EXIT_POPUP_HIDE_TIME_DELAY 3.0

static void _ge_ui_response_cb(void *data, Evas_Object *obj, void *ei)
{
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	ge_dbg("popup mode: %d", ugd->popup_mode);

	evas_object_del(obj);
	ugd->popup = NULL;
}

static Eina_Bool __ge_popup_timeout_cb(void *data)
{
	GE_CHECK_FALSE(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	if (ugd->del_timer) {
		ecore_timer_del(ugd->del_timer);
		ugd->del_timer = NULL;
	}

	/* Used for adding shortcut failed */
	if (ugd->popup_mode == GE_POPUP_EXIT) {
		ge_dbgW("Terminate me!");
		if (!ugd->is_attach_panel) {
			ug_destroy_me(ugd->ug);
			ugd->ug = NULL;
		}
	}

	GE_IF_DEL_OBJ(ugd->popup);
	return ECORE_CALLBACK_CANCEL;
}

static int __ge_popup_add_timer(void *data, double to_inc)
{
	GE_CHECK_VAL(data, -1);
	ge_ugdata *ugd = (ge_ugdata *)data;
	if (ugd->del_timer) {
		ecore_timer_del(ugd->del_timer);
		ugd->del_timer = NULL;
	}

	ugd->del_timer = ecore_timer_add(to_inc, __ge_popup_timeout_cb, data);
	return 0;
}

Evas_Object* ge_ui_load_edj(Evas_Object *parent, const char *file, const char *group)
{
	GE_CHECK_NULL(parent);
	GE_CHECK_NULL(file);
	GE_CHECK_NULL(group);
	Evas_Object *eo;
	int r;

	eo = elm_layout_add(parent);
	if (eo) {
		r = elm_layout_file_set(eo, file, group);
		if (!r) {
			ge_dbg("failed to load layout");
			evas_object_del(eo);
			return NULL;
		}

		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(eo, EVAS_HINT_FILL, EVAS_HINT_FILL);
	}

	return eo;
}

Evas_Object* ge_ui_create_nocontents(ge_ugdata* ugd)
{
	GE_CHECK_NULL(ugd);
	Evas_Object *layout = NULL;
	char label_str[GE_NOCONTENTS_LABEL_LEN_MAX] = {0,};

	if (_ge_get_view_mode(ugd) == GE_VIEW_THUMBS ||
	        _ge_get_view_mode(ugd) == GE_VIEW_THUMBS_EDIT) {
		if (ugd->file_type_mode) {
			if (ugd->file_type_mode == GE_FILE_T_IMAGE)
				snprintf(label_str, sizeof(label_str),
				         "%s", (char*)GE_STR_NO_IMAGES);
			else if (ugd->file_type_mode == GE_FILE_T_VIDEO)
				snprintf(label_str, sizeof(label_str),
				         "%s", (char*)GE_STR_NO_VIDEOS);
			else
				snprintf(label_str, sizeof(label_str),
				         "%s", (char*)GE_STR_NO_ITEMS);
		} else {
			snprintf(label_str, sizeof(label_str), "%s",
			         (char*)GE_STR_NO_ITEMS);
		}
	} else if (_ge_get_view_mode(ugd) == GE_VIEW_ALBUMS) {
		snprintf(label_str, sizeof(label_str), "%s",
		         (char*)GE_STR_NO_ALBUMS);
	} else {
		ge_dbgE("view mode is error.");
	}

	ge_dbg("\nNocontents label: %s", label_str);
	/* Full nocontents view layout */
	layout = elm_layout_add(ugd->naviframe);
	GE_CHECK_NULL(layout);
	elm_layout_theme_set(layout, "layout", "nocontents", "text");
	elm_object_part_text_set(layout, "elm.text", label_str);

	return layout;
}

Evas_Object *ge_ui_create_main_ly(Evas_Object *parent)
{
	GE_CHECK_NULL(parent);
	Evas_Object *layout = NULL;

	layout = elm_layout_add(parent);
	GE_CHECK_NULL(layout);

	/* Apply the layout style */
	const char *profile = elm_config_profile_get();
	ge_dbg("profile: %s", profile);
	if (!g_strcmp0(profile, "mobile"))	{
		elm_layout_theme_set(layout, "layout", "application",
		                     "default");
	} else if (!g_strcmp0(profile, "extension")) {
		elm_layout_theme_set(layout, "layout", "application",
		                     "noindicator");
	} else {
		elm_layout_theme_set(layout, "layout", "application",
		                     "default");
	}

	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_show(layout);
	return layout;
}

Evas_Object* ge_ui_create_naviframe(ge_ugdata *ugd, Evas_Object *parent)
{
	GE_CHECK_NULL(ugd);
	GE_CHECK_NULL(parent);
	Evas_Object *nf = NULL;

	nf = elm_naviframe_add(parent);
	GE_CHECK_NULL(nf);
	/* Disable Naviframe Back Button Auto creation function */
	elm_naviframe_prev_btn_auto_pushed_set(nf, EINA_FALSE);
	if (ugd->th) {
		elm_object_theme_set(nf, ugd->th);
	}
	elm_object_part_content_set(parent, "elm.swallow.content", nf);

	evas_object_show(nf);

	return nf;
}

Evas_Object* ge_ui_create_popup(ge_ugdata* ugd, ge_popup_mode_e mode,
                                char* desc)
{
	ge_dbg("");
	GE_CHECK_NULL(ugd);
	GE_CHECK_NULL(desc);

	if (ugd->popup) {
		ge_dbg("The existed popup is deleted");
		evas_object_del(ugd->popup);
		ugd->popup = NULL;
	}

	Evas_Object *popup = NULL;

	popup = elm_popup_add(ugd->ly_main);
	GE_CHECK_NULL(popup);

	/*Delete the Popup if the Popup has a BACK event.*/
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb,
	                               NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	elm_object_text_set(popup, desc);

	switch (mode) {
	case GE_POPUP_NOBUT:
	case GE_POPUP_EXIT:
		__ge_popup_add_timer(ugd, GE_NOBUT_EXIT_POPUP_HIDE_TIME_DELAY);
		break;
	case GE_POPUP_ONEBUT: {
		Evas_Object *btn = NULL;
		btn = _ge_but_create_but(popup, ugd->th, NULL, GE_STR_ID_OK,
		                         GE_BTN_POPUP, _ge_ui_response_cb, ugd);
		elm_object_part_content_set(popup, "button1", btn);
		break;
	}
	case GE_POPUP_TWOBUT: {
		Evas_Object *btn1 = NULL;
		btn1 = _ge_but_create_but(popup, ugd->th, NULL, GE_STR_ID_OK,
		                          GE_BTN_POPUP, _ge_ui_response_cb,
		                          ugd);
		elm_object_part_content_set(popup, "button1", btn1);
		Evas_Object *btn2 = NULL;
		btn2 = _ge_but_create_but(popup, ugd->th, NULL, GE_STR_ID_CANCEL,
		                          GE_BTN_POPUP, _ge_ui_response_cb,
		                          ugd);
		elm_object_part_content_set(popup, "button2", btn2);
		break;
	}
	default:
		ge_dbgE("mode is not supported...");
		break;
	}

	evas_object_show(popup);

	ugd->popup = popup;
	ugd->popup_mode = mode;

	return popup;
}

int _ge_ui_create_notiinfo(const char *text)
{
	GE_CHECK_VAL(text, -1);
	int ret = notification_status_message_post(text);
	if (ret != 0) {
		ge_sdbgE("status_message_post()... [0x%x]!", ret);
	}
	return ret;
}

/* *
* In case of system folder, the displayed name should be translated into system language
*
* @param name
*    check album display name for getting proper translation
*
* @return
*    the translated album display name
*/
char *_ge_ui_get_i18n_album_name(ge_album_s *cluster)
{
	GE_CHECK_NULL(cluster);
	char *i18n_name = NULL;

	if (_ge_data_is_camera_album(cluster)) {
		/* system folder name: Camera */
		i18n_name = GE_STR_CAMERA;
	} else if (_ge_data_is_default_album(GE_ALBUM_DOWNLOADS_NAME, cluster)) {
		/* system folder name: Downloads */
		i18n_name = GE_STR_DOWNLOADS;
	} else if (cluster->type == GE_ALL) {
		/* Update data in memory */
		GE_FREEIF(cluster->display_name);
		cluster->display_name = strdup(GE_ALBUM_ALL_NAME);
		i18n_name = cluster->display_name;
	} else if (cluster->type == GE_PHONE || cluster->type == GE_MMC) {
		if (_ge_data_is_root_path(cluster->path)) {
			/* check root case */
			i18n_name = GE_ALBUM_ROOT_NAME;
		} else {
			/* if the folder is not a system folder, return itself */
			i18n_name = cluster->display_name;
		}
	} else {
		/* if the folder is not a system folder, return itself */
		i18n_name = cluster->display_name;
	}

	if (i18n_name == NULL || strlen(i18n_name) <= 0) {
		i18n_name = GE_ALBUM_ROOT_NAME;
	}

	return i18n_name;
}

int _ge_ui_reset_scroller_pos(Evas_Object *obj)
{
	GE_CHECK_VAL(obj, -1);
	evas_object_data_set(obj, "prev_scroller_x", (void *)0);
	evas_object_data_set(obj, "prev_scroller_y", (void *)0);
	evas_object_data_set(obj, "prev_scroller_w", (void *)0);
	evas_object_data_set(obj, "prev_scroller_h", (void *)0);
	return 0;
}

int _ge_ui_del_scroller_pos(Evas_Object *obj)
{
	GE_CHECK_VAL(obj, -1);
	evas_object_data_del(obj, "prev_scroller_x");
	evas_object_data_del(obj, "prev_scroller_y");
	evas_object_data_del(obj, "prev_scroller_w");
	evas_object_data_del(obj, "prev_scroller_h");
	return 0;
}

int _ge_ui_save_scroller_pos(Evas_Object *obj)
{
	GE_CHECK_VAL(obj, -1);
	Evas_Coord x = 0;
	Evas_Coord y = 0;
	Evas_Coord w = 0;
	Evas_Coord h = 0;

	elm_scroller_region_get(obj, &x, &y, &w, &h);
	ge_dbg("(%dx%d), (%dx%d)", x, y, w, h);
	evas_object_data_set(obj, "prev_scroller_x", (void *)x);
	evas_object_data_set(obj, "prev_scroller_y", (void *)y);
	evas_object_data_set(obj, "prev_scroller_w", (void *)w);
	evas_object_data_set(obj, "prev_scroller_h", (void *)h);
	return 0;
}

int _ge_ui_restore_scroller_pos(Evas_Object *obj)
{
	GE_CHECK_VAL(obj, -1);
	Evas_Coord x = 0;
	Evas_Coord y = 0;
	Evas_Coord w = 0;
	Evas_Coord h = 0;

	x = (Evas_Coord)evas_object_data_get(obj, "prev_scroller_x");
	y = (Evas_Coord)evas_object_data_get(obj, "prev_scroller_y");
	w = (Evas_Coord)evas_object_data_get(obj, "prev_scroller_w");
	h = (Evas_Coord)evas_object_data_get(obj, "prev_scroller_h");
	ge_dbg("(%dx%d), (%dx%d)", x, y, w, h);
	if (w > 0 && h > 0) {
		elm_scroller_region_show(obj, x, y, w, h);
	}
	return 0;
}

int _ge_ui_set_translate_str(Evas_Object *obj, const char *str)
{
	GE_CHECK_VAL(str, -1);
	GE_CHECK_VAL(obj, -1);
	char *domain = GE_STR_DOMAIN_LOCAL;
	elm_object_domain_translatable_text_set(obj, domain, str);
	return 0;
}

int _ge_ui_set_translate_part_str(Evas_Object *obj, const char *part,
                                  const char *str)
{
	GE_CHECK_VAL(str, -1);
	GE_CHECK_VAL(part, -1);
	GE_CHECK_VAL(obj, -1);

	char *domain = GE_STR_DOMAIN_LOCAL;

	elm_object_domain_translatable_part_text_set(obj, part, domain, str);

	return 0;
}

int _ge_ui_set_translatable_item(Elm_Object_Item *nf_it, const char *str)
{
	GE_CHECK_VAL(str, -1);
	GE_CHECK_VAL(nf_it, -1);
	char *domain = GE_STR_DOMAIN_LOCAL;
	elm_object_item_domain_text_translatable_set(nf_it, domain, EINA_TRUE);
	return 0;
}

/* Update the label text for selected item showed in naviframe title  */
int _ge_ui_update_label_text(Elm_Object_Item *nf_it, int sel_cnt,
                             const char *text)
{
	GE_CHECK_VAL(nf_it, -1);
	ge_sdbg("Count: %d, text: %s", sel_cnt, text);
	char *pd_selected = GE_STR_PD_SELECTED;

	/* Update the label text */
	if (sel_cnt > 0) {
		char *text = NULL;
		text = g_strdup_printf(pd_selected, sel_cnt);
		elm_object_item_text_set(nf_it, text);
		GE_GFREEIF(text);
	} else {
		/* Don't need to update text if it's called by language_changed_cb*/
		elm_object_item_text_set(nf_it, text);
		_ge_ui_set_translatable_item(nf_it, text);
	}
	return 0;
}

int _ge_ui_get_indicator_state(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	/* Save old view's indicator values */
	ugd->indi_mode = elm_win_indicator_mode_get(ugd->win);
	ugd->indi_o_mode = elm_win_indicator_opacity_get(ugd->win);
	ge_dbgW("indi_o_mode: %d, indi_mode: %d", ugd->indi_o_mode,
	        ugd->indi_mode);
	/* Save old view's overlap mode */
	ugd->overlap_mode = (int)evas_object_data_get(ugd->conform, "overlap");
	ge_dbgW("overlap_mode: %d", ugd->overlap_mode);
	return 0;
}

int _ge_ui_hide_indicator(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	/* transparent indicator setting */
	elm_win_indicator_mode_set(ugd->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ugd->win, ELM_WIN_INDICATOR_TRANSPARENT);
	/* Modify to start content from 0,0 */
	elm_object_signal_emit(ugd->conform, "elm,state,indicator,overlap", "");
	/* Save overlap mode when showing new view(ex: ug) */
	evas_object_data_set(ugd->conform, "overlap", (void *)EINA_TRUE);
	return 0;
}

#ifdef _USE_HIDE_INDICATOR
/********Restore indicator state of caller*********/
int _ge_ui_reset_indicator(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	ge_dbgW("indi_o_mode: %d, indi_mode: %d", ugd->indi_o_mode,
	        ugd->indi_mode);
	ge_dbgW("overlap_mode: %d", ugd->overlap_mode);
	/* Set old view's indicator */
	elm_win_indicator_mode_set(ugd->win, ugd->indi_mode);
	elm_win_indicator_opacity_set(ugd->win, ugd->indi_o_mode);
	/* set old view's conformant overlap mode
	    if layout is different with new view and needs starts from (0,60) */
	if (!ugd->overlap_mode) {
		elm_object_signal_emit(ugd->conform,
		                       "elm,state,indicator,nooverlap", "");
		evas_object_data_set(ugd->conform, "overlap", NULL);
	}
	ge_dbgW("indicator restored done!");
	return 0;
}
#endif

Evas_Object *_ge_ui_add_toolbar(Evas_Object *parent)
{
	Evas_Object *toolbar = elm_toolbar_add(parent);
	GE_CHECK_NULL(toolbar);
	elm_object_style_set(toolbar, "default");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);
	return toolbar;
}

Elm_Object_Item *_ge_ui_append_item(Evas_Object *obj, const char *icon,
                                    const char *label, Evas_Smart_Cb func,
                                    const void *data)
{
	Elm_Object_Item *it = NULL;
	it = elm_toolbar_item_append(obj, icon, label, func, data);
	_ge_ui_set_translatable_item(it, label);
	return it;
}

int _ge_ui_disable_item(Elm_Object_Item *it, Eina_Bool b_disabled)
{
	GE_CHECK_VAL(it, -1);
	/* dlog fatal is enabled. Elm_Object_Item cannot be NULL. */
	elm_object_item_disabled_set(it, b_disabled);
	return 0;
}

