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

#include "ge-ui-util.h"
#include "ge-debug.h"

Evas_Object *_ge_nocontents_create(ge_ugdata *ugd)
{
	GE_CHECK_NULL(ugd);
	Evas_Object *noc_lay = NULL;

	ge_dbg("Nocontents label: %s", GE_STR_NO_ITEMS);
	/* Full view nocontents */
	noc_lay = elm_layout_add(ugd->naviframe);
	GE_CHECK_NULL(noc_lay);

	elm_layout_theme_set(noc_lay, "layout", "nocontents", "text");

	_ge_ui_set_translate_part_str(noc_lay, "elm.text", GE_STR_NO_ITEMS);
	_ge_ui_set_translate_part_str(noc_lay, "elm.help.text", GE_STR_NO_ITEMS_HELP_TEXT);

	elm_layout_signal_emit(noc_lay, "text,disabled", "");
	elm_layout_signal_emit(noc_lay, "align.center", "elm");

	return noc_lay;
}

