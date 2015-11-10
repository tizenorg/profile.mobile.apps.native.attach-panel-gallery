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

#include "gallery-efl.h"
#include "ge-main-view.h"
#include "ge-ui-util.h"
#include "ge-util.h"
#include "ge-button.h"
#include "ge-albums.h"

#if 0//unsued
static void __ge_main_done_cb(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("");
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	elm_object_item_disabled_set((Elm_Object_Item *)ei, EINA_TRUE);
	_ge_send_result(ugd);
	ug_destroy_me(ugd->ug);
}

static void __ge_main_cancel_cb(void *data, Evas_Object *obj, void *ei)
{
	ge_dbg("");
	GE_CHECK(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	elm_naviframe_item_pop(ugd->naviframe);
}

static int __ge_main_add_btns(ge_ugdata *ugd, Evas_Object *parent,
                              Elm_Object_Item *nf_it)
{
	ge_dbg("Multiple selection, show Done");
	/* Done */
	Elm_Object_Item *tb_it = NULL;
	Evas_Object *toolbar = _ge_ui_add_toolbar(parent);
	_ge_ui_append_item(toolbar, NULL, GE_STR_ID_CANCEL, __ge_main_cancel_cb,
	                   ugd);
	tb_it = _ge_ui_append_item(toolbar, NULL, GE_STR_ID_DONE,
	                           __ge_main_done_cb, ugd);
	if (tb_it != NULL) {
		_ge_ui_disable_item(tb_it, true);
	}

	ugd->done_it = tb_it;
	elm_object_item_part_content_set(nf_it, "toolbar", toolbar);
	return 0;
}

static Eina_Bool __ge_main_back_cb(void *data, Elm_Object_Item *it)
{
	ge_dbg("");
	GE_CHECK_FALSE(data);
	ge_ugdata *ugd = (ge_ugdata *)data;

	void *pop_cb = evas_object_data_get(ugd->naviframe,
	                                    GE_NAVIFRAME_POP_CB_KEY);
	if (pop_cb) {
		Eina_Bool(*_pop_cb)(void * ugd);
		_pop_cb = pop_cb;

		if (_pop_cb(ugd)) {
			/* Just pop edit view, dont destroy me */
			return EINA_FALSE;
		}
	}

	app_control_add_extra_data(ugd->service, GE_FILE_SELECT_RETURN_COUNT, "0");
	app_control_add_extra_data(ugd->service, GE_FILE_SELECT_RETURN_PATH, NULL);
	app_control_add_extra_data(ugd->service, APP_CONTROL_DATA_SELECTED, NULL);
	ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_FAILED);
	//elm_naviframe_item_pop(it);
	ug_destroy_me(ugd->ug);
	ge_dbg("ug_destroy_me");
	/*If return ture, ug will pop naviframe first.*/
	return EINA_FALSE;
}
#endif

static Evas_Object *__ge_main_create_ly(ge_ugdata *ugd, Evas_Object *parent)
{
	ge_dbg("");
	GE_CHECK_NULL(parent);
	Evas_Object *layout = ge_ui_load_edj(parent, GE_EDJ_FILE,
	                                     GE_GRP_ALBUMVIEW);
	GE_CHECK_NULL(layout);
	evas_object_show(layout);
	return layout;
}

int _ge_main_create_view(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	GE_CHECK_VAL(ugd->naviframe, -1);
	/* Create layout of albums view */
	ugd->albums_view_ly = __ge_main_create_ly(ugd, ugd->naviframe);
	GE_CHECK_VAL(ugd->albums_view_ly, -1);

	ge_create_gengrid(ugd);
	return 0;
}

